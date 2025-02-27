/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_dehaze_ctrl.c
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

#include "isp_dehaze_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_dehaze_ctrl_runtime  *_get_dehaze_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_dehaze_shared_buffer *shared_buf = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DEHAZE, (CVI_VOID *) &shared_buf);

	return &shared_buf->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_dehaze_ctrl_check_dehaze_attr_valid(const ISP_DEHAZE_ATTR_S *pstDehazeAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstDehazeAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstDehazeAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstDehazeAttr, UpdateInterval, 0, 0xff);
	CHECK_VALID_CONST(pstDehazeAttr, CumulativeThr, 0x0, 0x3fff);
	CHECK_VALID_CONST(pstDehazeAttr, MinTransMapValue, 0x0, 0x1fff);
	// CHECK_VALID_CONST(pstDehazeAttr, DehazeLumaEnable, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstDehazeAttr, DehazeSkinEnable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstDehazeAttr, AirLightMixWgt, 0x0, 0x20);
	CHECK_VALID_CONST(pstDehazeAttr, DehazeWgt, 0x0, 0x20);
	// CHECK_VALID_CONST(pstDehazeAttr, TransMapScale, 0x0, 0xff);
	CHECK_VALID_CONST(pstDehazeAttr, AirlightDiffWgt, 0x0, 0x10);
	CHECK_VALID_CONST(pstDehazeAttr, AirLightMax, 0x0, 0xfff);
	CHECK_VALID_CONST(pstDehazeAttr, AirLightMin, 0x0, 0xfff);
	// CHECK_VALID_CONST(pstDehazeAttr, SkinCb, 0x0, 0xff);
	// CHECK_VALID_CONST(pstDehazeAttr, SkinCr, 0x0, 0xff);
	CHECK_VALID_CONST(pstDehazeAttr, DehazeLumaCOEFFI, 0x0, 0x7d0);
	CHECK_VALID_CONST(pstDehazeAttr, DehazeSkinCOEFFI, 0x0, 0x7d0);
	CHECK_VALID_CONST(pstDehazeAttr, TransMapWgtWgt, 0x0, 0x80);
	// CHECK_VALID_CONST(pstDehazeAttr, TransMapWgtSigma, 0x0, 0xff);

	CHECK_VALID_AUTO_ISO_1D(pstDehazeAttr, Strength, 0, 0x64);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_dehaze_ctrl_get_dehaze_attr(VI_PIPE ViPipe, const ISP_DEHAZE_ATTR_S **pstDehazeAttr)
{
	if (pstDehazeAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_dehaze_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DEHAZE, (CVI_VOID *) &shared_buffer);
	*pstDehazeAttr = &shared_buffer->stDehazeAttr;

	return ret;
}

CVI_S32 isp_dehaze_ctrl_set_dehaze_attr(VI_PIPE ViPipe, const ISP_DEHAZE_ATTR_S *pstDehazeAttr)
{
	if (pstDehazeAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_dehaze_ctrl_runtime *runtime = _get_dehaze_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_dehaze_ctrl_check_dehaze_attr_valid(pstDehazeAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_DEHAZE_ATTR_S *p = CVI_NULL;

	isp_dehaze_ctrl_get_dehaze_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstDehazeAttr, sizeof(*pstDehazeAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

