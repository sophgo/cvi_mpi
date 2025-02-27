/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_tnr_ctrl.c
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
#include "cvi_isp.h"

#include "isp_tnr_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_tnr_ctrl_runtime  *_get_tnr_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_tnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_3DNR, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_tnr_ctrl_check_tnr_attr_valid(const ISP_TNR_ATTR_S *pstTNRAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstTNRAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstTNRAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstTNRAttr, UpdateInterval, 0, 0xff);
	// CHECK_VALID_CONST(pstTNRAttr, TuningMode, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstTNRAttr, TnrMtMode, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstTNRAttr, YnrCnrSharpenMtMode, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstTNRAttr, PreSharpenMtMode, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstTNRAttr, ChromaScalingDownMode, 0x0, 0x3);
	// CHECK_VALID_CONST(pstTNRAttr, CompGainEnable, CVI_FALSE, CVI_TRUE);

	// CHECK_VALID_AUTO_ISO_1D(pstTNRAttr, TnrStrength0, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRAttr, MapThdLow0, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRAttr, MapThdHigh0, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstTNRAttr, MtDetectUnit, 0x0, 0x5);
	CHECK_VALID_AUTO_ISO_1D(pstTNRAttr, BrightnessNoiseLevelLE, 0x1, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstTNRAttr, BrightnessNoiseLevelSE, 0x1, 0x3ff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRAttr, MtFiltMode, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_AUTO_ISO_1D(pstTNRAttr, MtFiltWgt, 0x0, 0x100);

	return ret;
}

static CVI_S32 isp_tnr_ctrl_check_tnr_noise_model_attr_valid(const ISP_TNR_NOISE_MODEL_ATTR_S *pstTNRNoiseModelAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_AUTO_ISO_1D(pstTNRNoiseModelAttr, RNoiseLevel0, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRNoiseModelAttr, GNoiseLevel0, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRNoiseModelAttr, BNoiseLevel0, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRNoiseModelAttr, RNoiseLevel1, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRNoiseModelAttr, GNoiseLevel1, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRNoiseModelAttr, BNoiseLevel1, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRNoiseModelAttr, RNoiseHiLevel0, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRNoiseModelAttr, GNoiseHiLevel0, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRNoiseModelAttr, BNoiseHiLevel0, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRNoiseModelAttr, RNoiseHiLevel1, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRNoiseModelAttr, GNoiseHiLevel1, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRNoiseModelAttr, BNoiseHiLevel1, 0x0, 0xff);

	UNUSED(pstTNRNoiseModelAttr);

	return ret;
}

static CVI_S32 isp_tnr_ctrl_check_tnr_luma_motion_attr_valid(const ISP_TNR_LUMA_MOTION_ATTR_S *pstTNRLumaMotionAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	CHECK_VALID_AUTO_ISO_2D(pstTNRLumaMotionAttr, L2mIn0, 4, 0x0, 0xfff);
	CHECK_VALID_AUTO_ISO_2D(pstTNRLumaMotionAttr, L2mOut0, 4, 0x0, 0x3f);
	CHECK_VALID_AUTO_ISO_2D(pstTNRLumaMotionAttr, L2mIn1, 4, 0x0, 0xfff);
	CHECK_VALID_AUTO_ISO_2D(pstTNRLumaMotionAttr, L2mOut1, 4, 0x0, 0x3f);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRLumaMotionAttr, MtLumaMode, CVI_FALSE, CVI_TRUE);

	return ret;
}

static CVI_S32 isp_tnr_ctrl_check_tnr_ghost_attr_valid(const ISP_TNR_GHOST_ATTR_S *pstTNRGhostAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_AUTO_ISO_2D(pstTNRGhostAttr, PrvMotion0, 4, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_2D(pstTNRGhostAttr, PrtctWgt0, 4, 0x0, 0xf);
	CHECK_VALID_AUTO_ISO_1D(pstTNRGhostAttr, MotionHistoryStr, 0x0, 0xf);

	return ret;
}

static CVI_S32 isp_tnr_ctrl_check_tnr_mt_prt_attr_valid(const ISP_TNR_MT_PRT_ATTR_S *pstTNRMtPrtAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstTNRMtPrtAttr, LowMtPrtEn, CVI_FALSE, CVI_TRUE);

	// CHECK_VALID_AUTO_ISO_1D(pstTNRMtPrtAttr, LowMtPrtLevelY, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRMtPrtAttr, LowMtPrtLevelU, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstTNRMtPrtAttr, LowMtPrtLevelV, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstTNRMtPrtAttr, LowMtPrtInY, 4, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstTNRMtPrtAttr, LowMtPrtInU, 4, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstTNRMtPrtAttr, LowMtPrtInV, 4, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstTNRMtPrtAttr, LowMtPrtOutY, 4, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstTNRMtPrtAttr, LowMtPrtOutU, 4, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstTNRMtPrtAttr, LowMtPrtOutV, 4, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstTNRMtPrtAttr, LowMtPrtAdvIn, 4, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstTNRMtPrtAttr, LowMtPrtAdvOut, 4, 0x0, 0xff);

	UNUSED(pstTNRMtPrtAttr);

	return ret;
}

static CVI_S32 isp_tnr_ctrl_check_tnr_motion_adapt_attr_valid(const ISP_TNR_MOTION_ADAPT_ATTR_S *pstTNRMotionAdaptAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_AUTO_ISO_2D(pstTNRMotionAdaptAttr, AdaptNrLumaStrIn, 4, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstTNRMotionAdaptAttr, AdaptNrLumaStrOut, 4, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstTNRMotionAdaptAttr, AdaptNrChromaStrIn, 4, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstTNRMotionAdaptAttr, AdaptNrChromaStrOut, 4, 0x0, 0xff);

	UNUSED(pstTNRMotionAdaptAttr);

	return ret;
}

static CVI_S32 isp_tnr_ctrl_set_tnr_mt_prt_attr_compatible(VI_PIPE ViPipe, ISP_TNR_MT_PRT_ATTR_S *pstTNRMtPrtAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	UNUSED(ViPipe);
	UNUSED(pstTNRMtPrtAttr);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_tnr_ctrl_get_tnr_internal_attr(VI_PIPE ViPipe, ISP_TNR_INTER_ATTR_S *pstTNRInterAttr)
{
	if (pstTNRInterAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_tnr_ctrl_runtime *runtime = _get_tnr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	*pstTNRInterAttr = runtime->tnr_internal_attr;

	return ret;
}

CVI_S32 isp_tnr_ctrl_set_tnr_internal_attr(VI_PIPE ViPipe, const ISP_TNR_INTER_ATTR_S *pstTNRInterAttr)
{
	if (pstTNRInterAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_tnr_ctrl_runtime *runtime = _get_tnr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	runtime->tnr_internal_attr = *pstTNRInterAttr;
	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_tnr_ctrl_get_tnr_attr(VI_PIPE ViPipe, const ISP_TNR_ATTR_S **pstTNRAttr)
{
	if (pstTNRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_tnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_3DNR, (CVI_VOID *) &shared_buffer);

	*pstTNRAttr = &shared_buffer->stTNRAttr;

	return ret;
}

CVI_S32 isp_tnr_ctrl_set_tnr_attr(VI_PIPE ViPipe, const ISP_TNR_ATTR_S *pstTNRAttr)
{
	if (pstTNRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_tnr_ctrl_runtime *runtime = _get_tnr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_tnr_ctrl_check_tnr_attr_valid(pstTNRAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_TNR_ATTR_S *p = CVI_NULL;

	isp_tnr_ctrl_get_tnr_attr(ViPipe, &p);
	memcpy((CVI_VOID *)p, pstTNRAttr, sizeof(*pstTNRAttr));

	runtime->tnr_internal_attr.bUpdateMotionUnit = CVI_TRUE;
	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_tnr_ctrl_get_tnr_noise_model_attr(VI_PIPE ViPipe, const ISP_TNR_NOISE_MODEL_ATTR_S **pstTNRNoiseModelAttr)
{
	if (pstTNRNoiseModelAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_tnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_3DNR, (CVI_VOID *) &shared_buffer);
	*pstTNRNoiseModelAttr = &shared_buffer->stTNRNoiseModelAttr;

	return ret;
}

CVI_S32 isp_tnr_ctrl_set_tnr_noise_model_attr(VI_PIPE ViPipe, const ISP_TNR_NOISE_MODEL_ATTR_S *pstTNRNoiseModelAttr)
{
	if (pstTNRNoiseModelAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_tnr_ctrl_runtime *runtime = _get_tnr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_tnr_ctrl_check_tnr_noise_model_attr_valid(pstTNRNoiseModelAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_TNR_NOISE_MODEL_ATTR_S *p = CVI_NULL;

	isp_tnr_ctrl_get_tnr_noise_model_attr(ViPipe, &p);
	memcpy((CVI_VOID *)p, pstTNRNoiseModelAttr, sizeof(*pstTNRNoiseModelAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_tnr_ctrl_get_tnr_luma_motion_attr(VI_PIPE ViPipe, const ISP_TNR_LUMA_MOTION_ATTR_S **pstTNRLumaMotionAttr)
{
	if (pstTNRLumaMotionAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_tnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_3DNR, (CVI_VOID *) &shared_buffer);
	*pstTNRLumaMotionAttr = &shared_buffer->stTNRLumaMotionAttr;

	return ret;
}

CVI_S32 isp_tnr_ctrl_set_tnr_luma_motion_attr(VI_PIPE ViPipe, const ISP_TNR_LUMA_MOTION_ATTR_S *pstTNRLumaMotionAttr)
{
	if (pstTNRLumaMotionAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_tnr_ctrl_runtime *runtime = _get_tnr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_tnr_ctrl_check_tnr_luma_motion_attr_valid(pstTNRLumaMotionAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_TNR_LUMA_MOTION_ATTR_S *p = CVI_NULL;

	isp_tnr_ctrl_get_tnr_luma_motion_attr(ViPipe, &p);
	memcpy((CVI_VOID *)p, pstTNRLumaMotionAttr, sizeof(*pstTNRLumaMotionAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_tnr_ctrl_get_tnr_ghost_attr(VI_PIPE ViPipe, const ISP_TNR_GHOST_ATTR_S **pstTNRGhostAttr)
{
	if (pstTNRGhostAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_tnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_3DNR, (CVI_VOID *) &shared_buffer);
	*pstTNRGhostAttr = &shared_buffer->stTNRGhostAttr;

	return ret;
}

CVI_S32 isp_tnr_ctrl_set_tnr_ghost_attr(VI_PIPE ViPipe, const ISP_TNR_GHOST_ATTR_S *pstTNRGhostAttr)
{
	if (pstTNRGhostAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_tnr_ctrl_runtime *runtime = _get_tnr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_tnr_ctrl_check_tnr_ghost_attr_valid(pstTNRGhostAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_TNR_GHOST_ATTR_S *p = CVI_NULL;

	isp_tnr_ctrl_get_tnr_ghost_attr(ViPipe, &p);
	memcpy((CVI_VOID *)p, pstTNRGhostAttr, sizeof(*pstTNRGhostAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_tnr_ctrl_get_tnr_mt_prt_attr(VI_PIPE ViPipe, const ISP_TNR_MT_PRT_ATTR_S **pstTNRMtPrtAttr)
{
	if (pstTNRMtPrtAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_tnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_3DNR, (CVI_VOID *) &shared_buffer);
	*pstTNRMtPrtAttr = &shared_buffer->stTNRMtPrtAttr;

	return ret;
}

CVI_S32 isp_tnr_ctrl_set_tnr_mt_prt_attr(VI_PIPE ViPipe, const ISP_TNR_MT_PRT_ATTR_S *pstTNRMtPrtAttr)
{
	if (pstTNRMtPrtAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_tnr_ctrl_runtime *runtime = _get_tnr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_tnr_ctrl_check_tnr_mt_prt_attr_valid(pstTNRMtPrtAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_TNR_MT_PRT_ATTR_S *p = CVI_NULL;

	isp_tnr_ctrl_get_tnr_mt_prt_attr(ViPipe, &p);
	memcpy((CVI_VOID *)p, pstTNRMtPrtAttr, sizeof(*pstTNRMtPrtAttr));

	isp_tnr_ctrl_set_tnr_mt_prt_attr_compatible(ViPipe, (ISP_TNR_MT_PRT_ATTR_S *)p);

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_tnr_ctrl_get_tnr_motion_adapt_attr(VI_PIPE ViPipe,
			const ISP_TNR_MOTION_ADAPT_ATTR_S **pstTNRMotionAdaptAttr)
{
	if (pstTNRMotionAdaptAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_tnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_3DNR, (CVI_VOID *) &shared_buffer);
	*pstTNRMotionAdaptAttr = &shared_buffer->stTNRMotionAdaptAttr;

	return ret;
}

CVI_S32 isp_tnr_ctrl_set_tnr_motion_adapt_attr(VI_PIPE ViPipe, const ISP_TNR_MOTION_ADAPT_ATTR_S *pstTNRMotionAdaptAttr)
{
	if (pstTNRMotionAdaptAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_tnr_ctrl_runtime *runtime = _get_tnr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_tnr_ctrl_check_tnr_motion_adapt_attr_valid(pstTNRMotionAdaptAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_TNR_MOTION_ADAPT_ATTR_S *p = CVI_NULL;

	isp_tnr_ctrl_get_tnr_motion_adapt_attr(ViPipe, &p);
	memcpy((CVI_VOID *)p, pstTNRMotionAdaptAttr, sizeof(*pstTNRMotionAdaptAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

