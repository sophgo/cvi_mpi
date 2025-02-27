/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_cnr_ctrl.c
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

#include "isp_cnr_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_cnr_ctrl_runtime  *_get_cnr_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_cnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CNR, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_cnr_ctrl_check_cnr_attr_valid(const ISP_CNR_ATTR_S *pstCNRAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstCNRAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstCNRAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstCNRAttr, UpdateInterval, 0, 0xff);

	// CHECK_VALID_AUTO_ISO_1D(pstCNRAttr, CnrStr, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstCNRAttr, NoiseSuppressStr, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstCNRAttr, NoiseSuppressGain, 0x1, 0x8);
	CHECK_VALID_AUTO_ISO_1D(pstCNRAttr, FilterType, 0x0, 0x1f);
	// CHECK_VALID_AUTO_ISO_1D(pstCNRAttr, MotionNrStr, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstCNRAttr, LumaWgt, 0x0, 0x8);
	CHECK_VALID_AUTO_ISO_1D(pstCNRAttr, DetailSmoothMode, 0x0, 0x1);

	return ret;
}

static CVI_S32 isp_cnr_ctrl_check_cnr_motion_attr_valid(const ISP_CNR_MOTION_NR_ATTR_S *pstCNRMotionNRAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstCNRMotionNRAttr, MotionCnrEnable, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_AUTO_ISO_2D(pstCNRMotionNRAttr, MotionCnrCoringLut, CNR_MOTION_LUT_NUM, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_2D(pstCNRMotionNRAttr, MotionCnrStrLut, CNR_MOTION_LUT_NUM, 0x0, 0xff);

	UNUSED(pstCNRMotionNRAttr);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_cnr_ctrl_get_cnr_attr(VI_PIPE ViPipe, const ISP_CNR_ATTR_S **pstCNRAttr)
{
	if (pstCNRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_cnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CNR, (CVI_VOID *) &shared_buffer);
	*pstCNRAttr = &shared_buffer->stCNRAttr;

	return ret;
}

CVI_S32 isp_cnr_ctrl_set_cnr_attr(VI_PIPE ViPipe, const ISP_CNR_ATTR_S *pstCNRAttr)
{
	if (pstCNRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_cnr_ctrl_runtime *runtime = _get_cnr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_cnr_ctrl_check_cnr_attr_valid(pstCNRAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_CNR_ATTR_S *p = CVI_NULL;

	isp_cnr_ctrl_get_cnr_attr(ViPipe, &p);
	memcpy((void *)p, pstCNRAttr, sizeof(*pstCNRAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_cnr_ctrl_get_cnr_motion_attr(VI_PIPE ViPipe, const ISP_CNR_MOTION_NR_ATTR_S **pstCNRMotionNRAttr)
{
	if (pstCNRMotionNRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_cnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CNR, (CVI_VOID *) &shared_buffer);
	*pstCNRMotionNRAttr = &shared_buffer->stCNRMotionNRAttr;

	return ret;
}

CVI_S32 isp_cnr_ctrl_set_cnr_motion_attr(VI_PIPE ViPipe, const ISP_CNR_MOTION_NR_ATTR_S *pstCNRMotionNRAttr)
{
	if (pstCNRMotionNRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_cnr_ctrl_runtime *runtime = _get_cnr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_cnr_ctrl_check_cnr_motion_attr_valid(pstCNRMotionNRAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_CNR_MOTION_NR_ATTR_S *p = CVI_NULL;

	isp_cnr_ctrl_get_cnr_motion_attr(ViPipe, &p);
	memcpy((void *)p, pstCNRMotionNRAttr, sizeof(*pstCNRMotionNRAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

