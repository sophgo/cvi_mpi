/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_ldci_ctrl.c
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

#include "isp_ldci_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_ldci_ctrl_runtime *_get_ldci_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isViPipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isViPipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_ldci_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_LDCI, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_ldci_ctrl_check_ldci_attr_valid(const ISP_LDCI_ATTR_S *pstLDCIAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstLDCIAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstLDCIAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstLDCIAttr, UpdateInterval, 0, 0xFF);
	// CHECK_VALID_CONST(pstLDCIAttr, GaussLPFSigma, 0, 0xFF);

	CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, LdciStrength, 0, 0x100);
	CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, LdciRange, 0, 0x3FF);
	CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, TprCoef, 0, 0x3FF);
	// CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, EdgeCoring, 0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, LumaWgtMax, 0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, LumaWgtMin, 0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, VarMapMax, 0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, VarMapMin, 0, 0xFF);
	CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, UvGainMax, 0, 0x7F);
	CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, UvGainMin, 0, 0x7F);
	// CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, BrightContrastHigh, 0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, BrightContrastLow, 0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, DarkContrastHigh, 0, 0xFF);
	// CHECK_VALID_AUTO_ISO_1D(pstLDCIAttr, DarkContrastLow, 0, 0xFF);

	CHECK_VALID_CONST(pstLDCIAttr, stManual.LumaPosWgt.Wgt, 0, 0x80);
	CHECK_VALID_CONST(pstLDCIAttr, stManual.LumaPosWgt.Sigma, 0x1, 0xFF);
	CHECK_VALID_CONST(pstLDCIAttr, stManual.LumaPosWgt.Mean, 0x1, 0xFF);
	for (CVI_U32 u32IsoIdx = 0; u32IsoIdx < ISP_AUTO_ISO_STRENGTH_NUM; ++u32IsoIdx) {
		CHECK_VALID_CONST(pstLDCIAttr, stAuto.LumaPosWgt[u32IsoIdx].Wgt, 0x1, 0x80);
		CHECK_VALID_CONST(pstLDCIAttr, stAuto.LumaPosWgt[u32IsoIdx].Sigma, 0x1, 0xFF);
		CHECK_VALID_CONST(pstLDCIAttr, stAuto.LumaPosWgt[u32IsoIdx].Mean, 0x1, 0xFF);
	}

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_ldci_ctrl_get_ldci_attr(VI_PIPE ViPipe, const ISP_LDCI_ATTR_S **pstLDCIAttr)
{
	if (pstLDCIAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ldci_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_LDCI, (CVI_VOID *) &shared_buffer);

	*pstLDCIAttr = &shared_buffer->stLdciAttr;

	return ret;
}

CVI_S32 isp_ldci_ctrl_set_ldci_attr(VI_PIPE ViPipe, const ISP_LDCI_ATTR_S *pstLDCIAttr)
{
	if (pstLDCIAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ldci_ctrl_runtime *runtime = _get_ldci_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_ldci_ctrl_check_ldci_attr_valid(pstLDCIAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_LDCI_ATTR_S *p = CVI_NULL;

	isp_ldci_ctrl_get_ldci_attr(ViPipe, &p);

	memcpy((CVI_VOID *) p, pstLDCIAttr, sizeof(ISP_LDCI_ATTR_S));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

