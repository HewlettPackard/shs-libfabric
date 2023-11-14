/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2022 Hewlett Packard Enterprise Development LP
 */
#include <stdio.h>
#include <stdlib.h>
#include <criterion/criterion.h>

#include "cxip.h"
#include "cxip_test_common.h"

void *memdup(const void *src, size_t n)
{
	void *dest;

	dest = malloc(n);
	if (dest == NULL)
		return NULL;

	return memcpy(dest, src, n);
}

TestSuite(auth_key, .timeout = CXIT_DEFAULT_TIMEOUT);

/* Test fi_getinfo() verification of hints argument. */
Test(auth_key, invalid_auth_key_size_domain_attr_hints)
{
	struct cxi_auth_key auth_key = {
		.svc_id = CXI_DEFAULT_SVC_ID,
		.vni = 1,
	};
	int ret;
	struct fi_info *hints;
	struct fi_info *info;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->auth_key_size = 1;
	hints->domain_attr->auth_key = memdup(&auth_key, 1);
	cr_assert_not_null(hints->domain_attr->auth_key, "memdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, -FI_ENODATA, "fi_getinfo failed: %d", ret);

	fi_freeinfo(hints);
}

/* Test fi_getinfo() verification of hints argument. */
Test(auth_key, missing_auth_key_size_domain_attr_hints)
{
	struct cxi_auth_key auth_key = {
		.svc_id = CXI_DEFAULT_SVC_ID,
		.vni = 1,
	};
	int ret;
	struct fi_info *hints;
	struct fi_info *info;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints->fabric_attr->prov_name, "strdup failed");

	hints->domain_attr->auth_key = memdup(&auth_key, 1);
	cr_assert_not_null(hints->domain_attr->auth_key, "memdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, -FI_ENODATA, "fi_getinfo failed: %d", ret);

	fi_freeinfo(hints);
}

/* Test fi_getinfo() verification of hints argument. */
Test(auth_key, invalid_auth_key_size_ep_attr_hints)
{
	struct cxi_auth_key auth_key = {
		.svc_id = CXI_DEFAULT_SVC_ID,
		.vni = 1,
	};
	int ret;
	struct fi_info *hints;
	struct fi_info *info;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints->fabric_attr->prov_name, "strdup failed");

	hints->ep_attr->auth_key_size = 1;
	hints->ep_attr->auth_key = memdup(&auth_key, 1);
	cr_assert_not_null(hints->ep_attr->auth_key, "memdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, -FI_ENODATA, "fi_getinfo failed: %d", ret);

	fi_freeinfo(hints);
}

/* Test fi_getinfo() verification of hints argument. */
Test(auth_key, missing_auth_key_size_ep_attr_hints)
{
	struct cxi_auth_key auth_key = {
		.svc_id = CXI_DEFAULT_SVC_ID,
		.vni = 1,
	};
	int ret;
	struct fi_info *hints;
	struct fi_info *info;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints->fabric_attr->prov_name, "strdup failed");

	hints->ep_attr->auth_key = memdup(&auth_key, 1);
	cr_assert_not_null(hints->ep_attr->auth_key, "memdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, -FI_ENODATA, "fi_getinfo failed: %d", ret);

	fi_freeinfo(hints);
}

/* Verify fi_getinfo() correctly echos back a valid auth_key hint using the
 * default svc_id.
 */
Test(auth_key, valid_default_domain_auth_key_hint)
{
	struct cxi_auth_key auth_key = {
		.svc_id = CXI_DEFAULT_SVC_ID,
		.vni = 1,
	};
	int ret;
	struct fi_info *hints;
	struct fi_info *info;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints->fabric_attr->prov_name, "strdup failed");

	hints->domain_attr->auth_key = memdup(&auth_key, sizeof(auth_key));
	cr_assert_not_null(hints->domain_attr->auth_key, "memdup failed");

	hints->domain_attr->auth_key_size = sizeof(auth_key);
	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	cr_assert_not_null(info->domain_attr->auth_key, "NULL domain auth_key");
	cr_assert_eq(hints->domain_attr->auth_key_size,
		     info->domain_attr->auth_key_size,
		     "fi_getinfo returned auth_key_size does not match hints");

	ret = memcmp(hints->domain_attr->auth_key, info->domain_attr->auth_key,
		     hints->domain_attr->auth_key_size);
	cr_assert_eq(ret, 0, "fi_getinfo returned auth_key does not match hints");

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

/* Verify fi_getinfo() correctly echos back a valid auth_key hint using the
 * default svc_id.
 */
Test(auth_key, valid_default_ep_auth_key_hint)
{
	struct cxi_auth_key auth_key = {
		.svc_id = CXI_DEFAULT_SVC_ID,
		.vni = 1,
	};
	int ret;
	struct fi_info *hints;
	struct fi_info *info;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints->fabric_attr->prov_name, "strdup failed");

	hints->ep_attr->auth_key = memdup(&auth_key, sizeof(auth_key));
	cr_assert_not_null(hints->ep_attr->auth_key, "memdup failed");

	hints->ep_attr->auth_key_size = sizeof(auth_key);
	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	cr_assert_not_null(info->ep_attr->auth_key, "NULL ep auth_key");
	cr_assert_eq(hints->ep_attr->auth_key_size,
		     info->ep_attr->auth_key_size,
		     "fi_getinfo returned auth_key_size does not match hints");

	ret = memcmp(hints->ep_attr->auth_key, info->ep_attr->auth_key,
		     hints->ep_attr->auth_key_size);
	cr_assert_eq(ret, 0, "fi_getinfo returned auth_key does not match hints");

	/* Since hints domain auth_key is NULL, CXI provider should echo the
	 * hints ep auth_key into info domain auth_key. This is the behavior
	 * some MPICH versions expect.
	 */
	cr_assert_not_null(info->domain_attr->auth_key, "NULL domain auth_key");
	cr_assert_eq(hints->ep_attr->auth_key_size,
		     info->domain_attr->auth_key_size,
		     "fi_getinfo returned auth_key_size does not match hints");

	ret = memcmp(hints->ep_attr->auth_key, info->domain_attr->auth_key,
		     hints->ep_attr->auth_key_size);
	cr_assert_eq(ret, 0, "fi_getinfo returned auth_key does not match hints");

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

/* Verify fi_getinfo() rejects a svc_id which has not been allocated thus making
 * the auth_key invalid.
 */
Test(auth_key, invalid_user_defined_domain_svc_id_hint)
{
	struct cxi_auth_key auth_key = {
		.svc_id = 0xffff,
		.vni = 1,
	};
	int ret;
	struct fi_info *hints;
	struct fi_info *info;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints->fabric_attr->prov_name, "strdup failed");

	hints->domain_attr->auth_key = memdup(&auth_key, sizeof(auth_key));
	cr_assert_not_null(hints->domain_attr->auth_key, "memdup failed");

	hints->domain_attr->auth_key_size = sizeof(auth_key);
	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, -FI_ENODATA, "fi_getinfo failed: %d", ret);

	fi_freeinfo(hints);
}

/* Verify fi_getinfo() rejects a svc_id which has not been allocated thus making
 * the auth_key invalid.
 */
Test(auth_key, invalid_user_defined_ep_svc_id_hint)
{
	struct cxi_auth_key auth_key = {
		.svc_id = 0xffff,
		.vni = 1,
	};
	int ret;
	struct fi_info *hints;
	struct fi_info *info;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints->fabric_attr->prov_name, "strdup failed");

	hints->ep_attr->auth_key = memdup(&auth_key, sizeof(auth_key));
	cr_assert_not_null(hints->ep_attr->auth_key, "memdup failed");

	hints->ep_attr->auth_key_size = sizeof(auth_key);
	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, -FI_ENODATA, "fi_getinfo failed: %d", ret);

	fi_freeinfo(hints);
}

/* Verify fi_domain() rejects an invalid auth_key. */
Test(auth_key, invalid_user_defined_domain_svc_id)
{
	struct cxi_auth_key auth_key = {
		.svc_id = 0xffff,
		.vni = 1,
	};
	int ret;
	struct fi_info *info;
	struct fid_fabric *fab;
	struct fid_domain *dom;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	ret = fi_fabric(info->fabric_attr, &fab, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_fabric failed: %d", ret);

	/* Override auth_key with bad auth_key. */
	if (info->domain_attr->auth_key)
		free(info->domain_attr->auth_key);
	info->domain_attr->auth_key = memdup(&auth_key, sizeof(auth_key));
	info->domain_attr->auth_key_size = sizeof(auth_key);

	ret = fi_domain(fab, info, &dom, NULL);
	cr_assert_eq(ret, -FI_EINVAL, "fi_domain failed: %d", ret);

	fi_close(&fab->fid);
	fi_freeinfo(info);
}

/* Verify fi_endpoint() rejects an invalid auth_key. */
Test(auth_key, invalid_user_defined_ep_svc_id)
{
	struct cxi_auth_key auth_key = {
		.svc_id = 0xffff,
		.vni = 1,
	};
	int ret;
	struct fi_info *info;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_ep *ep;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	ret = fi_fabric(info->fabric_attr, &fab, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_fabric failed: %d", ret);

	ret = fi_domain(fab, info, &dom, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_domain failed: %d", ret);

	/* Override auth_key with bad auth_key. */
	if (info->domain_attr->auth_key)
		free(info->domain_attr->auth_key);
	info->domain_attr->auth_key = memdup(&auth_key, sizeof(auth_key));
	info->domain_attr->auth_key_size = sizeof(auth_key);

	ret = fi_endpoint(dom, info, &ep, NULL);
	cr_assert_eq(ret, -FI_EINVAL, "fi_endpoint failed: %d", ret);

	fi_close(&dom->fid);
	fi_close(&fab->fid);
	fi_freeinfo(info);
}

/* Valid service ID but invalid VNI for the service ID. */
Test(auth_key, valid_user_defined_svc_id_invalid_vni_hints)
{
	int ret;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	uint16_t valid_vni = 0x120;
	struct fi_info *hints;
	struct fi_info *info;
	struct cxi_auth_key auth_key = {
		.vni = 0x123,
	};

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

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints->fabric_attr->prov_name, "strdup failed");

	auth_key.svc_id = svc_desc.svc_id;
	hints->ep_attr->auth_key_size = sizeof(auth_key);
	hints->ep_attr->auth_key = memdup(&auth_key, sizeof(auth_key));
	cr_assert_not_null(hints->ep_attr->auth_key, "memdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, -FI_ENODATA, "fi_getinfo failed: %d", ret);

	fi_freeinfo(hints);
	ret = cxil_destroy_svc(dev, svc_desc.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);
	cxil_close_device(dev);
}

/* Valid service ID but invalid VNI for the service ID. */
Test(auth_key, valid_user_defined_svc_id_invalid_vni_dom_attr)
{
	int ret;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	uint16_t valid_vni = 0x120;
	struct fi_info *info;
	struct cxi_auth_key auth_key = {
		.vni = 0x123,
	};
	struct fid_fabric *fab;
	struct fid_domain *dom;

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

	ret = fi_fabric(info->fabric_attr, &fab, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_fabric failed: %d", ret);

	/* Override auth_key with bad auth_key. */
	auth_key.svc_id = svc_desc.svc_id;

	if (info->domain_attr->auth_key)
		free(info->domain_attr->auth_key);
	info->domain_attr->auth_key = memdup(&auth_key, sizeof(auth_key));
	info->domain_attr->auth_key_size = sizeof(auth_key);

	ret = fi_domain(fab, info, &dom, NULL);
	cr_assert_eq(ret, -FI_EINVAL, "fi_domain failed: %d", ret);

	fi_close(&fab->fid);
	fi_freeinfo(info);
	ret = cxil_destroy_svc(dev, svc_desc.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);
	cxil_close_device(dev);
}

/* Valid service ID but invalid VNI for the service ID. */
Test(auth_key, valid_user_defined_svc_id_invalid_vni_ep_attr)
{
	int ret;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	uint16_t valid_vni = 0x120;
	struct fi_info *info;
	struct cxi_auth_key auth_key = {
		.vni = 0x123,
	};
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_ep *ep;

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

	ret = fi_fabric(info->fabric_attr, &fab, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_fabric failed: %d", ret);

	ret = fi_domain(fab, info, &dom, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_domain failed: %d", ret);

	/* Override auth_key with bad auth_key. */
	auth_key.svc_id = svc_desc.svc_id;

	if (info->domain_attr->auth_key)
		free(info->domain_attr->auth_key);
	info->domain_attr->auth_key = memdup(&auth_key, sizeof(auth_key));
	info->domain_attr->auth_key_size = sizeof(auth_key);

	ret = fi_endpoint(dom, info, &ep, NULL);
	cr_assert_eq(ret, -FI_EINVAL, "fi_endpoint failed: %d", ret);

	fi_close(&dom->fid);
	fi_close(&fab->fid);
	fi_freeinfo(info);
	ret = cxil_destroy_svc(dev, svc_desc.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);
	cxil_close_device(dev);
}

static void alloc_endpoint(struct fi_info *info, struct fid_fabric **fab,
			   struct fid_domain **dom, struct fid_av **av,
			   struct fid_cq **cq, struct fid_ep **ep)
{
	int ret;
	struct fi_cq_attr cq_attr = {
		.format = FI_CQ_FORMAT_TAGGED,
	};
	struct fi_av_attr av_attr = {};

	ret = fi_fabric(info->fabric_attr, fab, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_fabric failed: %d", ret);

	ret = fi_domain(*fab, info, dom, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_domain failed: %d", ret);

	ret = fi_cq_open(*dom, &cq_attr, cq, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_cq_open failed: %d", ret);

	ret = fi_av_open(*dom, &av_attr, av, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_av_open failed: %d", ret);

	ret = fi_endpoint(*dom, info, ep, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_endpoint failed: %d", ret);

	ret = fi_ep_bind(*ep, &(*av)->fid, 0);
	cr_assert_eq(ret, FI_SUCCESS, "fi_ep_bind failed: %d", ret);

	ret = fi_ep_bind(*ep, &(*cq)->fid, FI_TRANSMIT | FI_RECV);
	cr_assert_eq(ret, FI_SUCCESS, "fi_ep_bind failed: %d", ret);

	ret = fi_enable(*ep);
	cr_assert_eq(ret, FI_SUCCESS, "fi_enable failed: %d", ret);
}

Test(auth_key, valid_user_defined_svc_id_valid_vni_verify_vni_enforcement)
{
	int ret;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	struct fi_info *hints;
	struct fi_info *default_info;
	struct fi_info *user_info;
	struct cxi_auth_key auth_key = {};
	uint16_t valid_vni = 0x1234;
	struct fid_fabric *default_fab;
	struct fid_domain *default_dom;
	struct fid_av *default_av;
	struct fid_cq *default_cq;
	struct fid_ep *default_ep;
	struct fid_fabric *user_fab;
	struct fid_domain *user_dom;
	struct fid_av *user_av;
	struct fid_cq *user_cq;
	struct fid_ep *user_ep;
	char buf[256];
	fi_addr_t target_default_ep;
	struct fi_cq_tagged_entry event;
	struct fi_cq_err_entry error;

	/* Allocate infos for RDMA test. Default_info users the provider
	 * assigned default auth_key where user_info uses the user defined
	 * auth_key.
	 */
	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "0", FI_SOURCE, NULL, &default_info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

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

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints->fabric_attr->prov_name, "strdup failed");

	auth_key.svc_id = svc_desc.svc_id;
	auth_key.vni = valid_vni;
	hints->domain_attr->auth_key_size = sizeof(auth_key);
	hints->domain_attr->auth_key = memdup(&auth_key, sizeof(auth_key));
	cr_assert_not_null(hints->domain_attr->auth_key, "memdup failed");

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &user_info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	/* Allocate endpoints using different service IDs and VNIs. */
	alloc_endpoint(default_info, &default_fab, &default_dom, &default_av,
		       &default_cq, &default_ep);
	alloc_endpoint(user_info, &user_fab, &user_dom, &user_av,
		       &user_cq, &user_ep);

	/* Insert the default EP address into the user AVs. */
	ret = fi_av_insert(user_av, default_info->src_addr, 1,
			   &target_default_ep, 0, NULL);
	cr_assert_eq(ret, 1, "fi_av_insert failed: %d", ret);

	/* These two endpoints should not be able to talk due to operating in
	 * different VNIs. This should result in an I/O error at the initiator.
	 */
	ret = fi_recv(default_ep, buf, sizeof(buf), NULL, FI_ADDR_UNSPEC, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_recv failed: %d", ret);

	ret = fi_send(user_ep, buf, sizeof(buf), NULL, target_default_ep, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_send failed: %d", ret);

	do {
		ret = fi_cq_read(user_cq, &event, 1);
	} while (ret == -FI_EAGAIN);

	cr_assert_eq(ret, -FI_EAVAIL, "fi_cq_read failed: %d", ret);

	ret = fi_cq_readerr(user_cq, &error, 0);
	cr_assert_eq(ret, 1, "fi_cq_readerr failed: %d", ret);

	/* Single these tests are loopback on the same NIC, RC_PTLTE_NOT_FOUND
	 * is returned instead of RC_VNI_NOT_FOUND since the VNI is valid.
	 * Non-loopback should returned RC_VNI_NOT_FOUND.
	 */
	cr_assert_eq(error.prov_errno, C_RC_PTLTE_NOT_FOUND,
		     "Bad error.prov_errno: got=%d expected=%d",
		     error.prov_errno, C_RC_PTLTE_NOT_FOUND);

	fi_close(&user_ep->fid);
	fi_close(&user_cq->fid);
	fi_close(&user_av->fid);
	fi_close(&user_dom->fid);
	fi_close(&user_fab->fid);
	fi_close(&default_ep->fid);
	fi_close(&default_cq->fid);
	fi_close(&default_av->fid);
	fi_close(&default_dom->fid);
	fi_close(&default_fab->fid);
	fi_freeinfo(user_info);
	fi_freeinfo(hints);
	fi_freeinfo(default_info);
	ret = cxil_destroy_svc(dev, svc_desc.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);
	cxil_close_device(dev);
}

/* Use the Slingshot plugin environment variables to generate an auth_key. Only
 * a single entry per environment variable is specified.
 */
Test(auth_key, ss_plugin_env_vars_single_entry)
{
	int ret;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	struct fi_info *info;
	struct cxi_auth_key auth_key = {
		.vni = 288,
	};
	char svc_id_str[256];
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct cxip_nic_attr *nic_attr;

	/* Need to allocate a service to be used by libfabric. */
	ret = cxil_open_device(0, &dev);
	cr_assert_eq(ret, 0, "cxil_open_device failed: %d", ret);

	svc_desc.restricted_vnis = 1;
	svc_desc.enable = 1;
	svc_desc.num_vld_vnis = 1;
	svc_desc.vnis[0] = auth_key.vni;

	ret = cxil_alloc_svc(dev, &svc_desc, &fail_info);
	cr_assert_gt(ret, 0, "cxil_alloc_svc failed: %d", ret);
	svc_desc.svc_id = ret;
	auth_key.svc_id = ret;

	ret = setenv("SLINGSHOT_VNIS", "288", 1);
	cr_assert_eq(ret, 0, "setenv failed: %d", errno);

	ret = setenv("SLINGSHOT_DEVICES", "cxi0", 1);
	cr_assert_eq(ret, 0, "setenv failed: %d", errno);

	sprintf(svc_id_str, "%d", auth_key.svc_id);
	ret = setenv("SLINGSHOT_SVC_IDS", svc_id_str, 1);
	cr_assert_eq(ret, 0, "setenv failed: %d", errno);

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	cr_assert_eq(info->domain_attr->auth_key, NULL);
	cr_assert_eq(info->domain_attr->auth_key_size, 0);

	nic_attr = info->nic->prov_attr;
	cr_assert_eq(nic_attr->default_rgroup_id, auth_key.svc_id,
		     "fi_getinfo returned auth_key does not match Slingshot env vars");
	cr_assert_eq(nic_attr->default_vni, auth_key.vni,
		     "fi_getinfo returned auth_key does not match Slingshot env vars");

	ret = fi_fabric(info->fabric_attr, &fab, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_fabric failed: %d", ret);

	ret = fi_domain(fab, info, &dom, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_domain failed: %d", ret);

	fi_close(&dom->fid);
	fi_close(&fab->fid);
	fi_freeinfo(info);
	ret = cxil_destroy_svc(dev, svc_desc.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);
	cxil_close_device(dev);
}

/* Use the Slingshot plugin environment variables to generate an auth_key.
 * Multiple values per environment variable are specified.
 */
Test(auth_key, ss_plugin_env_vars_multiple_entries)
{
	int ret;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	struct fi_info *info;
	struct cxi_auth_key auth_key = {
		.vni = 288,
	};
	char svc_id_str[256];
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct cxip_nic_attr *nic_attr;

	/* Need to allocate a service to be used by libfabric. */
	ret = cxil_open_device(0, &dev);
	cr_assert_eq(ret, 0, "cxil_open_device failed: %d", ret);

	svc_desc.restricted_vnis = 1;
	svc_desc.enable = 1;
	svc_desc.num_vld_vnis = 1;
	svc_desc.vnis[0] = auth_key.vni;

	ret = cxil_alloc_svc(dev, &svc_desc, &fail_info);
	cr_assert_gt(ret, 0, "cxil_alloc_svc failed: %d", ret);
	svc_desc.svc_id = ret;
	auth_key.svc_id = ret;

	ret = setenv("SLINGSHOT_VNIS", "288,999", 1);
	cr_assert_eq(ret, 0, "setenv failed: %d", errno);

	ret = setenv("SLINGSHOT_DEVICES", "cxi1,cxi15,cxi4,cxi0", 1);
	cr_assert_eq(ret, 0, "setenv failed: %d", errno);

	sprintf(svc_id_str, "1024,1025,1026,%d", auth_key.svc_id);
	ret = setenv("SLINGSHOT_SVC_IDS", svc_id_str, 1);
	cr_assert_eq(ret, 0, "setenv failed: %d", errno);

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	cr_assert_eq(info->domain_attr->auth_key, NULL);
	cr_assert_eq(info->domain_attr->auth_key_size, 0);

	nic_attr = info->nic->prov_attr;
	cr_assert_eq(nic_attr->default_rgroup_id, auth_key.svc_id,
		     "fi_getinfo returned auth_key does not match Slingshot env vars");
	cr_assert_eq(nic_attr->default_vni, auth_key.vni,
		     "fi_getinfo returned auth_key does not match Slingshot env vars");

	ret = fi_fabric(info->fabric_attr, &fab, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_fabric failed: %d", ret);

	ret = fi_domain(fab, info, &dom, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_domain failed: %d", ret);

	fi_close(&dom->fid);
	fi_close(&fab->fid);
	fi_freeinfo(info);
	ret = cxil_destroy_svc(dev, svc_desc.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);
	cxil_close_device(dev);
}

#define DEFAULT_SERVICE_ID 1U

/* Use the Slingshot plugin environment variables to define auth_keys for a
 * cxi device which does not exist.
 */
Test(auth_key, ss_plugin_env_vars_no_nic)
{
	struct fi_info *info;
	int ret;
	struct cxip_nic_attr *nic_attr;

	ret = setenv("SLINGSHOT_VNIS", "288,999", 1);
	cr_assert_eq(ret, 0, "setenv failed: %d", errno);

	ret = setenv("SLINGSHOT_DEVICES", "cxi1,cxi15,cxi4", 1);
	cr_assert_eq(ret, 0, "setenv failed: %d", errno);

	ret = setenv("SLINGSHOT_SVC_IDS", "1024,1025,1026", 1);
	cr_assert_eq(ret, 0, "setenv failed: %d", errno);

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	cr_assert_eq(info->domain_attr->auth_key, NULL);
	cr_assert_eq(info->domain_attr->auth_key_size, 0);

	nic_attr = info->nic->prov_attr;
	cr_assert_eq(nic_attr->default_rgroup_id, DEFAULT_SERVICE_ID,
		     "Unexpected svc_id: %d", nic_attr->default_rgroup_id);

	fi_freeinfo(info);
}

/* Define valid Slingshot plugin environment variables and verify that user
 * provided auth_key is honored before using Slingshot plugin environment
 * variables to generate auth_key.
 */
Test(auth_key, ss_plugin_auth_key_priority)
{
	int ret;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	struct fi_info *info;
	struct fi_info *hints;
	char svc_id_str[256];
	struct cxi_auth_key auth_key = {
		.svc_id = CXI_DEFAULT_SVC_ID,
		.vni = 1,
	};
	struct fid_fabric *fab;
	struct fid_domain *dom;

	/* Need to allocate a service to be used by libfabric. */
	ret = cxil_open_device(0, &dev);
	cr_assert_eq(ret, 0, "cxil_open_device failed: %d", ret);

	svc_desc.restricted_vnis = 1;
	svc_desc.enable = 1;
	svc_desc.num_vld_vnis = 1;
	svc_desc.vnis[0] = auth_key.vni;

	ret = cxil_alloc_svc(dev, &svc_desc, &fail_info);
	cr_assert_gt(ret, 0, "cxil_alloc_svc failed: %d", ret);
	svc_desc.svc_id = ret;

	ret = setenv("SLINGSHOT_VNIS", "1", 1);
	cr_assert_eq(ret, 0, "setenv failed: %d", errno);

	ret = setenv("SLINGSHOT_DEVICES", "cxi0", 1);
	cr_assert_eq(ret, 0, "setenv failed: %d", errno);

	sprintf(svc_id_str, "%d", auth_key.svc_id);
	ret = setenv("SLINGSHOT_SVC_IDS", svc_id_str, 1);
	cr_assert_eq(ret, 0, "setenv failed: %d", errno);

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints->fabric_attr->prov_name, "strdup failed");

	hints->domain_attr->auth_key = memdup(&auth_key, sizeof(auth_key));
	cr_assert_not_null(hints->domain_attr->auth_key, "memdup failed");

	hints->domain_attr->auth_key_size = sizeof(auth_key);
	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	ret = memcmp(hints->domain_attr->auth_key, info->domain_attr->auth_key,
		     hints->domain_attr->auth_key_size);
	cr_assert_eq(ret, 0, "fi_getinfo returned auth_key does not match hints");
	cr_assert_eq(info->domain_attr->auth_key_size, sizeof(auth_key));

	ret = fi_fabric(info->fabric_attr, &fab, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_fabric failed: %d", ret);

	ret = fi_domain(fab, info, &dom, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_domain failed: %d", ret);

	fi_close(&dom->fid);
	fi_close(&fab->fid);
	fi_freeinfo(info);
	fi_freeinfo(hints);
	ret = cxil_destroy_svc(dev, svc_desc.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);
	cxil_close_device(dev);
}

/* Restrict the auth_key to a specific UID. */
Test(auth_key, uid_valid_service)
{
	int ret;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	struct fi_info *info;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	uid_t test_uid = 65530;
	uint64_t test_vni = 12345;
	struct cxi_auth_key auth_key = {};
	struct cxip_nic_attr *nic_attr;

	/* Need to allocate a service to be used by libfabric. */
	ret = cxil_open_device(0, &dev);
	cr_assert_eq(ret, 0, "cxil_open_device failed: %d", ret);

	svc_desc.restricted_members = 1;
	svc_desc.restricted_vnis = 1;
	svc_desc.enable = 1;
	svc_desc.num_vld_vnis = 1;
	svc_desc.vnis[0] = test_vni;
	svc_desc.members[0].type = CXI_SVC_MEMBER_UID;
	svc_desc.members[0].svc_member.uid = test_uid;

	ret = cxil_alloc_svc(dev, &svc_desc, &fail_info);
	cr_assert_gt(ret, 0, "cxil_alloc_svc failed: %d", ret);
	svc_desc.svc_id = ret;

	auth_key.svc_id = svc_desc.svc_id;
	auth_key.vni = test_vni;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	/* Ensure that returned auth_key does not contain allocated service ID
	 * since this is restricted to specific UID.
	 *
	 * Return auth_key hint should be NULL. NIC attr should not contain the
	 * service ID and VNI.
	 */
	cr_assert_eq(info->domain_attr->auth_key, NULL);
	cr_assert_eq(info->domain_attr->auth_key_size, 0);

	nic_attr = info->nic->prov_attr;
	cr_assert_neq(nic_attr->default_rgroup_id, auth_key.svc_id);
	cr_assert_neq(nic_attr->default_vni, auth_key.vni);

	fi_freeinfo(info);

	ret = seteuid(test_uid);
	cr_assert_eq(ret, 0, "seteuid failed: %d", errno);

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	/* Ensure that returned auth_key does not contain allocated service ID
	 * since this is restricted to specific UID.
	 *
	 * Return auth_key hint should be NULL. NIC attr should not contain the
	 * service ID and VNI.
	 */
	cr_assert_eq(info->domain_attr->auth_key, NULL);
	cr_assert_eq(info->domain_attr->auth_key_size, 0);

	nic_attr = info->nic->prov_attr;
	cr_assert_eq(nic_attr->default_rgroup_id, auth_key.svc_id);
	cr_assert_eq(nic_attr->default_vni, auth_key.vni);

	ret = fi_fabric(info->fabric_attr, &fab, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_fabric failed: %d", ret);

	ret = fi_domain(fab, info, &dom, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_domain failed: %d", ret);

	fi_close(&dom->fid);
	fi_close(&fab->fid);
	fi_freeinfo(info);

	/* Make sure non-root user cannot destroy service. */
	ret = cxil_destroy_svc(dev, svc_desc.svc_id);
	cr_assert_neq(ret, 0, "cxil_destroy_svc did not fail");

	ret = seteuid(0);
	cr_assert_eq(ret, 0, "seteuid failed: %d", errno);

	ret = cxil_destroy_svc(dev, svc_desc.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);
	cxil_close_device(dev);
}

/* Restrict the auth_key to a specific GID. */
Test(auth_key, gid_valid_service)
{
	int ret;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	struct fi_info *info;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	uid_t test_gid = 32766;
	uint64_t test_vni = 12345;
	struct cxi_auth_key auth_key = {};
	struct cxip_nic_attr *nic_attr;

	/* Need to allocate a service to be used by libfabric. */
	ret = cxil_open_device(0, &dev);
	cr_assert_eq(ret, 0, "cxil_open_device failed: %d", ret);

	svc_desc.restricted_members = 1;
	svc_desc.restricted_vnis = 1;
	svc_desc.enable = 1;
	svc_desc.num_vld_vnis = 1;
	svc_desc.vnis[0] = test_vni;
	svc_desc.members[0].type = CXI_SVC_MEMBER_GID;
	svc_desc.members[0].svc_member.gid = test_gid;

	ret = cxil_alloc_svc(dev, &svc_desc, &fail_info);
	cr_assert_gt(ret, 0, "cxil_alloc_svc failed: %d", ret);
	svc_desc.svc_id = ret;

	auth_key.svc_id = svc_desc.svc_id;
	auth_key.vni = test_vni;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	/* Ensure that returned auth_key does not contain allocated service ID
	 * since this is restricted to specific UID.
	 *
	 * Return auth_key hint should be NULL. NIC attr should not contain the
	 * service ID and VNI.
	 */
	cr_assert_eq(info->domain_attr->auth_key, NULL);
	cr_assert_eq(info->domain_attr->auth_key_size, 0);

	nic_attr = info->nic->prov_attr;
	cr_assert_neq(nic_attr->default_rgroup_id, auth_key.svc_id);
	cr_assert_neq(nic_attr->default_vni, auth_key.vni);

	fi_freeinfo(info);

	ret = setegid(test_gid);
	cr_assert_eq(ret, 0, "setegid failed: %d", errno);

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	/* Ensure that returned auth_key does contain allocated service ID
	 * since this is restricted to specific UID.
	 *
	 * Return auth_key hint should be NULL. NIC attr should contain the
	 * service ID and VNI.
	 */
	cr_assert_eq(info->domain_attr->auth_key, NULL);
	cr_assert_eq(info->domain_attr->auth_key_size, 0);

	nic_attr = info->nic->prov_attr;
	cr_assert_eq(nic_attr->default_rgroup_id, auth_key.svc_id);
	cr_assert_eq(nic_attr->default_vni, auth_key.vni);

	ret = fi_fabric(info->fabric_attr, &fab, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_fabric failed: %d", ret);

	ret = fi_domain(fab, info, &dom, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_domain failed: %d", ret);

	fi_close(&dom->fid);
	fi_close(&fab->fid);
	fi_freeinfo(info);

	ret = setegid(0);
	cr_assert_eq(ret, 0, "setegid failed: %d", errno);

	ret = cxil_destroy_svc(dev, svc_desc.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);
	cxil_close_device(dev);
}

/* Verify the priority between UID, GID, and unrestricted services get honored.
 */
Test(auth_key, uid_gid_default_service_id_priority)
{
	int ret;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	struct fi_info *info;
	uid_t test_uid = 65530;
	uint64_t test_uid_vni = 12345;
	uid_t test_gid = 32766;
	uint64_t test_gid_vni = 12344;
	struct cxi_auth_key uid_auth_key = {};
	struct cxi_auth_key gid_auth_key = {};
	struct cxip_nic_attr *nic_attr;

	/* Need to allocate a service to be used by libfabric. */
	ret = cxil_open_device(0, &dev);
	cr_assert_eq(ret, 0, "cxil_open_device failed: %d", ret);

	svc_desc.restricted_members = 1;
	svc_desc.restricted_vnis = 1;
	svc_desc.enable = 1;
	svc_desc.num_vld_vnis = 1;
	svc_desc.vnis[0] = test_uid_vni;
	svc_desc.members[0].type = CXI_SVC_MEMBER_UID;
	svc_desc.members[0].svc_member.uid = test_uid;

	ret = cxil_alloc_svc(dev, &svc_desc, &fail_info);
	cr_assert_gt(ret, 0, "cxil_alloc_svc failed: %d", ret);

	uid_auth_key.svc_id = ret;
	uid_auth_key.vni = test_uid_vni;

	svc_desc.vnis[0] = test_gid_vni;
	svc_desc.members[0].type = CXI_SVC_MEMBER_GID;
	svc_desc.members[0].svc_member.gid = test_gid;

	ret = cxil_alloc_svc(dev, &svc_desc, &fail_info);
	cr_assert_gt(ret, 0, "cxil_alloc_svc failed: %d", ret);

	gid_auth_key.svc_id = ret;
	gid_auth_key.vni = test_gid_vni;

	/* Since UID and GID have not changed, auth_key with default service ID
	 * should be returned.
	 */
	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	cr_assert_eq(info->domain_attr->auth_key, NULL);
	cr_assert_eq(info->domain_attr->auth_key_size, 0);

	nic_attr = info->nic->prov_attr;
	cr_assert_eq(nic_attr->default_rgroup_id, DEFAULT_SERVICE_ID,
		     "Default service ID was not returned: svc_id=%d",
		     nic_attr->default_rgroup_id);

	fi_freeinfo(info);

	/* Changing GID should result in GID auth_key being returned. */
	ret = setegid(test_gid);
	cr_assert_eq(ret, 0, "setegid failed: %d", errno);

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	cr_assert_eq(info->domain_attr->auth_key, NULL);
	cr_assert_eq(info->domain_attr->auth_key_size, 0);

	nic_attr = info->nic->prov_attr;
	cr_assert_eq(nic_attr->default_rgroup_id, gid_auth_key.svc_id);
	cr_assert_eq(nic_attr->default_vni, gid_auth_key.vni);

	fi_freeinfo(info);

	/* Changing the UID should result in UID auth_key being returned. */
	ret = seteuid(test_uid);
	cr_assert_eq(ret, 0, "seteuid failed: %d", errno);

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	cr_assert_eq(info->domain_attr->auth_key, NULL);
	cr_assert_eq(info->domain_attr->auth_key_size, 0);

	nic_attr = info->nic->prov_attr;
	cr_assert_eq(nic_attr->default_rgroup_id, uid_auth_key.svc_id);
	cr_assert_eq(nic_attr->default_vni, uid_auth_key.vni);

	fi_freeinfo(info);

	ret = seteuid(0);
	cr_assert_eq(ret, 0, "seteuid failed: %d", errno);

	ret = setegid(0);
	cr_assert_eq(ret, 0, "setegid failed: %d", errno);

	ret = cxil_destroy_svc(dev, gid_auth_key.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);

	ret = cxil_destroy_svc(dev, uid_auth_key.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);

	cxil_close_device(dev);
}

/* Test disabling the default service ID. */
Test(auth_key, default_service_id_disabled)
{
	int ret;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	struct fi_info *info;
	struct fid_fabric *fab;
	struct fid_domain *dom;

	/* Disable the default service ID. */
	ret = cxil_open_device(0, &dev);
	cr_assert_eq(ret, 0, "cxil_open_device failed: %d", ret);

	ret = cxil_get_svc(dev, DEFAULT_SERVICE_ID, &svc_desc);
	cr_assert_eq(ret, 0, "cxil_get_svc failed: %d", ret);
	cr_assert_eq(svc_desc.enable, 1,
		     "Default service ID unexpectedly disabled");

	svc_desc.enable = 0;

	ret = cxil_update_svc(dev, &svc_desc, &fail_info);
	cr_assert_eq(ret, 0, "cxil_open_device failed: %d", ret);

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	/* With the default service ID disabled, NULL auth_key should be
	 * returned.
	 */
	cr_assert_null(info->domain_attr->auth_key, "Domain auth_key not NULL");
	cr_assert_null(info->ep_attr->auth_key, "EP auth_key not NULL");

	ret = fi_fabric(info->fabric_attr, &fab, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_fabric failed: %d", ret);

	ret = fi_domain(fab, info, &dom, NULL);
	cr_assert_neq(ret, FI_SUCCESS, "fi_domain did not fail");

	fi_close(&fab->fid);
	fi_freeinfo(info);

	/* Restore default service. */
	svc_desc.enable = 1;
	ret = cxil_update_svc(dev, &svc_desc, &fail_info);
	cr_assert_eq(ret, 0, "cxil_open_device failed: %d", ret);

	cxil_close_device(dev);
}

#define DEFAULT_MAX_EP_AUTH_KEY 4

Test(auth_key, max_ep_auth_key_null_hints)
{
	int ret;
	struct fi_info *info;
	struct fi_info *tmp;
	int i = 0;
	size_t expected_ep_auth_key;

	ret = setenv("FI_CXI_COMPAT", "0", 1);
	cr_assert(ret == 0);

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, NULL, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	tmp = info;
	while (tmp) {
		/* The first 2 fi_info's should have max_ep_auth_key == 1*/
		if (i < 2)
			expected_ep_auth_key = 1;
		else
			expected_ep_auth_key = DEFAULT_MAX_EP_AUTH_KEY;

		cr_assert_eq(tmp->domain_attr->max_ep_auth_key,
			     expected_ep_auth_key,
			     "Invalid max_ep_auth_key: expected=%ld got=%ld info_count=%d",
			     expected_ep_auth_key,
			     tmp->domain_attr->max_ep_auth_key, i);
		tmp = tmp->next;
		i++;
	}

	fi_freeinfo(info);
}

/* Test fi_getinfo() verification of hints argument. */
Test(auth_key, zero_max_ep_auth_key_null_hint)
{
	int ret;
	struct fi_info *hints;
	struct fi_info *info;
	struct fi_info *tmp;
	int i = 0;
	size_t expected_ep_auth_key;

	ret = setenv("FI_CXI_COMPAT", "0", 1);
	cr_assert(ret == 0);

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->max_ep_auth_key = 0;
	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	tmp = info;
	while (tmp) {
		/* The first 2 fi_info's should have max_ep_auth_key == 1*/
		if (i < 2)
			expected_ep_auth_key = 1;
		else
			expected_ep_auth_key = DEFAULT_MAX_EP_AUTH_KEY;

		cr_assert_eq(tmp->domain_attr->max_ep_auth_key,
			     expected_ep_auth_key,
			     "Invalid max_ep_auth_key: expected=%ld got=%ld info_count=%d",
			     expected_ep_auth_key,
			     tmp->domain_attr->max_ep_auth_key, i);
		tmp = tmp->next;
		i++;
	}

	fi_freeinfo(hints);
	fi_freeinfo(info);
}

/* Test fi_getinfo() verification of hints argument. */
Test(auth_key, valid_max_ep_auth_key_null_hint)
{
	int ret;
	struct fi_info *hints;
	struct fi_info *info;
	struct fi_info *tmp;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->max_ep_auth_key = 1;
	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	tmp = info;
	while (tmp) {
		cr_assert_eq(tmp->domain_attr->max_ep_auth_key,
			     hints->domain_attr->max_ep_auth_key,
			     "Invalid max_ep_auth_key: expected=%ld got=%ld",
			     hints->domain_attr->max_ep_auth_key,
			     tmp->domain_attr->max_ep_auth_key);
		tmp = tmp->next;
	}

	fi_freeinfo(hints);
	fi_freeinfo(info);
}

/* Test fi_getinfo() verification of hints argument. */
Test(auth_key, invalid_max_ep_auth_key_null_hint)
{
	int ret;
	struct fi_info *hints;
	struct fi_info *info;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->max_ep_auth_key = 12345678;
	hints->domain_attr->mr_mode = FI_MR_ENDPOINT;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 NULL, FI_SOURCE, hints, &info);
	cr_assert_eq(ret, -FI_ENODATA, "fi_getinfo failed: %d", ret);

	fi_freeinfo(hints);
}

TestSuite(av_auth_key, .timeout = CXIT_DEFAULT_TIMEOUT);

static void open_av_auth_key(struct fi_info *info, struct fid_fabric **fab,
			     struct fid_domain **dom, struct fid_av **av)
{
	int ret;
	struct fi_av_attr av_attr = {};

	ret = fi_fabric(info->fabric_attr, fab, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_fabric failed: %d", ret);

	ret = fi_domain(*fab, info, dom, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_domain failed: %d", ret);

	ret = fi_av_open(*dom, &av_attr, av, NULL);
	cr_assert_eq(ret, FI_SUCCESS, "fi_av_open failed: %d", ret);
}

static void close_av_auth_key(struct fid_fabric *fab, struct fid_domain *dom,
			      struct fid_av *av)
{
	int ret;

	ret = fi_close(&av->fid);
	cr_assert_eq(ret, FI_SUCCESS);

	ret = fi_close(&dom->fid);
	cr_assert_eq(ret, FI_SUCCESS);

	ret = fi_close(&fab->fid);
	cr_assert_eq(ret, FI_SUCCESS);
}

Test(av_auth_key, insert_without_av_auth_key_set)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	struct cxip_nic_attr *nic_attr;
	fi_addr_t addr_key;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	nic_attr = info->nic->prov_attr;
	auth_key.vni = nic_attr->default_vni;

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_insert_auth_key(av, &auth_key, sizeof(auth_key), &addr_key,
				    0);
	cr_assert_eq(ret, -FI_EINVAL, "fi_av_insert_auth_key failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, lookup_without_av_auth_key_set)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	size_t size = sizeof(auth_key);
	fi_addr_t addr_key = 0;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_lookup_auth_key(av, addr_key, &auth_key, &size);
	cr_assert_eq(ret, -FI_EINVAL, "fi_av_lookup_auth_key failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

/* Insert multiple auth_keys. */
#define NUM_VNIS 4U
Test(av_auth_key, insert_lookup_valid_auth_key)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	struct cxi_auth_key lookup_auth_key = {};
	size_t auth_key_size;
	fi_addr_t addr_key;
	struct cxil_dev *dev;
	struct cxi_svc_fail_info fail_info = {};
	struct cxi_svc_desc svc_desc = {};
	int i;

	ret = cxil_open_device(0, &dev);
	cr_assert_eq(ret, 0, "cxil_open_device failed: %d", ret);

	svc_desc.restricted_vnis = 1;
	svc_desc.enable = 1;
	svc_desc.num_vld_vnis = NUM_VNIS;

	for (i = 0; i < NUM_VNIS; i++)
		svc_desc.vnis[i] = 123 + i;

	ret = cxil_alloc_svc(dev, &svc_desc, &fail_info);
	cr_assert_gt(ret, 0, "cxil_alloc_svc failed: %d", ret);
	svc_desc.svc_id = ret;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;
	hints->domain_attr->max_ep_auth_key = NUM_VNIS;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	open_av_auth_key(info, &fab, &dom, &av);

	for (i = 0; i < NUM_VNIS; i++) {
		auth_key.vni = svc_desc.vnis[i];

		ret = fi_av_insert_auth_key(av, &auth_key, sizeof(auth_key),
					    &addr_key, 0);
		cr_assert_eq(ret, FI_SUCCESS,
			     "fi_av_insert_auth_key failed: %d", ret);

		auth_key_size = sizeof(lookup_auth_key);
		ret = fi_av_lookup_auth_key(av, addr_key, &lookup_auth_key,
					    &auth_key_size);
		cr_assert_eq(ret, FI_SUCCESS,
			     "fi_av_lookup_auth_key failed: %d", ret);

		cr_assert_eq(auth_key_size, sizeof(lookup_auth_key),
			     "Invalid auth_key_size returned");
		cr_assert_eq(lookup_auth_key.vni, auth_key.vni,
			     "Incorrect auth_key.vni returned");
	}

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);

	ret = cxil_destroy_svc(dev, svc_desc.svc_id);
	cr_assert_eq(ret, 0, "cxil_destroy_svc failed: %d", ret);
	cxil_close_device(dev);
}

Test(av_auth_key, insert_invalid_null_auth_key)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	struct cxip_nic_attr *nic_attr;
	fi_addr_t addr_key;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	nic_attr = info->nic->prov_attr;
	auth_key.vni = nic_attr->default_vni;

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_insert_auth_key(av, NULL, sizeof(auth_key), &addr_key,
				    0);
	cr_assert_eq(ret, -FI_EINVAL, "fi_av_insert_auth_key failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, insert_invalid_null_fi_addr)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	struct cxip_nic_attr *nic_attr;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	nic_attr = info->nic->prov_attr;
	auth_key.vni = nic_attr->default_vni;

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_insert_auth_key(av, &auth_key, sizeof(auth_key), NULL, 0);
	cr_assert_eq(ret, -FI_EINVAL, "fi_av_insert_auth_key failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, insert_invalid_flags)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	struct cxip_nic_attr *nic_attr;
	fi_addr_t addr_key;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	nic_attr = info->nic->prov_attr;
	auth_key.vni = nic_attr->default_vni;

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_insert_auth_key(av, &auth_key, sizeof(auth_key), &addr_key,
				    0x123);
	cr_assert_eq(ret, -FI_EINVAL, "fi_av_insert_auth_key failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, insert_invalid_vni)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	fi_addr_t addr_key;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	auth_key.vni = 0x1234;

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_insert_auth_key(av, &auth_key, sizeof(auth_key), &addr_key,
				    0);
	cr_assert_eq(ret, -FI_EINVAL, "fi_av_insert_auth_key failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, insert_max_ep_auth_key_bounds_check)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	struct cxip_nic_attr *nic_attr;
	fi_addr_t addr_key;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;
	hints->domain_attr->max_ep_auth_key = 1;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	nic_attr = info->nic->prov_attr;
	auth_key.vni = nic_attr->default_vni;

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_insert_auth_key(av, &auth_key, sizeof(auth_key), &addr_key,
				    0);
	cr_assert_eq(ret, FI_SUCCESS, "fi_av_insert_auth_key failed: %d", ret);

	ret = fi_av_insert_auth_key(av, &auth_key, sizeof(auth_key), &addr_key,
				    0);
	cr_assert_eq(ret, -FI_ENOSPC, "fi_av_insert_auth_key failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, lookup_null_auth_key)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	fi_addr_t addr_key = 0;
	size_t auth_key_size = sizeof(auth_key);

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	auth_key.vni = 0x1234;

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_lookup_auth_key(av, addr_key, NULL, &auth_key_size);
	cr_assert_eq(ret, -FI_EINVAL, "fi_av_lookup_auth_key failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, lookup_null_auth_key_size)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	fi_addr_t addr_key = 0;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	auth_key.vni = 0x1234;

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_lookup_auth_key(av, addr_key, &auth_key, NULL);
	cr_assert_eq(ret, -FI_EINVAL, "fi_av_lookup_auth_key failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, remove)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	struct cxip_nic_attr *nic_attr;
	fi_addr_t addr_key;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;
	hints->domain_attr->max_ep_auth_key = 1;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	nic_attr = info->nic->prov_attr;
	auth_key.vni = nic_attr->default_vni;

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_insert_auth_key(av, &auth_key, sizeof(auth_key), &addr_key,
				    0);
	cr_assert_eq(ret, FI_SUCCESS, "fi_av_insert_auth_key failed: %d", ret);

	ret = fi_av_remove(av, &addr_key, 1, FI_AUTH_KEY);
	cr_assert_eq(ret, FI_SUCCESS, "fi_av_remove failed: %d", ret);

	ret = fi_av_insert_auth_key(av, &auth_key, sizeof(auth_key), &addr_key,
				    0);
	cr_assert_eq(ret, FI_SUCCESS, "fi_av_insert_auth_key failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, valid_insert_auth_key_addr)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	struct cxip_addr addr = {};
	struct cxip_nic_attr *nic_attr;
	fi_addr_t addr_key;
	size_t addr_key_size = sizeof(addr);

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;
	hints->domain_attr->max_ep_auth_key = 1;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	nic_attr = info->nic->prov_attr;
	auth_key.vni = nic_attr->default_vni;

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_insert_auth_key(av, &auth_key, sizeof(auth_key), &addr_key,
				    0);
	cr_assert_eq(ret, FI_SUCCESS, "fi_av_insert_auth_key failed: %d", ret);

	ret = fi_av_insert(av, &addr, 1, &addr_key, FI_AUTH_KEY, NULL);
	cr_assert_eq(ret, 1, "fi_av_insert failed: %d", ret);

	ret = fi_av_lookup(av, addr_key, &addr, &addr_key_size);
	cr_assert_eq(ret, FI_SUCCESS, "fi_av_lookup failed: %d", ret);

	cr_assert_eq(addr.vni, auth_key.vni,
		     "Invalid auth_key vni: expected=%u got=%u",
		     auth_key.vni, addr.vni);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, miss_auth_key_insert_flag)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	struct cxip_addr addr = {};
	struct cxip_nic_attr *nic_attr;
	fi_addr_t addr_key;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;
	hints->domain_attr->max_ep_auth_key = 1;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	nic_attr = info->nic->prov_attr;
	auth_key.vni = nic_attr->default_vni;

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_insert_auth_key(av, &auth_key, sizeof(auth_key), &addr_key,
				    0);
	cr_assert_eq(ret, FI_SUCCESS, "fi_av_insert_auth_key failed: %d", ret);

	ret = fi_av_insert(av, &addr, 1, &addr_key, 0, NULL);
	cr_assert_eq(ret, -FI_EINVAL, "fi_av_insert failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, invalid_user_id_flag)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxi_auth_key auth_key = {};
	struct cxip_addr addr = {};
	struct cxip_nic_attr *nic_attr;
	fi_addr_t addr_key;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;
	hints->domain_attr->max_ep_auth_key = 1;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	nic_attr = info->nic->prov_attr;
	auth_key.vni = nic_attr->default_vni;

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_insert_auth_key(av, &auth_key, sizeof(auth_key), &addr_key,
				    0);
	cr_assert_eq(ret, FI_SUCCESS, "fi_av_insert_auth_key failed: %d", ret);

	ret = fi_av_insert(av, &addr, 1, &addr_key, FI_AV_USER_ID, NULL);
	cr_assert_eq(ret, -FI_EINVAL, "fi_av_insert failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, invalid_user_id_auth_key_flags)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxip_addr addr = {};
	fi_addr_t addr_key;

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->max_ep_auth_key = 1;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_insert(av, &addr, 1, &addr_key,
			   (FI_AV_USER_ID | FI_AUTH_KEY), NULL);
	cr_assert_eq(ret, -FI_EINVAL, "fi_av_insert failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}

Test(av_auth_key, null_auth_key_addr)
{
	struct fi_info *hints;
	struct fi_info *info;
	int ret;
	struct fid_fabric *fab;
	struct fid_domain *dom;
	struct fid_av *av;
	struct cxip_addr addr = {};

	hints = fi_allocinfo();
	cr_assert_not_null(hints, "fi_allocinfo failed");

	hints->fabric_attr->prov_name = strdup("cxi");
	cr_assert_not_null(hints, "strdup failed");

	hints->domain_attr->mr_mode = FI_MR_ENDPOINT | FI_MR_ALLOCATED;
	hints->domain_attr->auth_key_size = FI_AV_AUTH_KEY;
	hints->domain_attr->max_ep_auth_key = 1;

	ret = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), "cxi0",
			 "255", FI_SOURCE, hints, &info);
	cr_assert_eq(ret, FI_SUCCESS, "fi_getinfo failed: %d", ret);

	open_av_auth_key(info, &fab, &dom, &av);

	ret = fi_av_insert(av, &addr, 1, NULL, FI_AUTH_KEY, NULL);
	cr_assert_eq(ret, -FI_EINVAL, "fi_av_insert failed: %d", ret);

	close_av_auth_key(fab, dom, av);

	fi_freeinfo(info);
	fi_freeinfo(hints);
}
