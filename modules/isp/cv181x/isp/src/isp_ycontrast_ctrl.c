/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_ycontrast_ctrl.c
 * Description:
 *
 */

#include "cvi_type.h"

#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"

#include "isp_ycontrast_ctrl.h"

#include "isp_iir_api.h"
#include "isp_mgr_buf.h"

static struct isp_ycontrast_ctrl_runtime *_get_ycontrast_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_ycur_shared_buffer *shared_buf = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_YCONTRAST, (CVI_VOID *) &shared_buf);

	return &shared_buf->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_ycontrast_ctrl_check_ycontrast_attr_valid(const ISP_YCONTRAST_ATTR_S *pstYContrastAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstYContrastAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstYContrastAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstYContrastAttr, UpdateInterval, 0, 0xff);
	CHECK_VALID_AUTO_LV_1D(pstYContrastAttr, ContrastLow, 0x0, 0x64);
	CHECK_VALID_AUTO_LV_1D(pstYContrastAttr, ContrastHigh, 0x0, 0x64);
	CHECK_VALID_AUTO_LV_1D(pstYContrastAttr, CenterLuma, 0x0, 0x40);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_ycontrast_ctrl_get_ycontrast_attr(VI_PIPE ViPipe, const ISP_YCONTRAST_ATTR_S **pstYContrastAttr)
{
	if (pstYContrastAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ycur_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_YCONTRAST, (CVI_VOID *) &shared_buffer);
	*pstYContrastAttr = &shared_buffer->stYContrastAttr;

	return ret;
}

CVI_S32 isp_ycontrast_ctrl_set_ycontrast_attr(VI_PIPE ViPipe, const ISP_YCONTRAST_ATTR_S *pstYContrastAttr)
{
	if (pstYContrastAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ycontrast_ctrl_runtime *runtime = _get_ycontrast_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_ycontrast_ctrl_check_ycontrast_attr_valid(pstYContrastAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_YCONTRAST_ATTR_S *p = CVI_NULL;

	isp_ycontrast_ctrl_get_ycontrast_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstYContrastAttr, sizeof(*pstYContrastAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

