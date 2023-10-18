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
#include "config.h"
#include "cxip.h"
#include "ofi.h"

static int cxip_nic_close(struct fid *fid)
{
	return ofi_nic_close(fid);
}

static int cxip_nic_control(struct fid *fid, int command, void *arg)
{
	return ofi_nic_control(fid, command, arg);
}

static int cxip_nic_tostr(const struct fid *fid_nic, char *buf, size_t len)
{
	return ofi_nic_tostr(fid_nic, buf, len);
}

static struct fi_ops cxip_nic_ops = {
	.size = sizeof(struct fi_ops),
	.close = cxip_nic_close,
	.control = cxip_nic_control,
	.tostr = cxip_nic_tostr,
};

int cxip_nic_alloc(struct cxip_if *nic_if, struct fid_nic **fid_nic)
{
	struct fid_nic *nic;
	int ret;

	/* Reuse the common fid_nic as must as possible. */
	nic = ofi_nic_dup(NULL);
	if (!nic)
		return -FI_ENOMEM;

	/* Update the fid_nic to point to our operations. */
	nic->fid.ops = &cxip_nic_ops;

	nic->device_attr->name = strdup(nic_if->info->device_name);
	if (!nic->device_attr->name) {
		ret = -FI_ENOMEM;
		goto err_free_nic;
	}

	ret = asprintf(&nic->device_attr->device_id, "0x%x",
		       nic_if->info->device_id);
	if (ret < 0)
		goto err_free_nic;

	ret = asprintf(&nic->device_attr->device_version, "%u",
		       nic_if->info->device_rev);
	if (ret < 0)
		goto err_free_nic;

	ret = asprintf(&nic->device_attr->vendor_id, "0x%x",
		       nic_if->info->vendor_id);
	if (ret < 0)
		goto err_free_nic;

	nic->device_attr->driver = strdup(nic_if->info->driver_name);

	nic->bus_attr->bus_type = FI_BUS_PCI;
	nic->bus_attr->attr.pci.domain_id = nic_if->info->pci_domain;
	nic->bus_attr->attr.pci.bus_id = nic_if->info->pci_bus;
	nic->bus_attr->attr.pci.device_id = nic_if->info->pci_device;
	nic->bus_attr->attr.pci.function_id = nic_if->info->pci_function;

	ret = asprintf(&nic->link_attr->address, "0x%x",
		       nic_if->info->nic_addr);
	if (ret < 0)
		goto err_free_nic;

	nic->link_attr->mtu = nic_if->info->link_mtu;
	/* Convert Mb/s to libfabric reported b/s */
	nic->link_attr->speed = (size_t)nic_if->speed * 1000000;
	nic->link_attr->state = nic_if->link ?  FI_LINK_UP : FI_LINK_DOWN;
	nic->link_attr->network_type = strdup("HPC Ethernet");

	*fid_nic = nic;

	return FI_SUCCESS;

err_free_nic:
	cxip_nic_close(&nic->fid);

	return ret;
}
