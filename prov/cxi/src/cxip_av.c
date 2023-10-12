/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2014 Intel Corporation, Inc. All rights reserved.
 * Copyright (c) 2016 Cisco Systems, Inc. All rights reserved.
 * Copyright (c) 2017 Los Alamos National Security, LLC.
 *                    All rights reserved.
 * Copyright (c) 2018 Cray Inc. All rights reserved.
 * Copyright (c) 2020 Cray Inc. All rights reserved.
 */

#include "config.h"

#include <sys/types.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <netinet/ether.h>

#include "cxip.h"

#include "ofi_osd.h"
#include "ofi_util.h"

#define CXIP_DBG(...) _CXIP_DBG(FI_LOG_AV, __VA_ARGS__)
#define CXIP_WARN(...) _CXIP_WARN(FI_LOG_AV, __VA_ARGS__)

/*
 * cxip_parse_cxi_addr() - Parse node and service arguments representing a CXI
 * address.
 */
static int cxip_parse_cxi_addr(const char *node, const char *service,
			       struct cxip_addr *addr)
{
	struct ether_addr *mac;
	uint32_t scan_nic;
	uint32_t scan_pid;

	if (!node)
		return -FI_ENODATA;

	mac = ether_aton(node);
	if (mac) {
		addr->nic = cxip_mac_to_nic(mac);
	} else if (sscanf(node, "%i", &scan_nic) == 1) {
		addr->nic = scan_nic;
	} else {
		return -FI_ENODATA;
	}

	if (!service)
		addr->pid = C_PID_ANY;
	else if (sscanf(service, "%i", &scan_pid) == 1)
		addr->pid = scan_pid;
	else
		return -FI_ENODATA;

	return FI_SUCCESS;
}

static inline void cxip_av_read_lock(struct cxip_av *av)
{
	if (!av->lockless)
		pthread_rwlock_rdlock(&av->lock);
}

static inline void cxip_av_write_lock(struct cxip_av *av)
{
	if (!av->lockless)
		pthread_rwlock_wrlock(&av->lock);
}

static inline void cxip_av_unlock(struct cxip_av *av)
{
	if (!av->lockless)
		pthread_rwlock_unlock(&av->lock);
}

static int cxip_av_insert_addr(struct cxip_av *av, struct cxip_addr *addr,
			       fi_addr_t *fi_addr, uint64_t flags)
{
	struct cxip_av_entry *entry;

	CXIP_DBG("Inserting nid=%#x pid=%d\n", addr->nic, addr->pid);

	HASH_FIND(hh, av->av_entry_hash, addr, sizeof(*addr), entry);
	if (entry) {
		if (fi_addr)
			*fi_addr = ofi_buf_index(entry);
		if (ofi_atomic_inc32(&entry->use_cnt) > 1)
			CXIP_WARN("nid=%#x pid=%d inserted multiple times\n",
				  addr->nic, addr->pid);

		return FI_SUCCESS;
	}

	entry = ofi_ibuf_alloc(av->av_entry_pool);
	if (!entry) {
		CXIP_WARN("Failed to allocated AV entry memory\n");
		if (fi_addr)
			*fi_addr = FI_ADDR_NOTAVAIL;
		return -FI_ENOMEM;
	}

	memcpy(&entry->addr, addr, sizeof(*addr));
	ofi_atomic_initialize32(&entry->use_cnt, 1);
	HASH_ADD(hh, av->av_entry_hash, addr, sizeof(*addr), entry);

	if (flags & FI_AV_USER_ID)
		entry->fi_addr = *fi_addr;
	else
		entry->fi_addr = ofi_buf_index(entry);

	if (fi_addr)
		*fi_addr = ofi_buf_index(entry);

	ofi_atomic_inc32(&av->av_entry_cnt);

	return FI_SUCCESS;
}

#define AV_INSERT_VALID_FLAGS (FI_MORE | FI_AV_USER_ID)

static int cxip_av_insert_validate_args(struct fid_av *fid, const void *addr_in,
					size_t count, fi_addr_t *fi_addr,
					uint64_t flags, void *context)
{
	uint64_t unsupported_flags = flags & ~AV_INSERT_VALID_FLAGS;
	struct cxip_av *av = container_of(fid, struct cxip_av, av_fid.fid);

	if (!addr_in && count) {
		CXIP_WARN("NULL addr buffer\n");
		return -FI_EINVAL;
	}

	if (unsupported_flags) {
		CXIP_WARN("Unsupported AV insert flags: %#lx\n",
			  unsupported_flags);
		return -FI_EINVAL;
	}

	/* FI_SYMMETRIC is an optimization using logical matching. This avoids
	 * doing a reverse lookup for support FI_SOURCE. Since no lookup
	 * occurs, FI_AV_USER_ID cannot be support.
	 */
	if (av->symmetric && (flags & FI_AV_USER_ID)) {
		CXIP_WARN("FI_SYMMETRIC not supported with FI_AV_USER_ID\n");
		return -FI_EINVAL;
	}

	if (!fi_addr && (flags & FI_AV_USER_ID)) {
		CXIP_WARN("NULL fi_addr with FI_AV_USER_ID\n");
		return -FI_EINVAL;
	}

	return FI_SUCCESS;
}

/* NETSIM collectives simulation reqires many-to-one fi_addr to cxip_addr,
 * i.e., multiple fi_addr values that resolve to the same target address. The
 * new reverse-lookup model requires unique one-to-one, i.e. every cxip_addr
 * must be unique. These filter functions allow insert/lookup modifications
 * of the values by replacing these functions in the test code.
 */
static struct cxip_addr *insert_in(const void *addr_in)
{
	return (struct cxip_addr *)addr_in;
}
static void insert_out(struct cxip_addr *addr_out,
		       struct cxip_addr *addr_in)
{
	*addr_out = *addr_in;
}
struct cxip_addr *(*cxip_av_addr_in)(const void *addr) = insert_in;
void (*cxip_av_addr_out)(struct cxip_addr *addr_out,
			 struct cxip_addr *addr) = insert_out;

static int cxip_av_insert(struct fid_av *fid, const void *addr_in, size_t count,
			  fi_addr_t *fi_addr, uint64_t flags, void *context)
{
	struct cxip_av *av = container_of(fid, struct cxip_av, av_fid.fid);
	size_t i;
	size_t success_cnt = 0;
	int ret;

	ret = cxip_av_insert_validate_args(fid, addr_in, count, fi_addr, flags,
					   context);
	if (ret != FI_SUCCESS)
		return ret;

	cxip_av_write_lock(av);

	for (i = 0; i < count; i++) {
		ret = cxip_av_insert_addr(av, cxip_av_addr_in(addr_in) + i,
					  fi_addr ? &fi_addr[i] : NULL, flags);
		if (ret == FI_SUCCESS)
			success_cnt++;
	}

	cxip_av_unlock(av);

	return success_cnt;
}

static int cxip_av_insertsvc_validate_args(struct fid_av *fid, const char *node,
					   const char *service,
					   fi_addr_t *fi_addr, uint64_t flags,
					   void *context)
{
	if (!node) {
		CXIP_WARN("NULL node\n");
		return -FI_EINVAL;
	}

	if (!service) {
		CXIP_WARN("NULL service\n");
		return -FI_EINVAL;
	}

	return FI_SUCCESS;
}

static int cxip_av_insertsvc(struct fid_av *fid, const char *node,
			     const char *service, fi_addr_t *fi_addr,
			     uint64_t flags, void *context)
{
	struct cxip_addr addr = {};
	int ret;

	ret = cxip_av_insertsvc_validate_args(fid, node, service, fi_addr,
					      flags, context);
	if (ret != FI_SUCCESS)
		return ret;

	ret = cxip_parse_cxi_addr(node, service, &addr);
	if (ret != FI_SUCCESS) {
		CXIP_WARN("Failed to parse node %s and service %s\n", node,
			  service);
		return ret;
	}

	return cxip_av_insert(fid, &addr, 1, fi_addr, flags, context);
}

int cxip_av_lookup_addr(struct cxip_av *av, fi_addr_t fi_addr,
			struct cxip_addr *addr)
{
	struct cxip_av_entry *entry;

	cxip_av_read_lock(av);

	entry = ofi_bufpool_get_ibuf(av->av_entry_pool, fi_addr);
	if (entry && addr)
		cxip_av_addr_out(addr, &entry->addr);

	cxip_av_unlock(av);

	if (entry)
		return FI_SUCCESS;

	CXIP_WARN("Invalid fi_addr %#lx\n", fi_addr);

	return -FI_EINVAL;
}

fi_addr_t cxip_av_lookup_fi_addr(struct cxip_av *av,
				 const struct cxip_addr *addr)
{
	struct cxip_av_entry *entry;
	fi_addr_t fi_addr;

	cxip_av_read_lock(av);

	HASH_FIND(hh, av->av_entry_hash, addr, sizeof(*addr), entry);
	fi_addr = entry ? entry->fi_addr : FI_ADDR_NOTAVAIL;

	cxip_av_unlock(av);

	return fi_addr;
}

int cxip_av_bind_ep(struct cxip_av *av, struct cxip_ep *ep)
{
	int ret;

	if (av->domain != ep->ep_obj->domain) {
		CXIP_WARN("EP belongs to different domain\n");
		return -FI_EINVAL;
	}

	cxip_av_write_lock(av);
	ret = fid_list_insert(&av->ep_list, NULL, &ep->ep.fid);
	cxip_av_unlock(av);

	if (ret != FI_SUCCESS) {
		CXIP_WARN("EP bind failed: %d\n", ret);
		return ret;
	}

	ofi_atomic_inc32(&av->ref);
	return FI_SUCCESS;
}

void cxip_av_unbind_ep(struct cxip_av *av, struct cxip_ep *ep)
{
	cxip_av_write_lock(av);
	fid_list_remove(&av->ep_list, NULL, &ep->ep.fid);
	cxip_av_unlock(av);

	ofi_atomic_dec32(&av->ref);
}

static int cxip_av_lookup(struct fid_av *fid, fi_addr_t fi_addr, void *addr_out,
			  size_t *addrlen)
{
	struct cxip_av *av = container_of(fid, struct cxip_av, av_fid.fid);
	struct cxip_addr addr;
	int ret;

	ret = cxip_av_lookup_addr(av, fi_addr, &addr);
	if (ret != FI_SUCCESS)
		return ret;

	memcpy(addr_out, &addr, MIN(*addrlen, sizeof(addr)));
	*addrlen = sizeof(addr);

	return FI_SUCCESS;
}

static void cxip_av_remove_addr(struct cxip_av *av, fi_addr_t fi_addr)
{
	struct cxip_av_entry *entry;
	int use_cnt;

	entry = ofi_bufpool_get_ibuf(av->av_entry_pool, fi_addr);
	if (!entry) {
		CXIP_WARN("Invalid fi_addr: %#lx\n", fi_addr);
		return;
	}

	use_cnt = ofi_atomic_dec32(&entry->use_cnt);
	if (use_cnt)
		return;

	CXIP_DBG("Removing nid=%#x pid=%d\n", entry->addr.nic,
		 entry->addr.pid);

	ofi_atomic_dec32(&av->av_entry_cnt);
	HASH_DELETE(hh, av->av_entry_hash, entry);
	ofi_ibuf_free(entry);
}

static int cxip_av_remove(struct fid_av *fid, fi_addr_t *fi_addr,
			  size_t count, uint64_t flags)
{
	struct cxip_av *av = container_of(fid, struct cxip_av, av_fid.fid);
	size_t i;

	if (flags) {
		CXIP_WARN("Unsupported flags: %#lx\n", flags);
		return -FI_EINVAL;
	}

	for (i = 0; i < count; i++) {
		cxip_av_write_lock(av);
		cxip_av_remove_addr(av, fi_addr[i]);
		cxip_av_unlock(av);
	}

	return FI_SUCCESS;
}

static const char *cxip_av_straddr(struct fid_av *fid, const void *addr,
				   char *buf, size_t *len)
{
	return ofi_straddr(buf, len, FI_ADDR_CXI, addr);
}

static int cxip_av_close(struct fid *fid)
{
	struct cxip_av *av = container_of(fid, struct cxip_av, av_fid.fid);
	struct cxip_domain *dom = av->domain;

	if (ofi_atomic_get32(&av->ref))
		return -FI_EBUSY;

	HASH_CLEAR(hh, av->av_entry_hash);
	ofi_bufpool_destroy(av->av_entry_pool);
	free(av);

	ofi_atomic_dec32(&dom->ref);

	return FI_SUCCESS;
}

static struct fi_ops_av cxip_av_fid_ops = {
	.size = sizeof(struct fi_ops_av),
	.insert = cxip_av_insert,
	.insertsvc = cxip_av_insertsvc,
	.insertsym = fi_no_av_insertsym,
	.remove = cxip_av_remove,
	.lookup = cxip_av_lookup,
	.straddr = cxip_av_straddr,
	.av_set = cxip_av_set,
};

static struct fi_ops cxip_av_fi_ops = {
	.size = sizeof(struct fi_ops),
	.close = cxip_av_close,
	.bind = fi_no_bind,
	.control = fi_no_control,
	.ops_open = fi_no_ops_open,
};

static int cxip_av_open_validate_args(struct fid_domain *domain,
				      struct fi_av_attr *attr,
				      struct fid_av **avp, void *context)
{
	if (!attr) {
		CXIP_WARN("NULL AV attributes\n");
		return -FI_EINVAL;
	}

	if (!avp) {
		CXIP_WARN("NULL AV\n");
		return -FI_EINVAL;
	}

	if (attr->rx_ctx_bits) {
		CXIP_WARN("rx_ctx_bits non-zero. SEPs not supported.\n");
		return -FI_EINVAL;
	}

	if (attr->name) {
		CXIP_WARN("Shared AVs not supported\n");
		return -FI_EINVAL;
	}

	if (attr->flags & FI_READ) {
		CXIP_WARN("FI_READ and shared AVs not supported\n");
		return -FI_EINVAL;
	}

	if (attr->flags & FI_EVENT) {
		CXIP_WARN("FI_EVENT not supported\n");
		return -FI_EINVAL;
	}

	switch (attr->type) {
	case FI_AV_UNSPEC:
	case FI_AV_MAP:
	case FI_AV_TABLE:
		break;
	default:
		CXIP_WARN("Invalid AV type: %d\n", attr->type);
		return -FI_EINVAL;
	}

	return FI_SUCCESS;
}

int cxip_av_open(struct fid_domain *domain, struct fi_av_attr *attr,
		 struct fid_av **avp, void *context)
{
	int ret;
	struct cxip_av *av;
	struct cxip_domain *dom;
	struct ofi_bufpool_attr pool_attr = {
		.size = sizeof(struct cxip_av_entry),
		.flags = OFI_BUFPOOL_NO_TRACK | OFI_BUFPOOL_INDEXED,
	};
	size_t orig_size;

	ret = cxip_av_open_validate_args(domain, attr, avp, context);
	if (ret != FI_SUCCESS)
		goto err;

	dom = container_of(domain, struct cxip_domain, util_domain.domain_fid);

	av = calloc(1, sizeof(*av));
	if (!av) {
		ret = -FI_ENOMEM;
		goto err;
	}

	/* Initialize embedded AV fields. */
	av->av_fid.fid.context = context;
	av->av_fid.fid.fclass = FI_CLASS_AV;
	av->av_fid.fid.ops = &cxip_av_fi_ops;
	av->av_fid.ops = &cxip_av_fid_ops;
	av->domain = dom;
	dlist_init(&av->ep_list);
	ofi_atomic_initialize32(&av->ref, 0);
	av->lockless = dom->util_domain.threading == FI_THREAD_DOMAIN;
	pthread_rwlock_init(&av->lock, NULL);
	av->av_entry_hash = NULL;
	av->symmetric = !!(attr->flags & FI_SYMMETRIC);
	ofi_atomic_initialize32(&av->av_entry_cnt, 0);

	/* Only FI_AV_TABLE is implemented. */
	av->type = attr->type == FI_AV_UNSPEC ? FI_AV_TABLE : attr->type;

	/* Allocate buffer pool and size it based on user input. */
	orig_size = attr->count ? attr->count : ofi_universe_size;
	orig_size = roundup_power_of_two(orig_size);
	pool_attr.chunk_cnt = orig_size;
	ret = ofi_bufpool_create_attr(&pool_attr, &av->av_entry_pool);
	if (ret) {
		CXIP_WARN("Faild to allocate buffer pool: %d\n", ret);
		goto err_free_av;
	}

	ofi_atomic_inc32(&dom->ref);

	*avp = &av->av_fid;

	return FI_SUCCESS;

err_free_av:
	free(av);
err:
	return ret;
}
