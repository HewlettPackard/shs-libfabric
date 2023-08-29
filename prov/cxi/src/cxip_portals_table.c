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

int cxip_portals_table_alloc(struct cxip_lni *lni, uint32_t vni, uint32_t pid,
			     struct cxip_portals_table **ptable)
{
	struct cxip_portals_table *table;
	int ret;

	table = malloc(sizeof(*table));
	if (!table) {
		CXIP_WARN("Failed to allocate IF domain\n");
		return -FI_ENOMEM;
	}

	ret = cxil_alloc_domain(lni->lni, vni, pid, &table->dom);
	if (ret) {
		CXIP_WARN("Failed to allocate CXI Domain, ret: %d\n", ret);
		ret = -FI_ENOSPC;
		goto free_table;
	}

	table->lni = lni;

	CXIP_DBG("Allocated portals table, %s VNI: %u PID: %u\n",
		 lni->iface->info->device_name, vni, table->dom->pid);

	*ptable = table;

	return FI_SUCCESS;

free_table:
	free(table);

	return ret;
}

/*
 * cxip_free_if_domain() - Free an IF Domain.
 */
void cxip_portals_table_free(struct cxip_portals_table *ptable)
{
	int ret;

	CXIP_DBG("Freeing portals table, %s VNI: %u PID: %u\n",
		 ptable->lni->iface->info->device_name, ptable->dom->vni,
		 ptable->dom->pid);

	ret = cxil_destroy_domain(ptable->dom);
	if (ret)
		CXIP_WARN("Failed to destroy domain: %d\n", ret);

	free(ptable);
}
