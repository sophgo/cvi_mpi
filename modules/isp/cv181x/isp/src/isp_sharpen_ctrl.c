/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_sharpen_ctrl.c
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

#include "isp_sharpen_ctrl.h"
#include "isp_mgr_buf.h"

#include "isp_drc_ctrl.h"
#include "isp_ynr_ctrl.h"

static struct isp_sharpen_ctrl_runtime  *_get_sharpen_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_sharpen_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_YEE, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_sharpen_ctrl_check_sharpen_attr_valid(const ISP_SHARPEN_ATTR_S *pstSharpenAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstSharpenAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstSharpenAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstSharpenAttr, UpdateInterval, 0, 0xff);
	CHECK_VALID_CONST(pstSharpenAttr, TuningMode, 0x0, 0xb);
	// CHECK_VALID_CONST(pstSharpenAttr, LumaAdpGainEn, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstSharpenAttr, DeltaAdpGainEn, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, DeltaAdpGain, SHARPEN_LUT_NUM, 0x0, 0x3f);
	// CHECK_VALID_CONST(pstSharpenAttr, NoiseSuppressEnable, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstSharpenAttr, SatShtCtrlEn, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstSharpenAttr, SoftClampEnable, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstSharpenAttr, SoftClampUB, 0x0, 0xff);
	// CHECK_VALID_CONST(pstSharpenAttr, SoftClampLB, 0x0, 0xff);

	CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, LumaAdpGain, SHARPEN_LUT_NUM, 0x0, 0x3f);
	// CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, LumaCorLutIn, EE_LUT_NODE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, LumaCorLutOut, EE_LUT_NODE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, MotionCorLutIn, EE_LUT_NODE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, MotionCorLutOut, EE_LUT_NODE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, MotionCorWgtLutIn, EE_LUT_NODE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, MotionCorWgtLutOut, EE_LUT_NODE, 0x0, 0xff);

	// CHECK_VALID_AUTO_ISO_1D(pstSharpenAttr, GlobalGain, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstSharpenAttr, OverShootGain, 0x0, 0x3f);
	CHECK_VALID_AUTO_ISO_1D(pstSharpenAttr, UnderShootGain, 0x0, 0x3f);
	// CHECK_VALID_AUTO_ISO_1D(pstSharpenAttr, HFBlendWgt, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstSharpenAttr, MFBlendWgt, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstSharpenAttr, OverShootThr, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstSharpenAttr, UnderShootThr, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstSharpenAttr, OverShootThrMax, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstSharpenAttr, UnderShootThrMin, 0x0, 0xff);

	// CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, MotionShtGainIn, EE_LUT_NODE, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, MotionShtGainOut, EE_LUT_NODE, 0x0, 0x80);

	CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, HueShtCtrl, 4, 0x0, 0x3f);

	// CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, SatShtGainIn, EE_LUT_NODE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstSharpenAttr, SatShtGainOut, EE_LUT_NODE, 0x0, 0xff);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_sharpen_ctrl_get_sharpen_attr(VI_PIPE ViPipe, const ISP_SHARPEN_ATTR_S **pstSharpenAttr)
{
	if (pstSharpenAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_sharpen_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_YEE, (CVI_VOID *) &shared_buffer);

	*pstSharpenAttr = &shared_buffer->stSharpenAttr;

	return ret;
}

CVI_S32 isp_sharpen_ctrl_set_sharpen_attr(VI_PIPE ViPipe, const ISP_SHARPEN_ATTR_S *pstSharpenAttr)
{
	if (pstSharpenAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_sharpen_ctrl_runtime *runtime = _get_sharpen_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_sharpen_ctrl_check_sharpen_attr_valid(pstSharpenAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_SHARPEN_ATTR_S *p = CVI_NULL;

	isp_sharpen_ctrl_get_sharpen_attr(ViPipe, &p);
	memcpy((void *)p, pstSharpenAttr, sizeof(*pstSharpenAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

