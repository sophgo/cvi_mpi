/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_ca2_ctrl.c
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
#include "isp_sts_ctrl.h"

#include "isp_ca2_ctrl.h"
#include "isp_mgr_buf.h"

#include "isp_ccm_ctrl.h"

static struct isp_ca2_ctrl_runtime  *_get_ca2_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_ca2_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CA2, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_ca2_ctrl_check_ca2_attr_valid(const ISP_CA2_ATTR_S *pstCA2Attr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstCA2Attr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstCA2Attr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstCA2Attr, UpdateInterval, 0, 0xff);
	CHECK_VALID_AUTO_ISO_2D(pstCA2Attr, Ca2In, CA_LITE_NODE, 0, 0xC0);
	CHECK_VALID_AUTO_ISO_2D(pstCA2Attr, Ca2Out, CA_LITE_NODE, 0, 0x7FF);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_ca2_ctrl_get_ca2_attr(VI_PIPE ViPipe, const ISP_CA2_ATTR_S **pstCA2Attr)
{
	if (pstCA2Attr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_ca2_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CA2, (CVI_VOID *) &shared_buffer);
	*pstCA2Attr = &shared_buffer->stCA2Attr;

	return ret;
}

CVI_S32 isp_ca2_ctrl_set_ca2_attr(VI_PIPE ViPipe, const ISP_CA2_ATTR_S *pstCA2Attr)
{
	if (pstCA2Attr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ca2_ctrl_runtime *runtime = _get_ca2_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_ca2_ctrl_check_ca2_attr_valid(pstCA2Attr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_CA2_ATTR_S *p = CVI_NULL;

	isp_ca2_ctrl_get_ca2_attr(ViPipe, &p);
	memcpy((void *)p, pstCA2Attr, sizeof(*pstCA2Attr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

