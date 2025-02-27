/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_feature_ctrl.c
 * Description:
 *
 */

#include "isp_defines.h"

#include "isp_feature_ctrl.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"

#include "isp_feature.h"
#include "isp_freeze.h"
#include "isp_main_local.h"

CVI_S32 isp_feature_ctrl_get_module_info(VI_PIPE ViPipe, ISP_INNER_STATE_INFO_S *innerParam)
{
	CVI_S32 ret = CVI_SUCCESS;

	struct ccm_info ccm_info;
	struct blc_info blc_info;
	struct drc_algo_info drc_info;
	struct mlsc_info info;

	isp_ccm_ctrl_get_ccm_info(ViPipe, &ccm_info);
	isp_blc_ctrl_get_blc_info(ViPipe, &blc_info);
	isp_drc_ctrl_get_algo_info(ViPipe, &drc_info);
	isp_mlsc_ctrl_get_mlsc_info(ViPipe, &info);

	// Get ccm interpolation result.
	for (CVI_U32 i = 0; i < 9; i++) {
		innerParam->ccm[i] = ccm_info.CCM[i];
	}
	// Get blc interpolation result.
	innerParam->blcOffsetR = blc_info.OffsetR;
	innerParam->blcOffsetGr = blc_info.OffsetGr;
	innerParam->blcOffsetGb = blc_info.OffsetGb;
	innerParam->blcOffsetB = blc_info.OffsetB;
	innerParam->blcGainR = blc_info.GainR;
	innerParam->blcGainGr = blc_info.GainGr;
	innerParam->blcGainGb = blc_info.GainGb;
	innerParam->blcGainB = blc_info.GainB;

	innerParam->drcGlobalToneBinNum = drc_info.globalToneBinNum;
	innerParam->drcGlobalToneBinSEStep = drc_info.globalToneSeStep;
	// ISP_FORLOOP_SET(innerParam->drcGlobalTone, drc_info.globalToneResult, LTM_GLOBAL_CURVE_NODE_NUM);
	// ISP_FORLOOP_SET(innerParam->drcDarkTone, drc_info.darkToneResult, LTM_DARK_CURVE_NODE_NUM);
	// ISP_FORLOOP_SET(innerParam->drcBrightTone, drc_info.brightToneResult, LTM_BRIGHT_CURVE_NODE_NUM);

	// ISP_FORLOOP_SET(innerParam->mlscGainTable.RGain, info.lut_r, CVI_ISP_LSC_GRID_POINTS);
	// ISP_FORLOOP_SET(innerParam->mlscGainTable.GGain, info.lut_g, CVI_ISP_LSC_GRID_POINTS);
	// ISP_FORLOOP_SET(innerParam->mlscGainTable.BGain, info.lut_b, CVI_ISP_LSC_GRID_POINTS);

	memcpy(innerParam->drcGlobalTone, drc_info.globalToneResult, sizeof(CVI_U32) * LTM_GLOBAL_CURVE_NODE_NUM);
	memcpy(innerParam->drcDarkTone, drc_info.darkToneResult, sizeof(CVI_U32) * LTM_DARK_CURVE_NODE_NUM);
	memcpy(innerParam->drcBrightTone, drc_info.brightToneResult, sizeof(CVI_U32) * LTM_BRIGHT_CURVE_NODE_NUM);

	memcpy(innerParam->mlscGainTable.RGain, info.lut_r, sizeof(CVI_U16) * CVI_ISP_LSC_GRID_POINTS);
	memcpy(innerParam->mlscGainTable.GGain, info.lut_g, sizeof(CVI_U16) * CVI_ISP_LSC_GRID_POINTS);
	memcpy(innerParam->mlscGainTable.BGain, info.lut_b, sizeof(CVI_U16) * CVI_ISP_LSC_GRID_POINTS);

	return ret;
}
