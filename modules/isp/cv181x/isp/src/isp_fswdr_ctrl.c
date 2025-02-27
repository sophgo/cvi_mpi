/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_fswdr_ctrl.c
 * Description:
 *
 */

#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"
#include "isp_ioctl.h"
#include "isp_mw_compat.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"
#include "cvi_isp.h"
#include "cvi_ae.h"

#include "isp_fswdr_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_fswdr_ctrl_runtime  *_get_fswdr_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_fswdr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_FUSION, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_fswdr_ctrl_check_fswdr_attr_valid(const ISP_FSWDR_ATTR_S *pstFSWDRAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CVI_BOOL CHECK_VALID_CONST(pstFSWDRAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstFSWDRAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstFSWDRAttr, UpdateInterval, 0x1, 0xFF);
	// CHECK_VALID_CONST(pstFSWDRAttr, MotionCompEnable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstFSWDRAttr, TuningMode, 0x0, 0x9);
	// CHECK_VALID_CONST(pstFSWDRAttr, WDRDCMode, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstFSWDRAttr, WDRLumaMode, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstFSWDRAttr, WDRType, 0x0, 0x2);
	// CHECK_VALID_CONST(pstFSWDRAttr, WDRCombineSNRAwareEn, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstFSWDRAttr, WDRCombineSNRAwareLowThr, 0x0, 0xffff);
	// CHECK_VALID_CONST(pstFSWDRAttr, WDRCombineSNRAwareHighThr, 0x0, 0xffff);
	CHECK_VALID_CONST(pstFSWDRAttr, WDRCombineSNRAwareSmoothLevel, 0x1, 0xbb8);
	// CHECK_VALID_CONST(pstFSWDRAttr, LocalToneRefinedDCMode, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstFSWDRAttr, LocalToneRefinedLumaMode, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstFSWDRAttr, DarkToneRefinedThrL, 0x0, 0xfff);
	CHECK_VALID_CONST(pstFSWDRAttr, DarkToneRefinedThrH, 0x0, 0xfff);
	CHECK_VALID_CONST(pstFSWDRAttr, DarkToneRefinedMaxWeight, 0x0, 0x100);
	CHECK_VALID_CONST(pstFSWDRAttr, DarkToneRefinedMinWeight, 0x0, 0x100);
	CHECK_VALID_CONST(pstFSWDRAttr, BrightToneRefinedThrL, 0x0, 0xfff);
	CHECK_VALID_CONST(pstFSWDRAttr, BrightToneRefinedThrH, 0x0, 0xfff);
	CHECK_VALID_CONST(pstFSWDRAttr, BrightToneRefinedMaxWeight, 0x0, 0x100);
	CHECK_VALID_CONST(pstFSWDRAttr, BrightToneRefinedMinWeight, 0x0, 0x100);
	CHECK_VALID_CONST(pstFSWDRAttr, WDRMotionFusionMode, 0x0, 0x3);
	// CHECK_VALID_CONST(pstFSWDRAttr, MtMode, CVI_FALSE, CVI_TRUE);

	CHECK_VALID_AUTO_LV_1D(pstFSWDRAttr, WDRCombineLongThr, 0x0, 0xfff);
	CHECK_VALID_AUTO_LV_1D(pstFSWDRAttr, WDRCombineShortThr, 0x0, 0xfff);
	CHECK_VALID_AUTO_LV_1D(pstFSWDRAttr, WDRCombineMaxWeight, 0x0, 0x100);
	CHECK_VALID_AUTO_LV_1D(pstFSWDRAttr, WDRCombineMinWeight, 0x0, 0x100);
	// CHECK_VALID_AUTO_LV_2D(pstFSWDRAttr, WDRMtIn, 4, 0x0, 0xff);
	CHECK_VALID_AUTO_LV_2D(pstFSWDRAttr, WDRMtOut, 4, 0x0, 0x100);
	CHECK_VALID_AUTO_LV_1D(pstFSWDRAttr, WDRLongWgt, 0x0, 0x100);
	// CHECK_VALID_AUTO_LV_1D(pstFSWDRAttr, WDRCombineSNRAwareToleranceLevel, 0x0, 0xff);
	// CHECK_VALID_AUTO_LV_1D(pstFSWDRAttr, MergeModeAlpha, 0x0, 0xff);
	CHECK_VALID_AUTO_LV_1D(pstFSWDRAttr, WDRMotionCombineLongThr, 0x0, 0xfff);
	CHECK_VALID_AUTO_LV_1D(pstFSWDRAttr, WDRMotionCombineShortThr, 0x0, 0xfff);
	CHECK_VALID_AUTO_LV_1D(pstFSWDRAttr, WDRMotionCombineMinWeight, 0x0, 0x100);
	CHECK_VALID_AUTO_LV_1D(pstFSWDRAttr, WDRMotionCombineMaxWeight, 0x0, 0x100);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_fswdr_ctrl_get_fswdr_attr(VI_PIPE ViPipe, const ISP_FSWDR_ATTR_S **pstFSWDRAttr)
{
	if (pstFSWDRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_fswdr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_FUSION, (CVI_VOID *) &shared_buffer);
	*pstFSWDRAttr = &shared_buffer->stFSWDRAttr;

	return ret;
}

CVI_S32 isp_fswdr_ctrl_set_fswdr_attr(VI_PIPE ViPipe, const ISP_FSWDR_ATTR_S *pstFSWDRAttr)
{
	if (pstFSWDRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_fswdr_ctrl_runtime *runtime = _get_fswdr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_fswdr_ctrl_check_fswdr_attr_valid(pstFSWDRAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_FSWDR_ATTR_S *p = CVI_NULL;

	isp_fswdr_ctrl_get_fswdr_attr(ViPipe, &p);

	memcpy((CVI_VOID *) p, pstFSWDRAttr, sizeof(*pstFSWDRAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_fswdr_ctrl_get_fswdr_info(VI_PIPE ViPipe, struct fswdr_info *info)
{
	CVI_S32 ret = CVI_SUCCESS;
	struct isp_fswdr_ctrl_runtime *runtime = _get_fswdr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	info->WDRCombineLongThr = runtime->fswdr_attr.WDRCombineLongThr;
	info->WDRCombineShortThr = runtime->fswdr_attr.WDRCombineShortThr;
	info->WDRCombineMinWeight = runtime->fswdr_attr.WDRCombineMinWeight;

	return ret;
}


