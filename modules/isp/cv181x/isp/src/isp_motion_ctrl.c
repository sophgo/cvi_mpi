/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_motion_ctrl.c
 * Description:
 *
 */

#include "cvi_sys.h"
#include "cvi_sys_base.h"

#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"

#include "isp_sts_ctrl.h"

#include "isp_motion_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_motion_ctrl_runtime *_get_motion_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_motion_shared_buffer *shared_buf = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_MOTION, (CVI_VOID *) &shared_buf);

	return &shared_buf->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_motion_ctrl_check_motion_attr_valid(const ISP_VC_ATTR_S *pstMotionAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	CHECK_VALID_CONST(pstMotionAttr, MotionThreshold, 0x0, 0xff);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_motion_ctrl_get_motion_attr(VI_PIPE ViPipe, const ISP_VC_ATTR_S **pstMotionAttr)
{
	if (pstMotionAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_motion_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_MOTION, (CVI_VOID *) &shared_buffer);
	*pstMotionAttr = &shared_buffer->stMotionAttr;

	return ret;
}

CVI_S32 isp_motion_ctrl_set_motion_attr(VI_PIPE ViPipe, const ISP_VC_ATTR_S *pstMotionAttr)
{
	if (pstMotionAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_motion_ctrl_runtime *runtime = _get_motion_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_motion_ctrl_check_motion_attr_valid(pstMotionAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_VC_ATTR_S *p = CVI_NULL;

	isp_motion_ctrl_get_motion_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstMotionAttr, sizeof(*pstMotionAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

