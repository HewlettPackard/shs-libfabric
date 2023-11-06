/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2019 Cray Inc. All rights reserved.
 */

#include "cxip.h"

#define CXIP_DBG(...) _CXIP_DBG(FI_LOG_MR, __VA_ARGS__)
#define CXIP_WARN(...) _CXIP_WARN(FI_LOG_MR, __VA_ARGS__)
#define CXIP_INFO(...) _CXIP_INFO(FI_LOG_MR, __VA_ARGS__)

#define MAP_FAIL_MSG "cxil_map lni: %d base: 0x%p len: %ld " \
		     "map_flags: 0x%0X failure: %d, %s\n"

/**
 * cxip_do_map() - IO map a buffer.
 */
static int cxip_do_map(struct ofi_mr_cache *cache, struct ofi_mr_entry *entry)
{
	int ret;
	struct cxip_md *md = (struct cxip_md *)entry->data;
	struct cxip_domain *dom;
	uint32_t map_flags = CXI_MAP_READ | CXI_MAP_WRITE;
	struct cxi_md_hints hints;
	void *ze_handle;
	void *ze_base_addr;
	size_t ze_base_size;
	uint64_t hmem_flags = entry->info.flags;

	dom = container_of(cache, struct cxip_domain, iomm);

	/* Prefer the ATS (scalable MD) whenever possible
	 *
	 * TODO: ATS (scalable MD) can only support CPU page sizes and should be
	 * avoided for non-standard page sizes.
	 */
	if (dom->scalable_iomm && entry->info.iface == FI_HMEM_SYSTEM) {
		md->md = dom->scalable_md.md;
		md->dom = dom;
		md->info = entry->info;

		return FI_SUCCESS;
	}

	memset(&hints, 0, sizeof(hints));

	if (entry->info.iface == FI_HMEM_SYSTEM) {
		if (dom->ats)
			map_flags |= CXI_MAP_ATS;

		if (!dom->odp)
			map_flags |= CXI_MAP_PIN;
	} else {
		/* TODO: Remove PIN when DMA buf move_notify is supported. */
		map_flags |= CXI_MAP_DEVICE | CXI_MAP_PIN;

		/* ZE support requires the use of the DMA buf FD and offset
		 * hints fields.
		 */
		if (entry->info.iface == FI_HMEM_ZE) {
			if (!cxip_env.ze_hmem_supported) {
				CXIP_WARN("ZE device memory not supported. Try disabling implicit scaling (EnableImplicitScaling=0 NEOReadDebugKeys=1).\n");
				return -FI_ENOSYS;
			}

			ret = ze_hmem_get_handle(entry->info.iov.iov_base,
						 entry->info.iov.iov_len,
						 &ze_handle);
			if (ret) {
				CXIP_WARN("ze_hmem_get_handle failed: %d:%s\n",
					  ret, fi_strerror(-ret));
				goto err;
			}

			ret = ze_hmem_get_base_addr(entry->info.iov.iov_base,
						    entry->info.iov.iov_len,
						    &ze_base_addr,
						    &ze_base_size);
			if (ret) {
				CXIP_WARN("ze_hmem_get_base_addr failed: %d:%s\n",
					  ret, fi_strerror(-ret));
				goto err;
			}

			hints.dmabuf_fd = (int)(uintptr_t)ze_handle;
			hints.dmabuf_offset =
				(uintptr_t)entry->info.iov.iov_base -
				(uintptr_t)ze_base_addr;
			hints.dmabuf_valid = true;
		}
	}

	if (!cxip_env.iotlb)
		map_flags |= CXI_MAP_NOCACHE;

	ret = cxil_map(dom->lni->lni, entry->info.iov.iov_base,
		       entry->info.iov.iov_len, map_flags, &hints, &md->md);
	if (ret) {
		CXIP_WARN(MAP_FAIL_MSG, dom->lni->lni->id,
			  entry->info.iov.iov_base, entry->info.iov.iov_len,
			  map_flags,  ret, fi_strerror(-ret));
		goto err;
	}

	/* zeHostMalloc() returns FI_HMEM_ZE but this cannot currently be
	 * registered with ofi_hmem_dev_register(). Thus skip it.
	 */
	if (cxip_env.disable_hmem_dev_register ||
	    ((entry->info.iface == FI_HMEM_ZE) &&
	      (hmem_flags & FI_HMEM_HOST_ALLOC)))
		ret = -FI_ENOSYS;
	else
		ret = ofi_hmem_dev_register(entry->info.iface,
					    entry->info.iov.iov_base,
					    entry->info.iov.iov_len,
					    &md->handle);
	switch (ret) {
	case FI_SUCCESS:
		break;

	case -FI_ENOSYS:
		md->handle = NO_DEV_REG_HANDLE;
		md->host_addr = NULL;
		break;

	default:
		CXIP_WARN("ofi_hmem_dev_register %s failed: %d:%s\n",
			  fi_tostr(&entry->info.iface, FI_TYPE_HMEM_IFACE), ret,
			  fi_strerror(-ret));
		goto err_unmap;
	}

	md->dom = dom;
	md->info = entry->info;
	md->cached = true;
	CXIP_DBG("addr:%p end:%p len:0x%lx iova:%llx lac:%d device:%d\n",
		 entry->info.iov.iov_base,
		 (char *)entry->info.iov.iov_base + entry->info.iov.iov_len,
		 entry->info.iov.iov_len, md->md->iova, md->md->lac,
		 !!(map_flags & CXI_MAP_DEVICE));

	return FI_SUCCESS;

err_unmap:
	cxil_unmap(md->md);
err:
	md->dom = NULL;
	return ret;
}

/**
 * cxip_do_unmap() - IO unmap a buffer.
 */
static void cxip_do_unmap(struct ofi_mr_cache *cache,
			  struct ofi_mr_entry *entry)
{
	int ret;
	struct cxip_md *md = (struct cxip_md *)entry->data;

	if (!md || !md->dom || md->md == md->dom->scalable_md.md)
		return;

	if (md->handle != NO_DEV_REG_HANDLE)
		ofi_hmem_dev_unregister(entry->info.iface, md->handle);

	ret = cxil_unmap(md->md);
	if (ret)
		CXIP_WARN("cxil_unmap failed: %d\n", ret);

	CXIP_DBG("addr:%p end:%p len:0x%lx iova:%llx lac:%d\n",
		 entry->info.iov.iov_base,
		 (char *)entry->info.iov.iov_base + entry->info.iov.iov_len,
		 entry->info.iov.iov_len, md->md->iova, md->md->lac);
}

static int cxip_scalable_iomm_init(struct cxip_domain *dom)
{
	int ret;
	uint32_t map_flags = (CXI_MAP_READ | CXI_MAP_WRITE | CXI_MAP_ATS);

	if (!cxip_env.iotlb)
		map_flags |= CXI_MAP_NOCACHE;

	ret = cxil_map(dom->lni->lni, 0, 0xfffffffffffff000, map_flags, NULL,
		       &dom->scalable_md.md);
	if (!ret) {
		dom->scalable_md.dom = dom;
		dom->scalable_iomm = true;

		CXIP_DBG("Scalable IOMM enabled.\n");

		if (cxip_env.ats_mlock_mode == CXIP_ATS_MLOCK_ALL) {
			ret = mlockall(MCL_CURRENT | MCL_FUTURE);
			if (ret) {
				CXIP_WARN("mlockall(MCL_CURRENT | MCL_FUTURE) failed: %d\n",
					  -errno);
			}
		}

		ret = FI_SUCCESS;
	} else {
		ret = -FI_ENOSYS;
	}

	return ret;
}

static void cxip_scalable_iomm_fini(struct cxip_domain *dom)
{
	cxil_unmap(dom->scalable_md.md);
}

static int cxip_ats_check(struct cxip_domain *dom)
{
	uint32_t map_flags = CXI_MAP_READ | CXI_MAP_WRITE | CXI_MAP_ATS |
			     CXI_MAP_PIN;
	int stack_var;
	struct cxi_md *md;
	int ret;

	ret = cxil_map(dom->lni->lni, &stack_var, sizeof(stack_var), map_flags,
		       NULL, &md);
	if (!ret) {
		cxil_unmap(md);
		CXIP_INFO("PCIe ATS supported.\n");
		return 1;
	}

	CXIP_INFO("PCIe ATS not supported.\n");
	return 0;
}

static void cxip_iomm_set_rocr_dev_mem_only(struct cxip_domain *dom)
{
	int dev_hmem_count = 0;
	bool rocr_support = false;
	int i;

	if (!dom->hmem) {
		dom->rocr_dev_mem_only = false;
		return;
	}

	for (i = 0; i < OFI_HMEM_MAX; i++) {
		if (i == FI_HMEM_SYSTEM)
			continue;

		if (hmem_ops[i].initialized) {
			dev_hmem_count++;

			if (i == FI_HMEM_ROCR)
				rocr_support = true;
		}
	}

	/* If FI_HMEM_ROCR is the ONLY device supported by libfabric and the
	 * core ROCR memory monitor is used, cxip_map can be optimized to avoid
	 * pointer queries.
	 */
	if (dev_hmem_count == 1 && rocr_support &&
	    default_rocr_monitor == rocr_monitor)
		dom->rocr_dev_mem_only = true;
	else
		dom->rocr_dev_mem_only = false;
}

/*
 * cxip_iomm_init() - Initialize domain IO memory map.
 */
int cxip_iomm_init(struct cxip_domain *dom)
{
	struct ofi_mem_monitor *memory_monitors[OFI_HMEM_MAX] = {
		[FI_HMEM_SYSTEM] = default_monitor,
		[FI_HMEM_CUDA] = default_cuda_monitor,
		[FI_HMEM_ROCR] = default_rocr_monitor,
		[FI_HMEM_ZE] = default_ze_monitor,
	};
	enum fi_hmem_iface iface;
	int ret;
	bool scalable;

	/* Check if ATS is supported */
	if (cxip_env.ats && cxip_ats_check(dom))
		dom->ats = true;

	if (cxip_env.odp && !(dom->util_domain.mr_mode & FI_MR_ALLOCATED))
		dom->odp = true;

	if (dom->util_domain.info_domain_caps & FI_HMEM)
		dom->hmem = true;

	scalable = dom->ats && dom->odp;

	CXIP_INFO("Domain ATS: %d ODP: %d HMEM: %d Scalable: %d\n",
		  dom->ats, dom->odp, dom->hmem, scalable);

	/* Unpinned ATS translation is scalable. A single MD covers all
	 * memory addresses and a cache isn't necessary.
	 */
	if (scalable) {
		ret = cxip_scalable_iomm_init(dom);
		if (ret) {
			CXIP_WARN("cxip_scalable_iomm_init() returned: %d\n",
				  ret);
			return ret;
		}
	}

	if (!scalable || dom->hmem) {
		dom->iomm.entry_data_size = sizeof(struct cxip_md);
		dom->iomm.add_region = cxip_do_map;
		dom->iomm.delete_region = cxip_do_unmap;
		ret = ofi_mr_cache_init(&dom->util_domain, memory_monitors,
					&dom->iomm);
		if (ret) {
			CXIP_INFO("MR cache init failed: %s. MR caching disabled.\n",
				  fi_strerror(-ret));
		} else {
			for (iface = 0; iface < OFI_HMEM_MAX; iface++) {
				if (dom->iomm.monitors[iface])
					CXIP_INFO("MR cache enabled for %s memory\n",
						  fi_tostr(&iface, FI_TYPE_HMEM_IFACE));
			}
		}
	}

	cxip_iomm_set_rocr_dev_mem_only(dom);

	return FI_SUCCESS;
}

/*
 * cxip_iomm_fini() - Finalize domain IO memory map.
 */
void cxip_iomm_fini(struct cxip_domain *dom)
{
	if (dom->scalable_iomm)
		cxip_scalable_iomm_fini(dom);

	if (!dom->scalable_iomm || dom->hmem)
		ofi_mr_cache_cleanup(&dom->iomm);
}

static int cxip_map_cache(struct cxip_domain *dom, struct ofi_mr_info *info,
			  struct cxip_md **md)
{
	struct ofi_mr_entry *entry;
	int ret;

	ret = ofi_mr_cache_search(&dom->iomm, info, &entry);
	if (ret) {
		CXIP_WARN("Failed to acquire mapping (%p, %lu): %d\n",
 			  info->iov.iov_base, info->iov.iov_len, ret);
		return ret;
	}

	*md = (struct cxip_md *)entry->data;

	return FI_SUCCESS;
}

static int cxip_map_nocache(struct cxip_domain *dom, struct fi_mr_attr *attr,
			    uint64_t hmem_flags, struct cxip_md **md)
{
	struct cxip_md *uncached_md;
	uint32_t map_flags;
	int ret;
	struct cxi_md_hints hints;
	void *ze_handle;
	void *ze_base_addr;
	size_t ze_base_size;

	/* Prefer the ATS (scalable MD) whenever possible
	 *
	 * TODO: ATS (scalable MD) can only support CPU page sizes and should be
	 * avoided for non-standard page sizes.
	 */
	if (dom->scalable_iomm && attr->iface == FI_HMEM_SYSTEM) {
		*md = &dom->scalable_md;
		return FI_SUCCESS;
	}

	memset(&hints, 0, sizeof(hints));

	uncached_md = calloc(1, sizeof(*uncached_md));
	if (!uncached_md)
		return -FI_ENOMEM;

	map_flags = CXI_MAP_READ | CXI_MAP_WRITE;
	if (attr->iface == FI_HMEM_SYSTEM) {
		if (dom->ats)
			map_flags |= CXI_MAP_ATS;

		if (!dom->odp)
			map_flags |= CXI_MAP_PIN;
	} else {
		/* TODO: Remove PIN when DMA buf move_notify is supported. */
		map_flags |= CXI_MAP_DEVICE | CXI_MAP_PIN;

		/* ZE support requires the use of the DMA buf FD and offset
		 * hints fields.
		 */
		if (attr->iface == FI_HMEM_ZE) {
			if (!cxip_env.ze_hmem_supported) {
				CXIP_WARN("ZE device memory not supported. Try disabling implicit scaling (EnableImplicitScaling=0 NEOReadDebugKeys=1).\n");
				ret = -FI_ENOSYS;
				goto err_free_uncached_md;
			}

			ret = ze_hmem_get_handle(attr->mr_iov->iov_base,
						 attr->mr_iov->iov_len,
						 &ze_handle);
			if (ret) {
				CXIP_WARN("ze_hmem_get_handle failed: %d:%s\n",
					  ret, fi_strerror(-ret));
				goto err_free_uncached_md;
			}

			ret = ze_hmem_get_base_addr(attr->mr_iov->iov_base,
						    attr->mr_iov->iov_len,
						    &ze_base_addr,
						    &ze_base_size);
			if (ret) {
				CXIP_WARN("ze_hmem_get_base_addr failed: %d:%s\n",
					  ret, fi_strerror(-ret));
				goto err_free_uncached_md;
			}

			hints.dmabuf_fd = (int)(uintptr_t)ze_handle;
			hints.dmabuf_offset =
				(uintptr_t)attr->mr_iov->iov_base -
				(uintptr_t)ze_base_addr;
			hints.dmabuf_valid = true;
		}
	}

	if (!cxip_env.iotlb)
		map_flags |= CXI_MAP_NOCACHE;

	ret = cxil_map(dom->lni->lni, attr->mr_iov->iov_base,
		       attr->mr_iov->iov_len, map_flags, &hints,
		       &uncached_md->md);
	if (ret) {
		CXIP_WARN("cxil_map failed: %d:%s\n", ret, fi_strerror(-ret));
		goto err_free_uncached_md;
	}

	/* zeHostMalloc() returns FI_HMEM_ZE but this cannot currently be
	 * registered with ofi_hmem_dev_register(). Thus skip it.
	 */
	if (cxip_env.disable_hmem_dev_register ||
	    ((attr->iface == FI_HMEM_ZE) && (hmem_flags & FI_HMEM_HOST_ALLOC)))
		ret = -FI_ENOSYS;
	else
		ret = ofi_hmem_dev_register(attr->iface,
					    (const void *)uncached_md->md->va,
					    uncached_md->md->len,
					    &uncached_md->handle);

	switch (ret) {
	case FI_SUCCESS:
		break;

	case -FI_ENOSYS:
		uncached_md->handle = NO_DEV_REG_HANDLE;
		uncached_md->host_addr = NULL;
		break;

	default:
		CXIP_WARN("ofi_hmem_dev_register %s failed: %d:%s\n",
			  fi_tostr(&attr->iface, FI_TYPE_HMEM_IFACE), ret,
			  fi_strerror(-ret));
		goto err_unmap;
	}

	uncached_md->dom = dom;
	uncached_md->info.iov.iov_base = (void *)uncached_md->md->va;
	uncached_md->info.iov.iov_len = uncached_md->md->len;
	uncached_md->info.iface = attr->iface;

	*md = uncached_md;

	return FI_SUCCESS;

err_unmap:
	cxil_unmap(uncached_md->md);
err_free_uncached_md:
	free(uncached_md);

	return ret;
}

static void cxip_map_get_mem_region_size(const void *buf, unsigned long len,
					 enum fi_hmem_iface iface,
					 void **out_buf, unsigned long *out_len)
{
	int ret;

	ret = ofi_hmem_get_base_addr(iface, buf, len, out_buf, out_len);
	if (ret) {
		*out_buf = (void *)buf;
		*out_len = len;
	}

	CXIP_DBG("%s: User addr=%p User len=%lu Region addr=%p Region len=%lu\n",
		 fi_tostr(&iface, FI_TYPE_HMEM_IFACE), buf, len, *out_buf,
		 *out_len);
}

/*
 * cxip_map() - Acquire IO mapping for buf.
 *
 * The IO memory map is searched for a IO mapping which covers buf. If no
 * mapping has been established, create one and cache it.
 */
int cxip_map(struct cxip_domain *dom, const void *buf, unsigned long len,
	     uint64_t flags, struct cxip_md **md)
{
	struct iovec iov = {
		.iov_base = (void *)buf,
		.iov_len = len,
	};
	struct fi_mr_attr attr = {
		.iov_count = 1,
		.mr_iov = &iov,
	};
	struct ofi_mr_info mr_info = {};
	uint64_t hmem_flags = 0;
	struct ofi_mr_entry *entry;
	bool cache = !(flags & OFI_MR_NOCACHE);

	/* TODO: ATS (scalable MD) can only support CPU page sizes and should be
	 * avoided for non-standard page sizes.
	 */
	if (dom->scalable_iomm && !dom->hmem) {
		*md = &dom->scalable_md;
		return FI_SUCCESS;
	}

	/* Since the MR cache find operates on virtual addresses and all device
	 * memory must support a unified virtual address space with system
	 * memory, the buffer pointer query can be avoided completely if the
	 * corresponding entry is in the cache.
	 */
	if (cache && cxip_domain_mr_cache_enabled(dom)) {
		entry = ofi_mr_cache_find(&dom->iomm, &attr, 0);
		if (entry) {
			*md = (struct cxip_md *)entry->data;
			return FI_SUCCESS;
		}
	}

	/* Since the MR cache search will allocate a new entry, the MR iface
	 * attribute must be defined for the proper MR cache memory monitor to
	 * be selected.
	 */
	if (dom->hmem)
		attr.iface = ofi_get_hmem_iface(buf, NULL, &hmem_flags);

	if (cache && cxip_domain_mr_cache_iface_enabled(dom, attr.iface)) {
		cxip_map_get_mem_region_size(iov.iov_base, iov.iov_len,
					     attr.iface, &iov.iov_base,
					     &iov.iov_len);

		mr_info.iface = attr.iface;
		mr_info.iov = iov;

		/* Overload IPC addr to pass in HMEM flags. */
		mr_info.flags = hmem_flags;

		return cxip_map_cache(dom, &mr_info, md);
	}

	return cxip_map_nocache(dom, &attr, flags, md);
}

static void cxip_unmap_cache(struct cxip_md *md)
{
	struct ofi_mr_entry *entry =
		container_of(md, struct ofi_mr_entry, data);

	ofi_mr_cache_delete(&md->dom->iomm, entry);
}

static void cxip_unmap_nocache(struct cxip_md *md)
{
	int ret;

	if (md->handle != NO_DEV_REG_HANDLE)
		ofi_hmem_dev_unregister(md->info.iface, md->handle);

	ret = cxil_unmap(md->md);
	if (ret)
		CXIP_WARN("cxil_unmap failed: %d\n", ret);

	free(md);
}

/*
 * cxip_unmap() - Release an IO mapping.
 *
 * Drop a refernce to the IO mapping. If this was the last reference, the
 * buffer may be unmapped.
 */
void cxip_unmap(struct cxip_md *md)
{
	/* Scalable MD is owned by the CXIP domain and thus will be freed when
	 * the domain is closed.
	 */
	if (md == &md->dom->scalable_md)
		return;

	if (md->cached)
		cxip_unmap_cache(md);
	else
		cxip_unmap_nocache(md);
}
