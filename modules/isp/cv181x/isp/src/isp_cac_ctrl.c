/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_cac_ctrl.c
 * Description:
 *
 */

#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"

#include "isp_cac_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_cac_ctrl_runtime *_get_cac_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_cac_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CAC, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_cac_ctrl_check_cac_attr_valid(const ISP_CAC_ATTR_S *pstCACAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstCACAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstCACAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstCACAttr, UpdateInterval, 1, 0xFF);
	CHECK_VALID_CONST(pstCACAttr, PurpleDetRange, 0, 0x80);
	// CHECK_VALID_CONST(pstCACAttr, PurpleCb, 0x0, 0xFF);
	// CHECK_VALID_CONST(pstCACAttr, PurpleCr, 0x0, 0xFF);
	// CHECK_VALID_CONST(pstCACAttr, PurpleCb2, 0x0, 0xFF);
	// CHECK_VALID_CONST(pstCACAttr, PurpleCr2, 0x0, 0xFF);
	// CHECK_VALID_CONST(pstCACAttr, PurpleCb3, 0x0, 0xFF);
	// CHECK_VALID_CONST(pstCACAttr, PurpleCr3, 0x0, 0xFF);
	// CHECK_VALID_CONST(pstCACAttr, GreenCb, 0x0, 0xFF);
	// CHECK_VALID_CONST(pstCACAttr, GreenCr, 0x0, 0xFF);
	CHECK_VALID_CONST(pstCACAttr, TuningMode, 0x0, 0x2);
	CHECK_VALID_ARRAY_1D(pstCACAttr, EdgeGainIn, 3, 0x0, 0x10);
	CHECK_VALID_ARRAY_1D(pstCACAttr, EdgeGainOut, 3, 0x0, 0x20);
	// CHECK_VALID_AUTO_ISO_1D(pstCACAttr, DePurpleStr, 0x0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstCACAttr, EdgeGlobalGain, 0x0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstCACAttr, EdgeCoring, 0x0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstCACAttr, EdgeStrMin, 0x0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstCACAttr, EdgeStrMax, 0x0, 0xFF);
	CHECK_VALID_AUTO_ISO_1D(pstCACAttr, DePurpleCbStr, 0x0, 0x8);
	CHECK_VALID_AUTO_ISO_1D(pstCACAttr, DePurpleCrStr, 0x0, 0x8);
	CHECK_VALID_AUTO_ISO_1D(pstCACAttr, DePurpleStrMaxRatio, 0x0, 0x40);
	CHECK_VALID_AUTO_ISO_1D(pstCACAttr, DePurpleStrMinRatio, 0x0, 0x40);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_cac_ctrl_get_cac_attr(VI_PIPE ViPipe, const ISP_CAC_ATTR_S **pstCACAttr)
{
	if (pstCACAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_cac_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CAC, (CVI_VOID *) &shared_buffer);
	*pstCACAttr = &shared_buffer->stCACAttr;

	return ret;
}

CVI_S32 isp_cac_ctrl_set_cac_attr(VI_PIPE ViPipe, const ISP_CAC_ATTR_S *pstCACAttr)
{
	if (pstCACAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_cac_ctrl_runtime *runtime = _get_cac_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_cac_ctrl_check_cac_attr_valid(pstCACAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_CAC_ATTR_S *p = CVI_NULL;

	isp_cac_ctrl_get_cac_attr(ViPipe, &p);
	memcpy((void *)p, pstCACAttr, sizeof(*pstCACAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

