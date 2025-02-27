/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_bnr_ctrl.c
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

#include "isp_bnr_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_bnr_ctrl_runtime  *_get_bnr_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_bnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_BNR, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_bnr_ctrl_check_bnr_attr_valid(const ISP_NR_ATTR_S *pstNRAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstNRAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstNRAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstNRAttr, UpdateInterval, 0, 0xff);
	// CHECK_VALID_CONST(pstNRAttr, CoringParamEnable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_AUTO_ISO_1D(pstNRAttr, WindowType, 0x0, 0xb);
	CHECK_VALID_AUTO_ISO_1D(pstNRAttr, DetailSmoothMode, 0x0, 0x1);
	// CHECK_VALID_AUTO_ISO_1D(pstNRAttr, NoiseSuppressStr, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstNRAttr, FilterType, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstNRAttr, NoiseSuppressStrMode, 0x0, 0xff);

	return ret;
}

static CVI_S32 isp_bnr_ctrl_check_bnr_filter_attr_valid(const ISP_NR_FILTER_ATTR_S *pstNRFilterAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	CHECK_VALID_CONST(pstNRFilterAttr, TuningMode, 0x0, 0xf);
	CHECK_VALID_AUTO_ISO_2D(pstNRFilterAttr, LumaStr, 8, 0x0, 0x1f);
	// CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, VarThr, 0x0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, CoringWgtLF, 0x0, 0x100);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, CoringWgtHF, 0x0, 0x100);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NonDirFiltStr, 0x0, 0x1f);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, VhDirFiltStr, 0x0, 0x1f);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, AaDirFiltStr, 0x0, 0x1f);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpSlopeR, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpSlopeGr, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpSlopeGb, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpSlopeB, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpLumaThrR, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpLumaThrGr, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpLumaThrGb, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpLumaThrB, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpLowOffsetR, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpLowOffsetGr, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpLowOffsetGb, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpLowOffsetB, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpHighOffsetR, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpHighOffsetGr, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpHighOffsetGb, 0x0, 0x3ff);
	CHECK_VALID_AUTO_ISO_1D(pstNRFilterAttr, NpHighOffsetB, 0x0, 0x3ff);

	return ret;
}


//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_bnr_ctrl_get_bnr_attr(VI_PIPE ViPipe, const ISP_NR_ATTR_S **pstNRAttr)
{
	if (pstNRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_bnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_BNR, (CVI_VOID *) &shared_buffer);

	*pstNRAttr = &shared_buffer->stNRAttr;

	return ret;
}

CVI_S32 isp_bnr_ctrl_set_bnr_attr(VI_PIPE ViPipe, const ISP_NR_ATTR_S *pstNRAttr)
{
	if (pstNRAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_bnr_ctrl_runtime *runtime = _get_bnr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_bnr_ctrl_check_bnr_attr_valid(pstNRAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_NR_ATTR_S *p = CVI_NULL;

	isp_bnr_ctrl_get_bnr_attr(ViPipe, &p);
	memcpy((CVI_VOID *)p, pstNRAttr, sizeof(*pstNRAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_bnr_ctrl_get_bnr_filter_attr(VI_PIPE ViPipe, const ISP_NR_FILTER_ATTR_S **pstNRFilterAttr)
{
	if (pstNRFilterAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_bnr_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_BNR, (CVI_VOID *) &shared_buffer);

	*pstNRFilterAttr = &shared_buffer->stNRFilterAttr;

	return ret;
}

CVI_S32 isp_bnr_ctrl_set_bnr_filter_attr(VI_PIPE ViPipe, const ISP_NR_FILTER_ATTR_S *pstNRFilterAttr)
{
	if (pstNRFilterAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_bnr_ctrl_runtime *runtime = _get_bnr_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_bnr_ctrl_check_bnr_filter_attr_valid(pstNRFilterAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_NR_FILTER_ATTR_S *p = CVI_NULL;

	isp_bnr_ctrl_get_bnr_filter_attr(ViPipe, &p);
	memcpy((CVI_VOID *)p, pstNRFilterAttr, sizeof(*pstNRFilterAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}


