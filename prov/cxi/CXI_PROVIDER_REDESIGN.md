# CXI Provider Redesign Plan

## Executive Summary

This document proposes a redesign of the CXI libfabric provider targeting three primary goals:
1. **Low latency** — minimize overhead on the critical send/receive/completion path
2. **Lock-free hot paths** — eliminate mutex/spinlock contention on data-plane operations
3. **Full libfabric functionality** — maintain support for all currently-implemented caps

The current provider (~35K lines) is a single-threaded-at-heart design protected by a coarse
`ep_obj->lock` (`ofi_genlock`). Under `FI_THREAD_SAFE` this becomes a real mutex that
serializes every send, receive, completion, and progress call. This plan replaces that model
with a multi-producer/single-consumer (MPSC) event pipeline, per-context command queues, and
lock-free data structures on the hot path, while keeping a narrow control-plane lock only for
rare slow-path operations (endpoint enable, MR registration, flow-control recovery).

---

## 1. Current Architecture Analysis

### 1.1 Object Hierarchy

```
fi_fabric  →  cxip_fabric
fi_domain  →  cxip_domain   (ofi_spin lock, ctrl_id_lock, cmdq_lock, trig_cmdq_lock)
fi_endpoint →  cxip_ep_obj  (ofi_genlock — the "god lock")
                ├── cxip_txc / cxip_txc_hpc / cxip_txc_rnr
                │     └── cxip_evtq  (tx_evtq — hardware event queue)
                │     └── cxip_cmdq  (tx_cmdq — HW command ring)
                └── cxip_rxc / cxip_rxc_hpc / cxip_rxc_rnr
                      └── cxip_evtq  (rx_evtq — hardware event queue)
                      └── cxip_cmdq  (rx_cmdq — HW command ring)
fi_cq      →  cxip_cq       (ep_list_lock, util_cq.cq_lock)
fi_cntr    →  cxip_cntr     (ofi_mutex lock)
```

### 1.2 Lock Inventory (current)

| Lock | Type | Protects | Hot-path? |
|---|---|---|---|
| `ep_obj->lock` | `ofi_genlock` (mutex/noop) | All txc/rxc state, req_table, ibuf_pool, msg_queue, evtq | **YES — every operation** |
| `cq->ep_list_lock` | `ofi_genlock` | EP list walked during `fi_cq_read` | **YES — every completion** |
| `domain->lock` | `ofi_spin` | Domain MR list, counters | Yes (MR operations) |
| `domain->ctrl_id_lock` | `ofi_spin` | req_ids / mr_ids indexers | Yes (MR operations) |
| `domain->cmdq_lock` | `ofi_genlock` | Domain-level TX cmdq list | Yes (auth_key multi-VNI) |
| `domain->trig_cmdq_lock` | `ofi_genlock` | Triggered op command queue | Rare (triggered ops) |
| `lni->cp_lock` | `pthread_rwlock` | Communication profile table | Slow path (alloc only) |
| `cntr->lock` | `ofi_mutex` | Counter context list | Slow path |
| `mr_domain->lock` | `ofi_spin` | MR domain hashtable | Yes (MR registration) |
| `zbcoll->lock` | `ofi_spin` | ZB collective group ID negotiation | Collective slow path |

**Total lock/unlock call sites in src/*.c: ~177** (grep count). The `ep_obj->lock` is held
across the entire duration of event-queue drain loops in `cxip_evtq_progress()`.

### 1.3 Critical Path Walk (fi_tsend → hardware)

```
fi_tsend()
  └─ cxip_tsend()
       └─ ofi_genlock_lock(ep_obj->lock)       ← LOCK ACQUIRED
            └─ cxip_txc_hpc.send_common()
                 ├─ cxip_evtq_req_alloc()      ← pool alloc under lock
                 ├─ cxip_ep_obj_map()          ← possibly MR registration
                 └─ cxip_txc_emit_idc_put()    ← writes to HW cmdq
                      └─ cxi_cq_ring()         ← doorbell write (MMIO)
       └─ ofi_genlock_unlock(ep_obj->lock)     ← LOCK RELEASED
```

### 1.4 Critical Path Walk (fi_cq_read → completion)

```
fi_cq_read()
  └─ ofi_cq_read()   (libfabric util)
       └─ cxip_util_cq_progress()
            └─ ofi_genlock_lock(cq->ep_list_lock)  ← LOCK 1
                 └─ cxip_ep_progress(ep)
                      └─ ofi_genlock_lock(ep_obj->lock)  ← LOCK 2
                           └─ cxip_evtq_progress()
                                └─ while(event) { req->cb(req, event); }
                           └─ ofi_genlock_unlock(ep_obj->lock)
            └─ ofi_genlock_unlock(cq->ep_list_lock)
       └─ ofi_cq_read_entries()
            └─ ofi_genlock_lock(cq->util_cq.cq_lock)  ← LOCK 3
```

Three distinct lock acquisitions for a single completion read. The two outer locks serialize
all threads reading from the same CQ.

### 1.5 Key HW Capabilities Available

- **IDC (Inline Data Command)**: Zero-copy for messages ≤ `C_MAX_IDC_PAYLOAD_UNR` bytes.
  Header+data in a single MMIO write. No DMA engine involvement.
- **Hardware unexpected list matching**: The Cassini NIC maintains an overflow list and
  priority list on the PTE. Unexpected messages are buffered in NIC SRAM without CPU
  intervention.
- **Counting events (CTs)**: Hardware increment counters without CPU involvement — used for
  `fi_cntr`.
- **Triggered operations**: Hardware-deferred commands that fire when a CT threshold is reached.
- **Event queue (EQ)**: Memory-mapped ring buffer written by hardware. CPU polls via
  `cxi_eq_peek_event()` — a simple memory load.
- **Rendezvous (RDZV)**: Two-sided protocol for large messages; hardware issues GET without
  CPU involvement beyond the initial PUT.

---

## 2. Design Goals and Constraints

### 2.1 Goals

| Goal | Metric |
|---|---|
| Single-thread send latency (FI_THREAD_DOMAIN) | ≤ existing baseline — no regression |
| Multi-thread send latency (FI_THREAD_SAFE) | Scalable with thread count; no global serialization |
| fi_cq_read latency | Single acquire-release cycle max |
| Completion throughput | Linear scaling with number of CQ readers |
| Memory footprint | ≤ 10% increase per endpoint |

### 2.2 Constraints

- Must remain binary-compatible with the libfabric ABI.
- Must preserve all caps: `FI_RMA | FI_ATOMICS | FI_TAGGED | FI_MSG | FI_COLLECTIVE |
  FI_HMEM | FI_TRIGGER | FI_DIRECTED_RECV | FI_MULTI_RECV | FI_NAMED_RX_CTX | FI_FENCE`.
- Must support `FI_THREAD_SAFE`, `FI_THREAD_FID`, and `FI_THREAD_DOMAIN`.
- Existing `libcxi` driver API cannot be changed (kernel boundary).
- Must handle all existing flow-control, rendezvous, and software-EP-mode scenarios.

---

## 3. Proposed Architecture

### 3.1 Core Principle: Separate Control Plane from Data Plane

The redesign draws a hard line between two classes of operations:

**Control plane** (slow path, locked):
- Endpoint enable/disable
- MR registration/deregistration
- AV insertion/removal
- Flow-control recovery (PTE state transitions)
- Rendezvous negotiation setup
- Collective join

**Data plane** (fast path, lock-free):
- Send / tagged send (IDC and DMA)
- Receive post
- RMA read/write
- Atomic operations
- Completion queue read/drain
- Counter read

### 3.2 Per-Context Command Queue Rings

Replace the single shared `ep_obj->txq` with **per-context, per-thread-stripe** command
queues:

```
                  ┌─ txc_stripe[0].cmdq  ← thread 0 claims
cxip_txc_hpc ─── ├─ txc_stripe[1].cmdq  ← thread 1 claims
                  └─ txc_stripe[N].cmdq  ← thread N claims
```

Each stripe owns a separate `cxi_cq` hardware command queue. A thread selects its stripe
via `gettid() % num_stripes` (or a pre-assigned thread-local index). Command emission to the
stripe's `cxi_cq` requires no lock because that stripe is owned by exactly one producer at
a time.

For `FI_THREAD_DOMAIN` (single-threaded use), `num_stripes = 1` and the selection is a
trivial dereference — identical cost to the current design.

**Stripe count policy**: Configured at `fi_endpoint()` time via `tx_attr.iov_limit` or an
environment variable. Default: `min(ncpus, tx_size / CXIP_MIN_TX_PER_STRIPE)`.

### 3.3 Lock-Free Request Tracking

Currently, `cxip_req` objects live in an `ofi_bufpool` and are indexed in `evtq->req_table`
(an `ofi_indexer`). Both require `ep_obj->lock` because they are shared.

**New model**: Each TX stripe has its own `req_pool` and `req_table`:

```c
struct cxip_txc_stripe {
    struct cxip_cmdq        *cmdq;         /* HW command queue — this stripe's ring */
    struct cxip_evtq         evtq;         /* HW event queue — this stripe's EQ */
    struct ofi_bufpool      *req_pool;     /* per-stripe — no sharing */
    struct indexer           req_table;    /* per-stripe — no sharing */
    atomic_int               inflight;     /* outstanding requests (atomic) */
    /* padding to cache line boundary */
    char                     _pad[0] __attribute__((aligned(64)));
};
```

Within a stripe, only ONE thread ever produces (posts operations). The EQ callback
(completion) is consumed by whichever thread calls `fi_cq_read`, but the `req_table` lookup
is keyed by `buffer_id` which encodes the stripe index in the high bits. The consuming thread
finds the right stripe and req_table without any lock:

```
buffer_id = (stripe_idx << STRIPE_SHIFT) | local_req_idx
```

Because the req_pool and req_table are per-stripe and no two threads share a stripe for
production, alloc/free of requests within a stripe needs only a **per-stripe spinlock** held
for the duration of the alloc. Alternatively an MPMC-safe slab can be used.

### 3.4 Lock-Free Completion Queue

Replace the two-level locking (`ep_list_lock` + `ep_obj->lock`) with a flat pipeline:

```
HW EQ (stripe 0)  ──┐
HW EQ (stripe 1)  ──┤──► cxip_cq_ring (MPSC lock-free ring) ──► fi_cq_read()
HW EQ (stripe N)  ──┘
```

#### 3.4.1 MPSC Completion Ring

A classic Michael-Scott MPSC queue (or a simpler cache-friendly LCRQ) holds
`struct cxip_cq_entry` objects:

```c
struct cxip_cq_entry {
    uint64_t    context;
    uint64_t    flags;
    uint64_t    data_len;
    uint64_t    buf;
    uint64_t    data;
    uint64_t    tag;
    fi_addr_t   addr;
    int         err;        /* 0 = success */
};
```

Producers (EQ callbacks per stripe) push into the MPSC ring with a single CAS. The consumer
(`fi_cq_read`) drains the ring without any lock. This eliminates `ep_list_lock`,
`ep_obj->lock` during completion, and `util_cq.cq_lock`.

For `FI_WAIT_FD` support, the consumer writes a byte to a self-pipe only when the ring
transitions from empty to non-empty.

#### 3.4.2 Progress Thread Option

An optional background progress thread can be configured to drain HW EQs and push to the
MPSC ring, completely decoupling application threads from hardware event processing. This
benefits `FI_PROGRESS_AUTO` and avoids inline progress overhead on the send path.

### 3.5 Receive Path: Deferred Lock Acquisition

For the RX path (posting receives), the hot path is posting to the HW PTE:

```c
/* NEW: lock-free fast path for receive post */
static ssize_t cxip_recv_fast(struct cxip_rxc *rxc, void *buf, size_t len, ...)
{
    /* State check via atomic load — no lock needed in steady state */
    if (atomic_load_explicit(&rxc->state, memory_order_acquire) != RXC_ENABLED)
        return cxip_recv_slow(rxc, buf, len, ...);  /* slow path: lock + handle FC */

    req = cxip_rxc_req_alloc_lock_free(rxc);        /* per-stripe pool */
    if (!req) return -FI_EAGAIN;

    /* Build and submit LE append to HW — no global lock */
    return cxip_rxc_append_le(rxc, req, buf, len, ...);
}
```

The state machine transitions (flow-control recovery, PTE re-enable) remain guarded by a
narrow `rxc->state_lock` (a spinlock held only during state transitions, not during normal
operation).

### 3.6 Revised Lock Hierarchy (New Design)

| Lock | Type | Scope | Held during |
|---|---|---|---|
| `ep_obj->ctrl_lock` | `ofi_mutex` | EP control plane | EP enable, MR bind, FC recovery only |
| `txc_stripe->req_lock` | `ofi_spin` | Per TX stripe | Request alloc/free (< 10 ns) |
| `rxc->state_lock` | `ofi_spin` | RXC state machine | State transitions only |
| `domain->ctrl_id_lock` | `ofi_spin` | MR ID allocator | MR register/close |
| `domain->mr_domain.lock` | `ofi_spin` | MR domain hash | MR cache lookup |
| `cq->wait_lock` | `ofi_spin` | CQ wait object | `fi_cq_sread` only |

**The `ep_obj->lock` is eliminated from all data-plane operations.**

---

## 4. Data Structure Changes

### 4.1 New `cxip_txc_hpc`

```c
struct cxip_txc_stripe {
    /* Cache-line 0: submission path */
    struct cxip_cmdq    *cmdq;
    struct cxip_evtq     evtq;
    ofi_spin_t           req_lock;
    struct ofi_bufpool  *req_pool;
    struct indexer       req_table;
    atomic_int           inflight;
    uint16_t             stripe_idx;
    char _pad0[64 - sizeof(struct cxip_cmdq *) - ...] ;

    /* Cache-line 1: completion path (read by consumer) */
    struct cxip_mpsc_ring *cq_ring;   /* shared with parent cxip_cq */
    char _pad1[64];
} __attribute__((aligned(64)));

struct cxip_txc_hpc {
    struct cxip_txc          base;        /* must be first */

    /* Stripe array — indexed by thread */
    struct cxip_txc_stripe  *stripes;
    int                       num_stripes;

    /* Slow path: rendezvous, flow control (protected by ep_obj->ctrl_lock) */
    struct cxip_rdzv_match_pte  *rdzv_pte;
    struct cxip_rdzv_nomatch_pte *rdzv_nomatch_pte[RDZV_NO_MATCH_PTES];
    struct indexer               rdzv_ids;
    struct dlist_entry           fc_peers;
    struct indexer               tx_ids;
    enum cxip_rdzv_proto         rdzv_proto;
    int                          max_eager_size;
    int                          rdzv_eager_size;
};
```

### 4.2 New `cxip_rxc_hpc`

```c
struct cxip_rxc_hpc {
    struct cxip_rxc   base;     /* must be first */

    /* Fast path: atomic state, per-context pool */
    atomic_int          state;          /* replaces enum cxip_rxc_state */
    ofi_spin_t          state_lock;     /* held only during transitions */
    struct ofi_bufpool *req_pool;       /* lock-free alloc via per-CPU zones */
    struct indexer      req_table;

    /* RX-initiated TX (rendezvous gets) — separate cmdq, no ep_obj->lock */
    struct cxip_cmdq   *rx_txq;
    atomic_int          orx_tx_reqs;

    /* Unexpected message buffers */
    struct cxip_ptelist_bufpool *req_list_bufpool;
    struct cxip_ptelist_bufpool *oflow_list_bufpool;

    /* Deferred event matching (PUT + PUT_OVERFLOW pairing) */
    struct def_event_ht  deferred_events; /* protected by state_lock during FC */

    /* Software unexpected list — only valid when state != RXC_ENABLED */
    struct dlist_entry   sw_recv_queue;
    struct dlist_entry   sw_ux_list;
    struct dlist_entry   sw_pending_ux_list;
    struct dlist_entry   replay_queue;
    struct dlist_entry   fc_drops;

    /* Metrics */
    int num_fc_eq_full, num_fc_no_match, num_fc_unexp;
    int num_fc_append_fail, num_fc_req_full;
    int num_sc_nic_hw2sw_append_fail, num_sc_nic_hw2sw_unexp;
    int max_eager_size;
    uint64_t rget_align_mask;
};
```

### 4.3 New `cxip_cq`

```c
struct cxip_mpsc_ring {
    /* Producer side */
    atomic_ulong      head __attribute__((aligned(64)));
    /* Consumer side */
    unsigned long     tail __attribute__((aligned(64)));
    /* Ring storage */
    unsigned long     size;     /* power of 2 */
    unsigned long     mask;
    struct cxip_cq_entry *entries;
};

struct cxip_cq {
    /* Consumer-visible fields */
    struct cxip_mpsc_ring  ring;        /* lock-free completion entries */
    struct fi_cq_attr      attr;
    struct cxip_domain    *domain;
    int                    ep_fd;       /* epoll fd for FI_WAIT_FD */
    ofi_spin_t             wait_lock;   /* serializes fi_cq_sread waiters only */

    /* Registered endpoints (slow path only — updated at bind/unbind) */
    struct dlist_entry     ep_list;
    ofi_mutex_t            ep_list_lock; /* held only at bind/close */

    /* Error queue (infrequent) */
    struct slist           err_queue;
    ofi_spin_t             err_lock;

    struct dlist_entry     dom_entry;
    unsigned int           ack_batch_size;
};
```

### 4.4 New `cxip_ep_obj`

```c
struct cxip_ep_obj {
    /* Control plane — slow path only */
    ofi_mutex_t           ctrl_lock;     /* replaces the old "god lock" */
    struct cxip_domain   *domain;
    struct cxip_av       *av;
    struct fid_peer_srx  *owner_srx;
    bool                  av_auth_key;
    struct cxi_auth_key   auth_key;
    uint16_t             *vnis;
    size_t                vni_count;
    struct cxip_addr      src_addr;
    fi_addr_t             fi_addr;
    bool                  enabled;
    uint32_t              protocol;
    uint64_t              caps;
    struct fi_ep_attr     ep_attr;
    struct fi_tx_attr     tx_attr;
    struct fi_rx_attr     rx_attr;

    /* Fast-path objects — cache-line aligned, lock-free access */
    struct cxip_txc      *txc __attribute__((aligned(64)));
    struct cxip_rxc      *rxc __attribute__((aligned(64)));

    /* Shared between TX stripes and CQ */
    struct cxip_cq       *send_cq;
    struct cxip_cq       *recv_cq;

    /* Sideband/control channel */
    struct cxip_ctrl      ctrl;
    struct cxip_cmdq     *rx_txq;    /* dedicated RX-initiated TX cmdq */

    /* Collectives */
    struct cxip_ep_coll_obj   coll;
    struct cxip_ep_zbcoll_obj zbcoll;

    struct cxip_portals_table *ptable;
    enum cassini_version       asic_ver;
    ofi_atomic32_t             ref;

    /* Hardware wait object for FI_WAIT_FD */
    struct cxil_wait_obj *priv_wait;
    int                   wait_fd;
};
```

---

## 5. Hot Path Implementation Details

### 5.1 Tagged Send (fi_tsend) — Lock-Free Fast Path

```c
static ssize_t cxip_tsend_fast(struct fid_ep *ep, const void *buf, size_t len,
                                void *desc, fi_addr_t dest_addr, uint64_t tag,
                                void *context)
{
    struct cxip_ep *cep = container_of(ep, struct cxip_ep, ep);
    struct cxip_txc_hpc *txc = container_of(cep->ep_obj->txc,
                                             struct cxip_txc_hpc, base);
    struct cxip_txc_stripe *stripe;
    struct cxip_req *req;
    int stripe_idx;

    /* 1. Select stripe — no lock, purely thread-local decision */
    stripe_idx = cxip_stripe_select(txc);   /* gettid() % num_stripes */
    stripe = &txc->stripes[stripe_idx];

    /* 2. Allocate request — per-stripe pool, brief spinlock */
    ofi_spin_lock(&stripe->req_lock);
    req = ofi_buf_alloc(stripe->req_pool);
    ofi_spin_unlock(&stripe->req_lock);
    if (!req) return -FI_EAGAIN;

    /* 3. Build and emit IDC or DMA command — no global lock */
    if (len <= txc->base.max_eager_size) {
        return cxip_tsend_idc(stripe, req, buf, len, dest_addr, tag, context, 0);
    } else {
        return cxip_tsend_rdzv(stripe, req, buf, len, dest_addr, tag, context, 0);
    }
}
```

The stripe spinlock (`req_lock`) is held for a buffer pool alloc only — a handful of
nanoseconds. The actual command emission (MMIO write + ring doorbell) happens without any
lock.

### 5.2 CQ Read — Single-Pass Drain

```c
static ssize_t cxip_cq_read(struct fid_cq *cq_fid, void *buf, size_t count)
{
    struct cxip_cq *cq = container_of(cq_fid, struct cxip_cq, ...);
    struct cxip_cq_entry *entries = buf;
    ssize_t i = 0;

    /* Drain hardware EQs into MPSC ring — inline or via background thread */
    cxip_cq_progress_inline(cq);      /* polls all registered stripe EQs */

    /* Drain MPSC ring into user buffer — no lock */
    while (i < (ssize_t)count) {
        struct cxip_cq_entry *e = cxip_mpsc_pop(&cq->ring);
        if (!e) break;
        entries[i++] = *e;
    }

    return i ? i : (cxip_cq_has_error(cq) ? -FI_EAVAIL : -FI_EAGAIN);
}
```

`cxip_cq_progress_inline()` iterates over all TX/RX stripe EQs. Each stripe EQ is
single-owner: only one thread drains it at a time (the caller of `fi_cq_read`). Because
we have one EQ per stripe and stripes are not shared between producers, no lock is needed
to drain a stripe EQ. Completions are pushed into the MPSC ring via a single CAS.

### 5.3 Progress Model

Three modes supported:

| Mode | Mechanism |
|---|---|
| `FI_PROGRESS_MANUAL` | Application calls `fi_cq_read` — inline drains all EQs |
| `FI_PROGRESS_AUTO` | Background thread drains HW EQs, pushes to MPSC ring |
| `FI_PROGRESS_MANUAL` + background | Hybrid: background handles rendezvous/ctrl, app drains data EQs |

For auto-progress, the background thread only acquires `rxc->state_lock` during flow-control
transitions. Normal steady-state event processing requires no lock.

---

## 6. Flow Control and State Machine

The existing flow-control state machine is preserved but restructured:

### 6.1 State Storage

Replace `enum cxip_rxc_state` field (protected by `ep_obj->lock`) with:
```c
atomic_int rxc->state;  /* read lock-free on fast path */
```

Fast path checks:
```c
if (atomic_load_explicit(&rxc->state, memory_order_acquire) == RXC_ENABLED)
    /* proceed lock-free */
else
    cxip_recv_slow(rxc, ...);  /* acquires state_lock */
```

### 6.2 Flow Control Recovery

FC recovery (PTE disable → onload UX → re-enable) is the slow path. It acquires
`rxc->state_lock` and `ep_obj->ctrl_lock` only for the duration of the state transition.
Posted receives during FC return `-FI_EAGAIN` (unchanged behavior). The state is written
atomically with `memory_order_release` so that racing fast-path threads see the new state
immediately.

### 6.3 Software EP Mode

`RXC_ENABLED_SOFTWARE` mode is unchanged functionally but the SW receive queue
(`sw_recv_queue`) is protected by `rxc->state_lock` instead of `ep_obj->lock`. Since SW
mode implies hardware matching is disabled, the fast path check falls through to the slow
path in all cases — correctness is maintained.

---

## 7. Memory Registration (fi_mr_reg)

MR registration is an inherently slow operation. The existing `ofi_mr_cache` is already
designed for concurrent access with its own internal locking. The redesign makes no changes
to MR registration locking.

**Improvement**: The inline `fi_mr_regattr` path for **already-cached** MRs (ODP / ATS mode)
is optimized by:
1. Checking the MR cache with a reader lock (`ofi_spin_lock` → `ofi_rwlock_rdlock`)
2. Only acquiring the writer lock on cache miss

This reduces contention when many threads register the same memory region concurrently.

---

## 8. RMA and Atomic Operations

### 8.1 RMA Write (fi_write)

Same pattern as tagged send: stripe selection → per-stripe req alloc → DMA command emission.
No `ep_obj->lock` needed.

```
fi_write()
  └─ cxip_write()
       ├─ stripe = cxip_stripe_select(txc)         [thread-local, no lock]
       ├─ req = cxip_stripe_req_alloc(stripe)       [per-stripe spinlock, ~5 ns]
       └─ cxip_emit_dma_put(stripe, req, ...)       [MMIO write, no lock]
```

### 8.2 RMA Read (fi_read)

Same pattern. The REPLY event is processed by the same stripe's EQ, so the
completion chain remains stripe-local.

### 8.3 Fetching Atomics

Fetching atomics require a result buffer registered with the NIC. These use the same
per-stripe req_pool and emit a DMA-AMO command. No changes to correctness semantics.

### 8.4 Non-Fetching Atomics (IDC-AMO)

Inline for small operands — zero per-request allocation needed when completion is not
required.

---

## 9. Collective Operations

Collective operations (`fi_join_collective`, `fi_barrier`, `fi_allreduce`, etc.) are
inherently control-plane operations involving rendezvous with other nodes. They continue to
use `ep_obj->ctrl_lock` (the new slow-path lock) for collective join state and the zbcoll
group ID negotiation lock.

The data-plane collective reduce operations (zero-buffer collectives) are already
hardware-offloaded via multicast PTEs and do not require changes to their fast path.

---

## 10. Triggered Operations

Triggered operations use the domain-level `trig_cmdq` and counter thresholds. This is a
slow-path feature (rare submission frequency) and continues to use
`domain->trig_cmdq_lock`. No changes proposed.

---

## 11. Address Vector (fi_av)

AV operations (insert, lookup) use the existing `util_av` locking from the libfabric
utility layer. AV lookups on the send path (`cxip_av_lookup_addr()`) become:

```c
/* Lock-free read: AV entries are immutable after insert + memory_order_release */
static inline struct cxip_addr *cxip_av_lookup_fast(struct cxip_av *av,
                                                     fi_addr_t addr)
{
    /* RCU-style: entries are never mutated after insertion */
    return &av->table[addr];   /* bounds-checked, no lock */
}
```

Insertions use `ofi_mutex_lock(&av->lock)` (existing). After insertion, the entry is
published with `atomic_thread_fence(memory_order_release)`.

---

## 12. Thread Safety Modes

| `fi_threading` | `num_stripes` | `req_lock` overhead | Notes |
|---|---|---|---|
| `FI_THREAD_DOMAIN` | 1 | Eliminated (single thread, skip lock) | Fastest; existing performance preserved |
| `FI_THREAD_FID` | 1 per endpoint | Per-stripe spinlock (~5 ns) | One thread per endpoint |
| `FI_THREAD_SAFE` | ncpus or configured | Per-stripe spinlock | Full multi-thread scalability |

For `FI_THREAD_DOMAIN`, the stripe selection is a compile-time constant (`stripe = &txc->stripes[0]`),
and the `req_lock` acquire/release is compiled away with a `likely()` branch that checks
`txc->num_stripes == 1`.

---

## 13. Implementation Phases

### Phase 1: Foundation — Per-Stripe TX (no behavioral change for single-thread)
**Scope**: Restructure `cxip_txc_hpc` to use a stripe array. For `num_stripes == 1`, behavior
is identical to current code. The `ep_obj->lock` is still present but only required for
stripe count > 1.

**Files modified**:
- `include/cxip.h` — add `cxip_txc_stripe`, update `cxip_txc_hpc`
- `src/cxip_txc.c` — stripe init/fini, stripe selection helper
- `src/cxip_msg_hpc.c` — update send path to use stripe
- `src/cxip_rma.c` — update RMA emission to use stripe
- `src/cxip_atomic.c` — update AMO emission to use stripe

**Deliverable**: Existing tests pass. No performance regression. Multi-stripe path
compiles but is not yet exposed.

### Phase 2: MPSC Completion Ring
**Scope**: Replace `util_cq` cirq + `cq_lock` with `cxip_mpsc_ring`. Remove
`ep_list_lock` from `fi_cq_read` critical path.

**Files modified**:
- `include/cxip.h` — `cxip_cq`, `cxip_cq_entry`, `cxip_mpsc_ring`
- `src/cxip_cq.c` — new read/write/error implementations
- `src/cxip_evtq.c` — push to MPSC ring from EQ callback

**Deliverable**: `fi_cq_read` acquires zero locks in steady state.

### Phase 3: Lock-Free RX State Check
**Scope**: Convert `rxc->state` to `atomic_int`. Add fast-path branch that skips
`ep_obj->lock` when `state == RXC_ENABLED`.

**Files modified**:
- `include/cxip.h` — `atomic_int state` in `cxip_rxc`
- `src/cxip_msg_hpc.c` — fast/slow path split in recv_common
- `src/cxip_rxc.c` — state transition helpers with `memory_order_release`

**Deliverable**: `fi_trecv` is lock-free in steady state.

### Phase 4: Eliminate `ep_obj->lock` from Data Plane
**Scope**: Remove `ep_obj->lock` acquire/release from all send/recv/rma/atomic fast
paths. Rename the remaining lock to `ctrl_lock` and restrict it to control-plane
operations only.

**Files modified**: All `src/cxip_*.c` files with `ofi_genlock_lock(ep_obj->lock)`
usages — convert each site to either lock-free (data plane) or `ctrl_lock` (control plane).

**Deliverable**: All 177 lock sites audited and reclassified. Benchmark shows linear
scaling with thread count.

### Phase 5: Multi-Stripe Exposure and Auto-Progress
**Scope**: Expose `num_stripes` via environment variable. Implement background progress
thread for `FI_PROGRESS_AUTO`.

**Files modified**:
- `src/cxip_info.c` — expose new env vars
- `src/cxip_ep.c` — stripe allocation at endpoint enable
- New file: `src/cxip_progress.c` — background progress thread

**Deliverable**: Multi-threaded MPI communication at full NIC bandwidth with no lock
contention.

### Phase 6: HMEM and GPU Buffer Support
**Scope**: Validate that HMEM (ROCm, CUDA, oneAPI Level Zero) paths work correctly with
the new per-stripe structure. GPU buffer MR registration is a slow-path operation and
requires no changes. The copy-to/from HMEM helpers are called without locks already.

**Deliverable**: Existing HMEM tests pass with new implementation.

---

## 14. Compatibility and Testing Strategy

### 14.1 API Compatibility
All libfabric API functions remain identical. The redesign is purely internal.

### 14.2 Thread Safety Regression Tests
The existing `test/` suite covers all operation types. Additional stress tests:
- `test/tagged_stress.c` — already exists, extend with multi-thread variants
- New: `test/mt_tsend.c` — `N` threads × `M` sends per thread, verify all completions
- New: `test/mt_cq_read.c` — multiple threads reading from a single CQ

### 14.3 Flow Control Tests
FC recovery tests in `test/msg.c` and `test/tagged.c` must pass unchanged because
the FC state machine behavior is preserved.

### 14.4 Latency Benchmarks
- Single-thread pingpong: must not regress vs. current code
- Multi-thread fan-out: measure with 1, 2, 4, 8, 16 threads

---

## 15. Risk Analysis

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| ABI mismatch with libcxi | Low | High | All libcxi API calls unchanged |
| Memory ordering bugs (TSO vs. non-TSO) | Medium | High | Use `memory_order_acquire/release` explicitly; validate on Arm |
| MPSC ring overflow | Low | Medium | Ring sized at `tx_size * num_stripes`; overflow returns `-FI_EAGAIN` |
| Flow control + multi-stripe interaction | Medium | High | FC transitions drain all stripes before state change |
| Rendezvous buffer_id encoding (stripe bits) | Medium | Medium | Define `STRIPE_SHIFT` constant; verify no collision with existing 16-bit buffer_id space |
| Performance regression under FI_THREAD_DOMAIN | Low | High | `num_stripes == 1` path is identical to current code |

---

## 16. File-by-File Change Summary

| File | Change Type | Description |
|---|---|---|
| `include/cxip.h` | Major refactor | Add stripe structs, MPSC ring, rewrite cxip_cq/ep_obj, atomic rxc state |
| `src/cxip_ep.c` | Moderate | Stripe allocation, ctrl_lock rename, remove data-plane lock usage |
| `src/cxip_txc.c` | Moderate | Stripe init/fini, stripe selection, req alloc per stripe |
| `src/cxip_rxc.c` | Moderate | Atomic state, fast/slow recv split, state transition helpers |
| `src/cxip_msg_hpc.c` | Major | Full send/recv hot path rewrite using stripes |
| `src/cxip_msg_rnr.c` | Moderate | Same stripe pattern for RNR protocol |
| `src/cxip_rma.c` | Moderate | Stripe-aware DMA emission |
| `src/cxip_atomic.c` | Moderate | Stripe-aware AMO emission |
| `src/cxip_cq.c` | Major | MPSC ring implementation, lock-free cq_read |
| `src/cxip_evtq.c` | Moderate | Push to MPSC ring, per-stripe EQ init |
| `src/cxip_cmdq.c` | Minor | Per-stripe cmdq helpers |
| `src/cxip_dom.c` | Minor | Remove data-plane lock sites |
| `src/cxip_mr.c` | Minor | Use rdlock for cached lookups |
| `src/cxip_ctrl.c` | Minor | Use ctrl_lock instead of ep_obj->lock |
| `src/cxip_av.c` | Minor | Release fence on insert |
| New: `src/cxip_progress.c` | New | Background progress thread (Phase 5) |

---

## 17. Environment Variables (New)

| Variable | Default | Description |
|---|---|---|
| `FI_CXI_TX_STRIPES` | `0` (auto) | Number of TX stripes per endpoint. 0 = auto based on ncpus |
| `FI_CXI_PROGRESS_THREAD` | `0` | Enable background progress thread (0=off, 1=on) |
| `FI_CXI_PROGRESS_CPU` | `-1` | CPU affinity for progress thread (-1=any) |
| `FI_CXI_MPSC_RING_SIZE` | `0` (inherit cq_size) | Override MPSC ring entry count |

---

## Appendix A: MPSC Ring Implementation Sketch

```c
/*
 * Single-producer-friendly variant: the "push" side uses XCHG on head,
 * ensuring ordering without a full fence on x86 TSO.
 * For non-TSO architectures, memory_order_acq_rel is used.
 */

static inline bool cxip_mpsc_push(struct cxip_mpsc_ring *r,
                                   const struct cxip_cq_entry *e)
{
    unsigned long head = atomic_fetch_add_explicit(&r->head, 1,
                                                    memory_order_acq_rel);
    if (head - r->tail >= r->size)
        return false;  /* ring full */
    r->entries[head & r->mask] = *e;
    atomic_thread_fence(memory_order_release);
    return true;
}

static inline struct cxip_cq_entry *cxip_mpsc_pop(struct cxip_mpsc_ring *r)
{
    unsigned long head = atomic_load_explicit(&r->head, memory_order_acquire);
    if (r->tail == head)
        return NULL;
    struct cxip_cq_entry *e = &r->entries[r->tail & r->mask];
    atomic_thread_fence(memory_order_acquire);
    r->tail++;
    return e;
}
```

---

## Appendix B: Stripe Selection

```c
static __thread int tls_stripe_idx = -1;

static inline int cxip_stripe_select(struct cxip_txc_hpc *txc)
{
    if (txc->num_stripes == 1)
        return 0;
    if (tls_stripe_idx < 0)
        tls_stripe_idx = (int)(gettid() % txc->num_stripes);
    return tls_stripe_idx;
}
```

Thread-local storage avoids the `gettid()` syscall on subsequent calls (the value is cached
in TLS). On the first call per thread, the syscall cost (~10 ns) is amortized over all
subsequent sends from that thread.

---

*Document version: 1.0 — Initial proposal*
*Target: prov/cxi redesign for low-latency, lock-free operation*
