/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_crosstalk_ctrl.c
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

#include "isp_crosstalk_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_crosstalk_ctrl_runtime  *_get_crosstalk_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_crosstalk_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CROSSTALK, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_crosstalk_ctrl_check_crosstalk_attr_valid(const ISP_CROSSTALK_ATTR_S *pstCrosstalkAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstCrosstalkAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstCrosstalkAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstCrosstalkAttr, UpdateInterval, 0, 0xff);
	CHECK_VALID_ARRAY_1D(pstCrosstalkAttr, GrGbDiffThreSec, 4, 0x0, 0xfff);
	CHECK_VALID_ARRAY_1D(pstCrosstalkAttr, FlatThre, 4, 0x0, 0xfff);
	CHECK_VALID_AUTO_ISO_1D(pstCrosstalkAttr, Strength, 0x0, 0x100);

	return ret;
}
//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_crosstalk_ctrl_get_crosstalk_attr(VI_PIPE ViPipe, const ISP_CROSSTALK_ATTR_S **pstCrosstalkAttr)
{
	if (pstCrosstalkAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_crosstalk_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CROSSTALK, (CVI_VOID *) &shared_buffer);
	*pstCrosstalkAttr = &shared_buffer->stCrosstalkAttr;

	return ret;
}

CVI_S32 isp_crosstalk_ctrl_set_crosstalk_attr(VI_PIPE ViPipe, const ISP_CROSSTALK_ATTR_S *pstCrosstalkAttr)
{
	if (pstCrosstalkAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_crosstalk_ctrl_runtime *runtime = _get_crosstalk_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_crosstalk_ctrl_check_crosstalk_attr_valid(pstCrosstalkAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_CROSSTALK_ATTR_S *p = CVI_NULL;

	isp_crosstalk_ctrl_get_crosstalk_attr(ViPipe, &p);
	memcpy((void *)p, pstCrosstalkAttr, sizeof(*pstCrosstalkAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

