/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_sts_ctrl.c
 * Description:
 *
 */

#include "cvi_isp.h"
#include "cvi_awb.h"

#include "isp_main_local.h"
#include "isp_ioctl.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_dis_ctrl.h"
#include "isp_motion_ctrl.h"
#include "isp_sts_ctrl.h"
#include "isp_mw_compat.h"
#include "isp_mgr_buf.h"

struct isp_sts_ctrl_runtime *sts_ctrl_runtime[VI_MAX_PIPE_NUM];

//-----------------------------------------------------------------------------
//  statistics configuration
//-----------------------------------------------------------------------------

static struct isp_sts_ctrl_runtime **_get_sts_ctrl_runtime(VI_PIPE ViPipe);

// 1822 AE footprint
//  AE     : 6 * AE_ZONE_ROW * (256 / 8) =  2880 = 0x0B40 byte (Assume AE_ZONE_ROW = 15)
//  AE_HIST:                             =  8192 = 0x2000 byte
//  FACE_AE:                             =   128 = 0x0080 byte
//  TOTAL  : (AE + AE_HIST + FACE_AE) * 2= 22670 = 0x588E byte
CVI_S32 isp_sts_ctrl_get_ae_sts(VI_PIPE ViPipe, ISP_AE_STATISTICS_COMPAT_S **ae_sts)
{
	if (ae_sts == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_sts_ctrl_runtime **runtime_ptr = _get_sts_ctrl_runtime(ViPipe);
	struct isp_sts_ctrl_runtime *runtime = *runtime_ptr;

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	*ae_sts = &(runtime->ae_sts);

	return ret;
}

CVI_S32 isp_sts_ctrl_get_awb_sts(VI_PIPE ViPipe, ISP_CHANNEL_LIST_E chn, ISP_WB_STATISTICS_S **awb_sts)
{
	if (awb_sts == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_sts_ctrl_runtime **runtime_ptr = _get_sts_ctrl_runtime(ViPipe);
	struct isp_sts_ctrl_runtime *runtime = *runtime_ptr;

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	*awb_sts = &(runtime->awb_sts[chn]);

	return ret;
}

CVI_S32 isp_sts_ctrl_get_af_sts(VI_PIPE ViPipe, ISP_AF_STATISTICS_S **af_sts)
{
	if (af_sts == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_sts_ctrl_runtime **runtime_ptr = _get_sts_ctrl_runtime(ViPipe);
	struct isp_sts_ctrl_runtime *runtime = *runtime_ptr;

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	*af_sts = &(runtime->af_sts);

	return ret;
}

CVI_S32 isp_sts_ctrl_get_gms_sts(VI_PIPE ViPipe, ISP_DIS_STATS_INFO **disStatsInfo)
{
	if (disStatsInfo == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_sts_ctrl_runtime **runtime_ptr = _get_sts_ctrl_runtime(ViPipe);
	struct isp_sts_ctrl_runtime *runtime = *runtime_ptr;

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	*disStatsInfo = &(runtime->gms_sts);

	return ret;
}

CVI_S32 isp_sts_ctrl_get_dci_sts(VI_PIPE ViPipe, ISP_DCI_STATISTICS_S **dciStatsInfo)
{
	if (dciStatsInfo == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_sts_ctrl_runtime **runtime_ptr = _get_sts_ctrl_runtime(ViPipe);
	struct isp_sts_ctrl_runtime *runtime = *runtime_ptr;

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	*dciStatsInfo = &(runtime->dci_sts);

	return ret;
}

CVI_S32 isp_sts_ctrl_get_motion_sts(VI_PIPE ViPipe, ISP_MOTION_STATS_INFO **motionStatsInfo)
{
	if (motionStatsInfo == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_sts_ctrl_runtime **runtime_ptr = _get_sts_ctrl_runtime(ViPipe);
	struct isp_sts_ctrl_runtime *runtime = *runtime_ptr;

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	*motionStatsInfo = &(runtime->motion_sts);

	return ret;
}

CVI_S32 isp_sts_ctrl_get_pre_be_frm_idx(VI_PIPE ViPipe, CVI_U32 *frame_cnt)
{
	CVI_S32 ret = CVI_SUCCESS;
	struct isp_sts_ctrl_runtime **runtime_ptr = _get_sts_ctrl_runtime(ViPipe);
	struct isp_sts_ctrl_runtime *runtime = *runtime_ptr;

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	*frame_cnt = runtime->pre_frame_cnt;

	return ret;
}

static struct isp_sts_ctrl_runtime  **_get_sts_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	if (sts_ctrl_runtime[ViPipe] == CVI_NULL) {
		isp_mgr_buf_get_sts_ctrl_runtime_addr(ViPipe,
			(CVI_VOID *) &sts_ctrl_runtime[ViPipe]);
	}

	return &sts_ctrl_runtime[ViPipe];
}
