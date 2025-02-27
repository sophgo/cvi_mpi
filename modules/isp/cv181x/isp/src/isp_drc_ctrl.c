/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_drc_ctrl.c
 * Description:
 *
 */

#include <stdlib.h>
#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"
#include "isp_ioctl.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"
#include "cvi_isp.h"
#include "cvi_ae.h"
#include "isp_3a.h"
#include "isp_sts_ctrl.h"
#include "isp_lut.h"

#include "isp_drc_ctrl.h"
#include "isp_gamma_ctrl.h"
#include "isp_fswdr_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_drc_ctrl_runtime  *_get_drc_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_drc_shared_buffer *shared_buf = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DRC, (CVI_VOID *) &shared_buf);

	return &shared_buf->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_drc_ctrl_check_drc_attr_valid(const ISP_DRC_ATTR_S *pstDRCAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstDRCAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstDRCAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstDRCAttr, UpdateInterval, 0, 0xff);
	CHECK_VALID_CONST(pstDRCAttr, TuningMode, 0x0, 0x7);
	// CHECK_VALID_CONST(pstDRCAttr, LocalToneEn, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstDRCAttr, LocalToneRefineEn, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstDRCAttr, ToneCurveSelect, 0x0, 0x1);
	// CHECK_VALID_ARRAY_1D(pstDRCAttr, CurveUserDefine, DRC_GLOBAL_USER_DEFINE_NUM, 0x0, 0xffff);
	// CHECK_VALID_ARRAY_1D(pstDRCAttr, DarkUserDefine, DRC_DARK_USER_DEFINE_NUM, 0x0, 0xffff);
	// CHECK_VALID_ARRAY_1D(pstDRCAttr, BrightUserDefine, DRC_BRIGHT_USER_DEFINE_NUM, 0x0, 0xffff);
	CHECK_VALID_CONST(pstDRCAttr, ToneCurveSmooth, 0x0, 0x1f4);
	CHECK_VALID_CONST(pstDRCAttr, CoarseFltScale, 0x3, 0x6);
	CHECK_VALID_CONST(pstDRCAttr, SdrTargetYGainMode, 0x0, 0x1);
	// CHECK_VALID_CONST(pstDRCAttr, DetailEnhanceEn, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_ARRAY_1D(pstDRCAttr, LumaGain, 33, 0x0, 0xff);
	// CHECK_VALID_ARRAY_1D(pstDRCAttr, DetailEnhanceMtIn, 4, 0x0, 0xff);
	CHECK_VALID_ARRAY_1D(pstDRCAttr, DetailEnhanceMtOut, 4, 0x0, 0x100);
	// CHECK_VALID_CONST(pstDRCAttr, OverShootThd, 0x0, 0xff);
	// CHECK_VALID_CONST(pstDRCAttr, UnderShootThd, 0x0, 0xff);
	CHECK_VALID_CONST(pstDRCAttr, OverShootGain, 0x0, 0x3f);
	CHECK_VALID_CONST(pstDRCAttr, UnderShootGain, 0x0, 0x3f);
	// CHECK_VALID_CONST(pstDRCAttr, OverShootThdMax, 0x0, 0xff);
	// CHECK_VALID_CONST(pstDRCAttr, UnderShootThdMin, 0x0, 0xff);
	// CHECK_VALID_CONST(pstDRCAttr, SoftClampEnable, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstDRCAttr, SoftClampUB, 0x0, 0xff);
	// CHECK_VALID_CONST(pstDRCAttr, SoftClampLB, 0x0, 0xff);
	// CHECK_VALID_CONST(pstDRCAttr, dbg_182x_sim_enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstDRCAttr, DarkMapStr, 0x0, 0x80);
	CHECK_VALID_CONST(pstDRCAttr, BritMapStr, 0x0, 0x80);
	CHECK_VALID_CONST(pstDRCAttr, SdrDarkMapStr, 0x0, 0x80);
	CHECK_VALID_CONST(pstDRCAttr, SdrBritMapStr, 0x0, 0x80);


	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, TargetYScale, 0x0, 0x800);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, HdrStrength, 0x0, 0x100);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, DEAdaptPercentile, 0x0, 0x19);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, DEAdaptTargetGain, 0x1, 0x60);
	// CHECK_VALID_AUTO_LV_1D(pstDRCAttr, DEAdaptGainUB, 0x1, 0xff);
	// CHECK_VALID_AUTO_LV_1D(pstDRCAttr, DEAdaptGainLB, 0x1, 0xff);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, BritInflectPtLuma, 0x0, 0x64);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, BritContrastLow, 0x0, 0x64);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, BritContrastHigh, 0x0, 0x64);
	// CHECK_VALID_AUTO_LV_1D(pstDRCAttr, SdrTargetY, 0x0, 0xff);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, SdrTargetYGain, 0x20, 0x80);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, SdrGlobalToneStr, 0x0, 0x100);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, SdrDEAdaptPercentile, 0x0, 0x19);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, SdrDEAdaptTargetGain, 0x1, 0x40);
	// CHECK_VALID_AUTO_LV_1D(pstDRCAttr, SdrDEAdaptGainLB, 0x1, 0xff);
	// CHECK_VALID_AUTO_LV_1D(pstDRCAttr, SdrDEAdaptGainUB, 0x1, 0xff);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, SdrBritInflectPtLuma, 0x0, 0x64);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, SdrBritContrastLow, 0x0, 0x64);
	CHECK_VALID_AUTO_LV_1D(pstDRCAttr, SdrBritContrastHigh, 0x0, 0x64);
	CHECK_VALID_AUTO_ISO_1D(pstDRCAttr, TotalGain, 0x0, 0xff);

	return ret;
}

static CVI_S32 isp_drc_ctrl_set_drc_attr_compatible(VI_PIPE ViPipe, ISP_DRC_ATTR_S *pstDRCAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	UNUSED(ViPipe);
	UNUSED(pstDRCAttr);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_drc_ctrl_get_drc_attr(VI_PIPE ViPipe, const ISP_DRC_ATTR_S **pstDRCAttr)
{
	if (pstDRCAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_drc_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DRC, (CVI_VOID *) &shared_buffer);
	*pstDRCAttr = &shared_buffer->stDRCAttr;

	return ret;
}

static CVI_S32 isp_drc_ctrl_modify_rgbgamma_for_drc_tuning_mode(VI_PIPE ViPipe, ISP_DRC_ATTR_S *pstDRCAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	struct isp_drc_ctrl_runtime *runtime = _get_drc_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_BOOL bModifyRGBGamma = CVI_FALSE;

	bModifyRGBGamma = (pstDRCAttr->Enable)
		&& ((pstDRCAttr->TuningMode == 1) || (pstDRCAttr->TuningMode == 2));

	if (runtime->modify_rgbgamma_for_drc_tuning != bModifyRGBGamma) {
		runtime->modify_rgbgamma_for_drc_tuning = bModifyRGBGamma;

		isp_gamma_ctrl_set_rgbgamma_curve(ViPipe,
			(runtime->modify_rgbgamma_for_drc_tuning) ? CURVE_DRC_TUNING_CURVE : CURVE_DEFAULT);
	}

	return ret;
}

CVI_S32 isp_drc_ctrl_set_drc_attr(VI_PIPE ViPipe, const ISP_DRC_ATTR_S *pstDRCAttr)
{
	if (pstDRCAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_drc_ctrl_runtime *runtime = _get_drc_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_drc_ctrl_check_drc_attr_valid(pstDRCAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_DRC_ATTR_S *p = CVI_NULL;

	isp_drc_ctrl_get_drc_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstDRCAttr, sizeof(*pstDRCAttr));

	isp_drc_ctrl_set_drc_attr_compatible(ViPipe, (ISP_DRC_ATTR_S *) p);

	isp_drc_ctrl_modify_rgbgamma_for_drc_tuning_mode(ViPipe, (ISP_DRC_ATTR_S *) p);

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_drc_ctrl_get_algo_info(VI_PIPE ViPipe, struct drc_algo_info *info)
{
	CVI_S32 ret = CVI_SUCCESS;
	struct isp_drc_ctrl_runtime *runtime = _get_drc_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	info->globalToneBinNum = runtime->drc_param_out.ltm_global_tone_bin_num;
	info->globalToneSeStep = runtime->drc_param_out.ltm_global_tone_bin_se_step;
	info->globalToneResult = runtime->drc_param_out.reg_ltm_deflt_lut;
	info->darkToneResult = runtime->drc_param_out.reg_ltm_dark_lut;
	info->brightToneResult = runtime->drc_param_out.reg_ltm_brit_lut;

	info->drc_g_curve_1_quan_bit = runtime->drc_param_out.reg_ltm_g_curve_1_quan_bit;

	return ret;
}

