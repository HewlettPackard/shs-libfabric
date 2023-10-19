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

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <ctype.h>

#include <criterion/criterion.h>
#include <criterion/parameterized.h>

#include "libcxi/libcxi.h"
#include "cxip.h"
#include "cxip_test_common.h"

TestSuite(fid_nic, .timeout = 5);

Test(fid_nic, validate_nic_attr)
{
	int ret;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	uint16_t valid_vni = 0x120;
	struct fi_info *info;
	struct cxip_nic_attr *nic_attr;

	/* Need to allocate a service to be used by libfabric. */
	ret = cxil_open_device(0, &dev);
	cr_assert_eq(ret, 0, "cxil_open_device failed: %d", ret);

	svc_desc.restricted_vnis = 1;
	svc_desc.enable = 1;
	svc_desc.num_vld_vnis = 1;
	svc_desc.vnis[0] = valid_vni;

	ret = cxil_alloc_svc(dev, &svc_desc, &fail_info);
	cr_assert_gt(ret, 0, "cxil_alloc_svc failed: %d", ret);
	svc_desc.svc_id = ret;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	nic_attr = (struct cxip_nic_attr *)info->nic->prov_attr;
	cr_assert_eq(nic_attr->version, 1);
	cr_assert_eq(nic_attr->addr, dev->info.nid);
	cr_assert_eq(nic_attr->default_rgroup_id, svc_desc.svc_id);
	cr_assert_eq(nic_attr->default_vni, valid_vni);

	fi_freeinfo(info);
	ret = cxil_destroy_svc(dev, svc_desc.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);
	cxil_close_device(dev);
}
