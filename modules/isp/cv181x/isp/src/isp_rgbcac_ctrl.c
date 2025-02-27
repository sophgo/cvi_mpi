/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_rgbcac_ctrl.c
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

#include "isp_rgbcac_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_rgbcac_ctrl_runtime *_get_rgbcac_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_rgbcac_shared_buffer *shared_buf = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_RGBCAC, (CVI_VOID *) &shared_buf);

	return &shared_buf->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_rgbcac_ctrl_check_rgbcac_attr_valid(const ISP_RGBCAC_ATTR_S *pstRGBCACAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstRGBCACAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstRGBCACAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstRGBCACAttr, UpdateInterval, 1, 0xFF);
	CHECK_VALID_CONST(pstRGBCACAttr, PurpleDetRange0, 0x0, 0x80);
	CHECK_VALID_CONST(pstRGBCACAttr, PurpleDetRange1, 0x0, 0x80);
	// CHECK_VALID_CONST(pstRGBCACAttr, DePurpleStrMax0, 0x0, 0xFF);
	// CHECK_VALID_CONST(pstRGBCACAttr, DePurpleStrMin0, 0x0, 0xFF);
	// CHECK_VALID_CONST(pstRGBCACAttr, DePurpleStrMax1, 0x0, 0xFF);
	// CHECK_VALID_CONST(pstRGBCACAttr, DePurpleStrMin1, 0x0, 0xFF);
	CHECK_VALID_CONST(pstRGBCACAttr, EdgeGlobalGain, 0x0, 0xFFF);
	CHECK_VALID_ARRAY_1D(pstRGBCACAttr, EdgeGainIn, 3, 0x0, 0xFFF);
	CHECK_VALID_ARRAY_1D(pstRGBCACAttr, EdgeGainOut, 3, 0x0, 0xFFF);
	CHECK_VALID_CONST(pstRGBCACAttr, LumaScale, 0x0, 0x7FF);
	CHECK_VALID_CONST(pstRGBCACAttr, UserDefineLuma, 0x0, 0xFFF);
	CHECK_VALID_CONST(pstRGBCACAttr, LumaBlendWgt, 0x0, 0x20);
	CHECK_VALID_CONST(pstRGBCACAttr, LumaBlendWgt2, 0x0, 0x20);
	CHECK_VALID_CONST(pstRGBCACAttr, LumaBlendWgt3, 0x0, 0x20);
	CHECK_VALID_CONST(pstRGBCACAttr, PurpleCb, 0x0, 0xFFF);
	CHECK_VALID_CONST(pstRGBCACAttr, PurpleCr, 0x0, 0xFFF);
	CHECK_VALID_CONST(pstRGBCACAttr, PurpleCb2, 0x0, 0xFFF);
	CHECK_VALID_CONST(pstRGBCACAttr, PurpleCr2, 0x0, 0xFFF);
	CHECK_VALID_CONST(pstRGBCACAttr, PurpleCb3, 0x0, 0xFFF);
	CHECK_VALID_CONST(pstRGBCACAttr, PurpleCr3, 0x0, 0xFFF);
	CHECK_VALID_CONST(pstRGBCACAttr, GreenCb, 0x0, 0xFFF);
	CHECK_VALID_CONST(pstRGBCACAttr, GreenCr, 0x0, 0xFFF);
	CHECK_VALID_CONST(pstRGBCACAttr, TuningMode, 0x0, 0x2);
	// CHECK_VALID_AUTO_ISO_1D(pstRGBCACAttr, DePurpleStr0, 0x0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstRGBCACAttr, DePurpleStr1, 0x0, 0xFF);
	CHECK_VALID_AUTO_ISO_1D(pstRGBCACAttr, EdgeCoring, 0x0, 0xFFF);
	CHECK_VALID_AUTO_ISO_1D(pstRGBCACAttr, DePurpleCrStr0, 0x0, 0x10);
	CHECK_VALID_AUTO_ISO_1D(pstRGBCACAttr, DePurpleCbStr0, 0x0, 0x10);
	CHECK_VALID_AUTO_ISO_1D(pstRGBCACAttr, DePurpleCrStr1, 0x0, 0x10);
	CHECK_VALID_AUTO_ISO_1D(pstRGBCACAttr, DePurpleCbStr1, 0x0, 0x10);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_rgbcac_ctrl_get_rgbcac_attr(VI_PIPE ViPipe, const ISP_RGBCAC_ATTR_S **pstRGBCACAttr)
{
	if (pstRGBCACAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_rgbcac_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_RGBCAC, (CVI_VOID *) &shared_buffer);
	*pstRGBCACAttr = &shared_buffer->stRGBCACAttr;

	return ret;
}

CVI_S32 isp_rgbcac_ctrl_set_rgbcac_attr(VI_PIPE ViPipe, const ISP_RGBCAC_ATTR_S *pstRGBCACAttr)
{
	if (pstRGBCACAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_rgbcac_ctrl_runtime *runtime = _get_rgbcac_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_rgbcac_ctrl_check_rgbcac_attr_valid(pstRGBCACAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_RGBCAC_ATTR_S *p = CVI_NULL;

	isp_rgbcac_ctrl_get_rgbcac_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstRGBCACAttr, sizeof(*pstRGBCACAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

