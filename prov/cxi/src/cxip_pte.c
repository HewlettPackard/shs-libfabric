/*
 * (C) Copyright 2023 Hewlett Packard Enterprise Development LP
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "cxip.h"

#define CXIP_DBG(...) _CXIP_DBG(FI_LOG_DOMAIN, __VA_ARGS__)
#define CXIP_INFO(...) _CXIP_INFO(FI_LOG_DOMAIN, __VA_ARGS__)
#define CXIP_WARN(...) _CXIP_WARN(FI_LOG_DOMAIN, __VA_ARGS__)

/* Caller musthold ep_obj->lock. */
int cxip_pte_set_state(struct cxip_pte *pte, struct cxip_cmdq *cmdq,
		       enum c_ptlte_state new_state, uint32_t drop_count)
{
	int ret;
	struct c_set_state_cmd set_state = {
		.command.opcode = C_CMD_TGT_SETSTATE,
		.ptlte_index = pte->pte->ptn,
		.ptlte_state = new_state,
		.drop_count = drop_count,
	};

	ret = cxi_cq_emit_target(cmdq->dev_cmdq, &set_state);
	if (ret) {
		CXIP_WARN("Failed to enqueue command: %d\n", ret);
		return -FI_EAGAIN;
	}

	cxi_cq_ring(cmdq->dev_cmdq);

	return FI_SUCCESS;
}

/*
 * cxip_pte_set_wait() - Set a new PTE state synchronously.
 *
 * TODO: EP lock associated with the EP must be taken.
 */
int cxip_pte_set_state_wait(struct cxip_pte *pte, struct cxip_cmdq *cmdq,
			    struct cxip_evtq *evtq,
			    enum c_ptlte_state new_state, uint32_t drop_count)
{
	int ret;

	ret = cxip_pte_set_state(pte, cmdq, new_state, drop_count);
	if (ret == FI_SUCCESS) {
		do {
			sched_yield();
			cxip_evtq_progress(evtq);
		} while (pte->state != new_state);
	}

	return ret;
}

/*
 * cxip_pte_append() - Append a buffer to a PtlTE.
 *
 * Caller must hold ep_obj->lock.
 */
int cxip_pte_append(struct cxip_pte *pte, uint64_t iova, size_t len,
		    unsigned int lac, enum c_ptl_list list,
		    uint32_t buffer_id, uint64_t match_bits,
		    uint64_t ignore_bits, uint32_t match_id,
		    uint64_t min_free, uint32_t flags,
		    struct cxip_cntr *cntr, struct cxip_cmdq *cmdq,
		    bool ring)
{
	union c_cmdu cmd = {};
	int rc;

	cmd.command.opcode                = C_CMD_TGT_APPEND;
	cmd.target.ptl_list               = list;
	cmd.target.ptlte_index            = pte->pte->ptn;
	cmd.target.buffer_id              = buffer_id;
	cmd.target.lac                    = lac;
	cmd.target.start                  = iova;
	cmd.target.length                 = len;
	cmd.target.ct                     = cntr ? cntr->ct->ctn : 0;
	cmd.target.match_bits             = match_bits;
	cmd.target.ignore_bits            = ignore_bits;
	cmd.target.match_id               = match_id;
	cmd.target.min_free               = min_free;

	cxi_target_cmd_setopts(&cmd.target, flags);

	rc = cxi_cq_emit_target(cmdq->dev_cmdq, &cmd);
	if (rc) {
		CXIP_DBG("Failed to write Append command: %d\n", rc);
		/* Return error according to Domain Resource Management */
		return -FI_EAGAIN;
	}

	if (ring)
		cxi_cq_ring(cmdq->dev_cmdq);

	return FI_SUCCESS;
}

/*
 * cxip_pte_unlink() - Unlink a buffer from a PtlTE.
 *
 * Caller must hold ep_obj->lock.
 */
int cxip_pte_unlink(struct cxip_pte *pte, enum c_ptl_list list,
		    int buffer_id, struct cxip_cmdq *cmdq)
{
	union c_cmdu cmd = {};
	int rc;

	cmd.command.opcode = C_CMD_TGT_UNLINK;
	cmd.target.ptl_list = list;
	cmd.target.ptlte_index  = pte->pte->ptn;
	cmd.target.buffer_id = buffer_id;

	rc = cxi_cq_emit_target(cmdq->dev_cmdq, &cmd);
	if (rc) {
		CXIP_DBG("Failed to write Append command: %d\n", rc);
		/* Return error according to Domain Resource Management */
		return -FI_EAGAIN;
	}

	cxi_cq_ring(cmdq->dev_cmdq);

	return FI_SUCCESS;
}

/*
 * cxip_pte_map() - Map a PtlTE to a specific PID index. A single PtlTE can be
 * mapped into MAX_PTE_MAP_COUNT different PID indices.
 */
int cxip_pte_map(struct cxip_pte *pte, uint64_t pid_idx, bool is_multicast)
{
	int ret;

	if (pte->pte_map_count >= MAX_PTE_MAP_COUNT)
		return -FI_ENOSPC;

	ret = cxil_map_pte(pte->pte, pte->ptable->dom, pid_idx, is_multicast,
			   &pte->pte_map[pte->pte_map_count]);
	if (ret) {
		CXIP_WARN("Failed to map PTE: %d\n", ret);
		return -FI_EADDRINUSE;
	}

	pte->pte_map_count++;

	return FI_SUCCESS;
}

/*
 * cxip_pte_alloc_nomap() - Allocate a PtlTE without performing any mapping
 * during allocation.
 */
int cxip_pte_alloc_nomap(struct cxip_portals_table *ptable, struct cxi_eq *evtq,
			 struct cxi_pt_alloc_opts *opts,
			 void (*state_change_cb)(struct cxip_pte *pte,
						 const union c_event *event),
			 void *ctx, struct cxip_pte **pte)
{
	struct cxip_pte *new_pte;
	int ret;

	new_pte = calloc(1, sizeof(*new_pte));
	if (!new_pte) {
		CXIP_WARN("Failed to allocate PTE structure\n");
		return -FI_ENOMEM;
	}

	/* Allocate a PTE */
	ret = cxil_alloc_pte(ptable->lni->lni, evtq, opts,
			     &new_pte->pte);
	if (ret) {
		CXIP_WARN("Failed to allocate PTE: %d\n", ret);
		ret = -FI_ENOSPC;
		goto free_mem;
	}

	ofi_spin_lock(&ptable->lni->iface->lock);
	dlist_insert_tail(&new_pte->pte_entry, &ptable->lni->iface->ptes);
	ofi_spin_unlock(&ptable->lni->iface->lock);

	new_pte->ptable = ptable;
	new_pte->state_change_cb = state_change_cb;
	new_pte->ctx = ctx;
	new_pte->state = C_PTLTE_DISABLED;

	*pte = new_pte;

	return FI_SUCCESS;

free_mem:
	free(new_pte);

	return ret;
}

/*
 * cxip_pte_alloc() - Allocate and map a PTE for use.
 */
int cxip_pte_alloc(struct cxip_portals_table *ptable, struct cxi_eq *evtq,
		   uint64_t pid_idx, bool is_multicast,
		   struct cxi_pt_alloc_opts *opts,
		   void (*state_change_cb)(struct cxip_pte *pte,
					   const union c_event *event),
		   void *ctx, struct cxip_pte **pte)
{
	int ret;

	ret = cxip_pte_alloc_nomap(ptable, evtq, opts, state_change_cb,
				   ctx, pte);
	if (ret)
		return ret;

	ret = cxip_pte_map(*pte, pid_idx, is_multicast);
	if (ret)
		goto free_pte;

	return FI_SUCCESS;

free_pte:
	cxip_pte_free(*pte);

	return ret;
}

/*
 * cxip_pte_free() - Free a PTE.
 */
void cxip_pte_free(struct cxip_pte *pte)
{
	int ret;
	int i;

	ofi_spin_lock(&pte->ptable->lni->iface->lock);
	dlist_remove(&pte->pte_entry);
	ofi_spin_unlock(&pte->ptable->lni->iface->lock);

	for (i = pte->pte_map_count; i > 0; i--) {
		ret = cxil_unmap_pte(pte->pte_map[i - 1]);
		if (ret)
			CXIP_WARN("Failed to unmap PTE: %d\n", ret);
	}

	ret = cxil_destroy_pte(pte->pte);
	if (ret)
		CXIP_WARN("Failed to free PTE: %d\n", ret);

	free(pte);
}

/*
 * cxip_pte_state_change() - Atomically update PTE state. Used during
 * STATE_CHANGE event processing.
 */
int cxip_pte_state_change(struct cxip_if *dev_if, const union c_event *event)
{
	struct cxip_pte *pte;

	ofi_spin_lock(&dev_if->lock);

	dlist_foreach_container(&dev_if->ptes,
				struct cxip_pte, pte, pte_entry) {
		if (pte->pte->ptn == event->tgt_long.ptlte_index) {
			pte->state = event->tgt_long.initiator.state_change.ptlte_state;
			if (pte->state_change_cb)
				pte->state_change_cb(pte, event);

			ofi_spin_unlock(&dev_if->lock);
			return FI_SUCCESS;
		}
	}

	ofi_spin_unlock(&dev_if->lock);

	return -FI_EINVAL;
}
