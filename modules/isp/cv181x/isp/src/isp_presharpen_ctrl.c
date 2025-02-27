/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_presharpen_ctrl.c
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

#include "isp_presharpen_ctrl.h"
#include "isp_mgr_buf.h"

#include "isp_drc_ctrl.h"
#include "isp_ynr_ctrl.h"

static struct isp_presharpen_ctrl_runtime  *_get_presharpen_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_presharpen_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_PREYEE, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_presharpen_ctrl_check_presharpen_attr_valid(const ISP_PRESHARPEN_ATTR_S *pstPreSharpenAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstPreSharpenAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstPreSharpenAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstPreSharpenAttr, UpdateInterval, 0, 0xff);
	CHECK_VALID_CONST(pstPreSharpenAttr, TuningMode, 0x0, 0xb);
	// CHECK_VALID_CONST(pstPreSharpenAttr, LumaAdpGainEn, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstPreSharpenAttr, DeltaAdpGainEn, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, DeltaAdpGain, SHARPEN_LUT_NUM, 0x0, 0x3f);
	// CHECK_VALID_CONST(pstPreSharpenAttr, NoiseSuppressEnable, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstPreSharpenAttr, SatShtCtrlEn, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstPreSharpenAttr, SoftClampEnable, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstPreSharpenAttr, SoftClampUB, 0x0, 0xff);
	// CHECK_VALID_CONST(pstPreSharpenAttr, SoftClampLB, 0x0, 0xff);

	CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, LumaAdpGain, SHARPEN_LUT_NUM, 0x0, 0x3f);
	// CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, LumaCorLutIn, EE_LUT_NODE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, LumaCorLutOut, EE_LUT_NODE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, MotionCorLutIn, EE_LUT_NODE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, MotionCorLutOut, EE_LUT_NODE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, MotionCorWgtLutIn, EE_LUT_NODE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, MotionCorWgtLutOut, EE_LUT_NODE, 0x0, 0xff);

	// CHECK_VALID_AUTO_ISO_1D(pstPreSharpenAttr, GlobalGain, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstPreSharpenAttr, OverShootGain, 0x0, 0x3f);
	CHECK_VALID_AUTO_ISO_1D(pstPreSharpenAttr, UnderShootGain, 0x0, 0x3f);
	// CHECK_VALID_AUTO_ISO_1D(pstPreSharpenAttr, HFBlendWgt, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstPreSharpenAttr, MFBlendWgt, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstPreSharpenAttr, OverShootThr, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstPreSharpenAttr, UnderShootThr, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstPreSharpenAttr, OverShootThrMax, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstPreSharpenAttr, UnderShootThrMin, 0x0, 0xff);

	// CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, MotionShtGainIn, EE_LUT_NODE, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, MotionShtGainOut, EE_LUT_NODE, 0x0, 0x80);

	CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, HueShtCtrl, 4, 0x0, 0x3f);

	// CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, SatShtGainIn, EE_LUT_NODE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstPreSharpenAttr, SatShtGainOut, EE_LUT_NODE, 0x0, 0xff);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_presharpen_ctrl_get_presharpen_attr(VI_PIPE ViPipe, const ISP_PRESHARPEN_ATTR_S **pstPreSharpenAttr)
{
	if (pstPreSharpenAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_presharpen_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_PREYEE, (CVI_VOID *) &shared_buffer);

	*pstPreSharpenAttr = &shared_buffer->stPreSharpenAttr;

	return ret;
}

CVI_S32 isp_presharpen_ctrl_set_presharpen_attr(VI_PIPE ViPipe, const ISP_PRESHARPEN_ATTR_S *pstPreSharpenAttr)
{
	if (pstPreSharpenAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_presharpen_ctrl_runtime *runtime = _get_presharpen_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_presharpen_ctrl_check_presharpen_attr_valid(pstPreSharpenAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_PRESHARPEN_ATTR_S *p = CVI_NULL;

	isp_presharpen_ctrl_get_presharpen_attr(ViPipe, &p);
	memcpy((void *)p, pstPreSharpenAttr, sizeof(*pstPreSharpenAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

