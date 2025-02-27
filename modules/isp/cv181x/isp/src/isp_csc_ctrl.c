/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_csc_ctrl.c
 * Description:
 *
 */

#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"

#include "isp_csc_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_csc_ctrl_runtime  *_get_csc_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_csc_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CSC, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_csc_ctrl_check_csc_attr_valid(const ISP_CSC_ATTR_S *pstCSCAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstCSCAttr, Enable, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstCSCAttr, UpdateInterval, 0, 0xff);
	CHECK_VALID_CONST(pstCSCAttr, enColorGamut, 0x0, ISP_CSC_COLORGAMUT_NUM);
	CHECK_VALID_CONST(pstCSCAttr, Hue, 0x0, 0x64);
	CHECK_VALID_CONST(pstCSCAttr, Luma, 0x0, 0x64);
	CHECK_VALID_CONST(pstCSCAttr, Contrast, 0x0, 0x64);
	CHECK_VALID_CONST(pstCSCAttr, Saturation, 0x0, 0x64);
	CHECK_VALID_ARRAY_1D(pstCSCAttr, stUserMatrx.userCscCoef, CSC_MATRIX_SIZE, -0x2000, 0x1FFF);
	CHECK_VALID_ARRAY_1D(pstCSCAttr, stUserMatrx.userCscOffset, CSC_OFFSET_SIZE, -0x100, 0xFF);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_csc_ctrl_get_csc_attr(VI_PIPE ViPipe, const ISP_CSC_ATTR_S **pstCSCAttr)
{
	if (pstCSCAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_csc_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CSC, (CVI_VOID *) &shared_buffer);
	*pstCSCAttr = &shared_buffer->stCSCAttr;

	return ret;
}

CVI_S32 isp_csc_ctrl_set_csc_attr(VI_PIPE ViPipe, const ISP_CSC_ATTR_S *pstCSCAttr)
{
	if (pstCSCAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_csc_ctrl_runtime *runtime = _get_csc_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_csc_ctrl_check_csc_attr_valid(pstCSCAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_CSC_ATTR_S *p = CVI_NULL;

	isp_csc_ctrl_get_csc_attr(ViPipe, &p);
	memcpy((void *)p, pstCSCAttr, sizeof(*pstCSCAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

