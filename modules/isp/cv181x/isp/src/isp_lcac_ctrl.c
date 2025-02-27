/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 *
 * File Name: isp_lcac_ctrl.c
 * Description:
 *
 */

#include <math.h>

#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"
#include "isp_ioctl.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"
#include "isp_sts_ctrl.h"

#include "isp_lcac_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_lcac_ctrl_runtime *_get_lcac_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isViPipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isViPipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_lcac_shared_buffer *shared_buf = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_LCAC, (CVI_VOID *) &shared_buf);

	return &shared_buf->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_lcac_ctrl_check_lcac_attr_valid(const ISP_LCAC_ATTR_S *pstLCACAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstLCACAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstLCACAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstLCACAttr, UpdateInterval, 1, 0xFF);
	CHECK_VALID_CONST(pstLCACAttr, TuningMode, 0, 0x6);
	CHECK_VALID_CONST(pstLCACAttr, DePurpleCrStr0, 0, 0x40);
	CHECK_VALID_CONST(pstLCACAttr, DePurpleCbStr0, 0, 0x40);
	CHECK_VALID_CONST(pstLCACAttr, DePurpleCrStr1, 0, 0x40);
	CHECK_VALID_CONST(pstLCACAttr, DePurpleCbStr1, 0, 0x40);
	CHECK_VALID_CONST(pstLCACAttr, FilterTypeBase, 0, 0x3);
	CHECK_VALID_CONST(pstLCACAttr, EdgeGainBase0, 0, 0x1C);
	CHECK_VALID_CONST(pstLCACAttr, EdgeGainBase1, 0, 0x23);
	CHECK_VALID_CONST(pstLCACAttr, EdgeStrWgtBase, 0, 0x10);
	CHECK_VALID_CONST(pstLCACAttr, DePurpleStrMaxBase, 0, 0x80);
	CHECK_VALID_CONST(pstLCACAttr, DePurpleStrMinBase, 0, 0x80);
	CHECK_VALID_CONST(pstLCACAttr, FilterScaleAdv, 0, 0xF);
	CHECK_VALID_CONST(pstLCACAttr, LumaWgt, 0, 0x40);
	CHECK_VALID_CONST(pstLCACAttr, FilterTypeAdv, 0, 0x5);
	CHECK_VALID_CONST(pstLCACAttr, EdgeGainAdv0, 0, 0x1C);
	CHECK_VALID_CONST(pstLCACAttr, EdgeGainAdv1, 0, 0x23);
	CHECK_VALID_CONST(pstLCACAttr, EdgeStrWgtAdvG, 0, 0x10);
	// CHECK_VALID_CONST(pstLCACAttr, DePurpleStrMaxAdv, 0, 0xFF);
	// CHECK_VALID_CONST(pstLCACAttr, DePurpleStrMinAdv, 0, 0xFF);
	CHECK_VALID_CONST(pstLCACAttr, EdgeWgtBase.Wgt, 0, 0x80);
	CHECK_VALID_CONST(pstLCACAttr, EdgeWgtBase.Sigma, 0x1, 0xFF);
	CHECK_VALID_CONST(pstLCACAttr, EdgeWgtAdv.Wgt, 0, 0x80);
	CHECK_VALID_CONST(pstLCACAttr, EdgeWgtAdv.Sigma, 0x1, 0xFF);
	CHECK_VALID_AUTO_ISO_1D(pstLCACAttr, DePurpleCrGain, 0, 0xFFF);
	CHECK_VALID_AUTO_ISO_1D(pstLCACAttr, DePurpleCbGain, 0, 0xFFF);
	CHECK_VALID_AUTO_ISO_1D(pstLCACAttr, DePurepleCrWgt0, 0, 0x40);
	CHECK_VALID_AUTO_ISO_1D(pstLCACAttr, DePurepleCbWgt0, 0, 0x40);
	CHECK_VALID_AUTO_ISO_1D(pstLCACAttr, DePurepleCrWgt1, 0, 0x40);
	CHECK_VALID_AUTO_ISO_1D(pstLCACAttr, DePurepleCbWgt1, 0, 0x40);
	// CHECK_VALID_AUTO_ISO_1D(pstLCACAttr, EdgeCoringBase, 0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstLCACAttr, EdgeCoringAdv, 0, 0xFF);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_lcac_ctrl_get_lcac_attr(VI_PIPE ViPipe, const ISP_LCAC_ATTR_S **pstLCACAttr)
{
	ISP_LOG_INFO("+\n");
	if (pstLCACAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_lcac_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_LCAC, (CVI_VOID *) &shared_buffer);
	*pstLCACAttr = &shared_buffer->stLCACAttr;

	return ret;
}

CVI_S32 isp_lcac_ctrl_set_lcac_attr(VI_PIPE ViPipe, const ISP_LCAC_ATTR_S *pstLCACAttr)
{
	ISP_LOG_INFO("+\n");
	if (pstLCACAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_lcac_ctrl_runtime *runtime = _get_lcac_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_lcac_ctrl_check_lcac_attr_valid(pstLCACAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_LCAC_ATTR_S *p = CVI_NULL;

	isp_lcac_ctrl_get_lcac_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstLCACAttr, sizeof(ISP_LCAC_ATTR_S));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

