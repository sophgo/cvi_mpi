/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_ca_ctrl.c
 * Description:
 *
 */

#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"
#include "isp_ioctl.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"
#include "isp_sts_ctrl.h"

#include "isp_ca_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_ca_ctrl_runtime  *_get_ca_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_ca_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CA, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_ca_ctrl_check_ca_attr_valid(const ISP_CA_ATTR_S *pstCAAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstCAAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstCAAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstCAAttr, UpdateInterval, 0, 0xff);
	// CHECK_VALID_CONST(pstCAAttr, CaCpMode, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_ARRAY(pstCAAttr, CPLutY, CA_LUT_NUM, 0, 0xff);
	// CHECK_VALID_ARRAY(pstCAAttr, CPLutU, CA_LUT_NUM, 0, 0xff);
	// CHECK_VALID_ARRAY(pstCAAttr, CPLutV, CA_LUT_NUM, 0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstCAAttr, ISORatio, 0, 0x7ff);
	CHECK_VALID_AUTO_ISO_2D(pstCAAttr, YRatioLut, CA_LUT_NUM, 0, 0x7ff);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_ca_ctrl_get_ca_attr(VI_PIPE ViPipe, const ISP_CA_ATTR_S **pstCAAttr)
{
	if (pstCAAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_ca_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CA, (CVI_VOID *) &shared_buffer);
	*pstCAAttr = &shared_buffer->stCAAttr;

	return ret;
}

CVI_S32 isp_ca_ctrl_set_ca_attr(VI_PIPE ViPipe, const ISP_CA_ATTR_S *pstCAAttr)
{
	if (pstCAAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ca_ctrl_runtime *runtime = _get_ca_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_ca_ctrl_check_ca_attr_valid(pstCAAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_CA_ATTR_S *p = CVI_NULL;

	isp_ca_ctrl_get_ca_attr(ViPipe, &p);
	memcpy((void *)p, pstCAAttr, sizeof(*pstCAAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

