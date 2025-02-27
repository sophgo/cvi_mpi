/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_mono_ctrl.c
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

#include "isp_mono_ctrl.h"
#include "isp_mgr_buf.h"


static struct isp_mono_ctrl_runtime  *_get_mono_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_mono_shared_buffer *shared_buf = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_MONO, (CVI_VOID *) &shared_buf);

	return &shared_buf->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_mono_ctrl_check_mono_attr_valid(const ISP_MONO_ATTR_S *pstMonoAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	UNUSED(pstMonoAttr);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_mono_ctrl_get_mono_attr(VI_PIPE ViPipe, const ISP_MONO_ATTR_S **pstMonoAttr)
{
	if (pstMonoAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_mono_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_MONO, (CVI_VOID *) &shared_buffer);
	*pstMonoAttr = &shared_buffer->stMonoAttr;

	return ret;
}

CVI_S32 isp_mono_ctrl_set_mono_attr(VI_PIPE ViPipe, const ISP_MONO_ATTR_S *pstMonoAttr)
{
	if (pstMonoAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_mono_ctrl_runtime *runtime = _get_mono_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_mono_ctrl_check_mono_attr_valid(pstMonoAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_MONO_ATTR_S *p = CVI_NULL;

	isp_mono_ctrl_get_mono_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstMonoAttr, sizeof(*pstMonoAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

