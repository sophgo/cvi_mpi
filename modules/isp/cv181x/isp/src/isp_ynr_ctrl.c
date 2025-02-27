/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_ynr_ctrl.c
 * Description:
 *
 */

#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"
#include "isp_ioctl.h"
#include "cvi_isp.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"

#include "isp_ynr_ctrl.h"
#include "isp_mgr_buf.h"
#include "isp_ccm_ctrl.h"
#include "isp_gamma_ctrl.h"

static struct isp_ynr_ctrl_runtime  *_get_ynr_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_ynr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_YNR, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_BOOL is_value_in_array(CVI_S32 value, CVI_S32 *array, CVI_U32 length)
{
	CVI_U32 i;

	for (i = 0; i < length; i++)
		if (array[i] == value)
			break;

	return i != length;
}

static CVI_S32 isp_ynr_ctrl_check_ynr_attr_valid(const ISP_YNR_ATTR_S *pstYNRAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstYNRAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstYNRAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstYNRAttr, UpdateInterval, 0, 0xff);
	// CHECK_VALID_CONST(pstYNRAttr, CoringParamEnable, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstYNRAttr, FiltModeEnable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstYNRAttr, FiltMode, 0x0, 0x100);

	CVI_S32 TuningModeList[] = {8, 11, 12, 13, 14, 15};

	if (!is_value_in_array(pstYNRAttr->TuningMode, TuningModeList, ARRAY_SIZE(TuningModeList))) {
		ISP_LOG_WARNING("tuning moode only accept values in 8, 11, 12, 13, 14, 15\n");
		ret = CVI_FAILURE_ILLEGAL_PARAM;
	}

	CHECK_VALID_AUTO_ISO_1D(pstYNRAttr, WindowType, 0x0, 0xb);
	CHECK_VALID_AUTO_ISO_1D(pstYNRAttr, DetailSmoothMode, 0x0, 0x1);
	// CHECK_VALID_AUTO_ISO_1D(pstYNRAttr, NoiseSuppressStr, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstYNRAttr, FilterType, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstYNRAttr, NoiseCoringMax, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstYNRAttr, NoiseCoringBase, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstYNRAttr, NoiseCoringAdv, 0x0, 0xff);

	return ret;
}

static CVI_S32 isp_ynr_ctrl_check_ynr_filter_attr_valid(const ISP_YNR_FILTER_ATTR_S *pstYNRFilterAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_AUTO_ISO_1D(pstYNRFilterAttr, VarThr, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstYNRFilterAttr, CoringWgtLF, 0x0, 0x100);
	CHECK_VALID_AUTO_ISO_1D(pstYNRFilterAttr, CoringWgtHF, 0x0, 0x100);
	CHECK_VALID_AUTO_ISO_1D(pstYNRFilterAttr, NonDirFiltStr, 0x0, 0x1f);
	CHECK_VALID_AUTO_ISO_1D(pstYNRFilterAttr, VhDirFiltStr, 0x0, 0x1f);
	// CHECK_VALID_AUTO_ISO_1D(pstYNRFilterAttr, AaDirFiltStr, 0x0, 0x1f);
	// CHECK_VALID_AUTO_ISO_1D(pstYNRFilterAttr, CoringWgtMax, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstYNRFilterAttr, FilterMode, 0x0, 0x3ff);

	return ret;
}

static CVI_S32 isp_ynr_ctrl_check_ynr_motion_attr_valid(const ISP_YNR_MOTION_NR_ATTR_S *pstYNRMotionNRAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_AUTO_ISO_1D(pstYNRMotionNRAttr, MotionCoringWgtMax, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_2D(pstYNRMotionNRAttr, MotionYnrLut, 16, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_2D(pstYNRMotionNRAttr, MotionCoringWgt, 16, 0x0, 0x100);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_ynr_ctrl_get_ynr_attr(VI_PIPE ViPipe, const ISP_YNR_ATTR_S **pstYNRAttr)
{
	if (pstYNRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_ynr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_YNR, (CVI_VOID *) &shared_buffer);
	*pstYNRAttr = &shared_buffer->stYNRAttr;

	return ret;
}

CVI_S32 isp_ynr_ctrl_set_ynr_attr(VI_PIPE ViPipe, const ISP_YNR_ATTR_S *pstYNRAttr)
{
	if (pstYNRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ynr_ctrl_runtime *runtime = _get_ynr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_ynr_ctrl_check_ynr_attr_valid(pstYNRAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_YNR_ATTR_S *p = CVI_NULL;

	isp_ynr_ctrl_get_ynr_attr(ViPipe, &p);
	memcpy((void *)p, pstYNRAttr, sizeof(*pstYNRAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_ynr_ctrl_get_ynr_filter_attr(VI_PIPE ViPipe, const ISP_YNR_FILTER_ATTR_S **pstYNRFilterAttr)
{
	if (pstYNRFilterAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_ynr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_YNR, (CVI_VOID *) &shared_buffer);
	*pstYNRFilterAttr = &shared_buffer->stYNRFilterAttr;

	return ret;
}

CVI_S32 isp_ynr_ctrl_set_ynr_filter_attr(VI_PIPE ViPipe, const ISP_YNR_FILTER_ATTR_S *pstYNRFilterAttr)
{
	if (pstYNRFilterAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ynr_ctrl_runtime *runtime = _get_ynr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_ynr_ctrl_check_ynr_filter_attr_valid(pstYNRFilterAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_YNR_FILTER_ATTR_S *p = CVI_NULL;

	isp_ynr_ctrl_get_ynr_filter_attr(ViPipe, &p);
	memcpy((void *)p, pstYNRFilterAttr, sizeof(*pstYNRFilterAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

CVI_S32 isp_ynr_ctrl_get_ynr_motion_attr(VI_PIPE ViPipe, const ISP_YNR_MOTION_NR_ATTR_S **pstYNRMotionNRAttr)
{
	if (pstYNRMotionNRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_ynr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_YNR, (CVI_VOID *) &shared_buffer);
	*pstYNRMotionNRAttr = &shared_buffer->stYNRMotionNRAttr;

	return ret;
}

CVI_S32 isp_ynr_ctrl_set_ynr_motion_attr(VI_PIPE ViPipe, const ISP_YNR_MOTION_NR_ATTR_S *pstYNRMotionNRAttr)
{
	if (pstYNRMotionNRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ynr_ctrl_runtime *runtime = _get_ynr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_ynr_ctrl_check_ynr_motion_attr_valid(pstYNRMotionNRAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_YNR_MOTION_NR_ATTR_S *p = CVI_NULL;

	isp_ynr_ctrl_get_ynr_motion_attr(ViPipe, &p);
	memcpy((void *)p, pstYNRMotionNRAttr, sizeof(*pstYNRMotionNRAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

