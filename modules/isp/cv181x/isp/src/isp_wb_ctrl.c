/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_wb_ctrl.c
 * Description:
 *
 */
#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"
#include "isp_interpolate.h"
#include "isp_defines.h"
#include "isp_wb_ctrl.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_wb_ctrl_runtime  *_get_wb_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_wb_shared_buffer *shared_buf = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_WBGAIN, (CVI_VOID *) &shared_buf);

	return &shared_buf->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_wb_ctrl_check_wb_colortone_attr_valid(const ISP_COLOR_TONE_ATTR_S *pstColorToneAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	CHECK_VALID_CONST(pstColorToneAttr, u16RedCastGain, 0x0, 0x1000);
	CHECK_VALID_CONST(pstColorToneAttr, u16GreenCastGain, 0x0, 0x1000);
	CHECK_VALID_CONST(pstColorToneAttr, u16BlueCastGain, 0x0, 0x1000);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_wb_ctrl_get_wb_colortone_attr(VI_PIPE ViPipe, const ISP_COLOR_TONE_ATTR_S **pstColorToneAttr)
{
	if (pstColorToneAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_wb_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_WBGAIN, (CVI_VOID *) &shared_buffer);
	*pstColorToneAttr = &shared_buffer->stColorToneAttr;

	return ret;
}

CVI_S32 isp_wb_ctrl_set_wb_colortone_attr(VI_PIPE ViPipe, const ISP_COLOR_TONE_ATTR_S *pstColorToneAttr)
{
	if (pstColorToneAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_wb_ctrl_runtime *runtime = _get_wb_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_wb_ctrl_check_wb_colortone_attr_valid(pstColorToneAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_COLOR_TONE_ATTR_S *p = CVI_NULL;

	isp_wb_ctrl_get_wb_colortone_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstColorToneAttr, sizeof(*pstColorToneAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

