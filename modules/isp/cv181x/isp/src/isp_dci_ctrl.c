/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_dci_ctrl.c
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
#include "isp_dci_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_dci_ctrl_runtime *_get_dci_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_dci_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DCI, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_dci_ctrl_check_dci_attr_valid(const ISP_DCI_ATTR_S *pstDCIAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstDCIAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstDCIAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstDCIAttr, UpdateInterval, 0, 0xff);
	// CHECK_VALID_CONST(pstDCIAttr, TuningMode, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstDCIAttr, Method, 0x0, 0x1);
	CHECK_VALID_CONST(pstDCIAttr, Speed, 0x0, 0x1f4);
	CHECK_VALID_CONST(pstDCIAttr, DciStrength, 0x0, 0x100);
	CHECK_VALID_CONST(pstDCIAttr, DciGamma, 0x64, 0x320);
	// CHECK_VALID_CONST(pstDCIAttr, DciOffset, 0x0, 0xff);
	// CHECK_VALID_CONST(pstDCIAttr, ToleranceY, 0x0, 0xff);
	// CHECK_VALID_CONST(pstDCIAttr, Sensitivity, 0x0, 0xff);

	CHECK_VALID_AUTO_ISO_1D(pstDCIAttr, ContrastGain, 0, 0x100);
	// CHECK_VALID_AUTO_ISO_1D(pstDCIAttr, BlcThr, 0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstDCIAttr, WhtThr, 0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstDCIAttr, BlcCtrl, 0, 0x200);
	CHECK_VALID_AUTO_ISO_1D(pstDCIAttr, WhtCtrl, 0, 0x200);
	CHECK_VALID_AUTO_ISO_1D(pstDCIAttr, DciGainMax, 0, 0x100);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_dci_ctrl_get_dci_attr(VI_PIPE ViPipe, const ISP_DCI_ATTR_S **pstDCIAttr)
{
	if (pstDCIAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_dci_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DCI, (CVI_VOID *) &shared_buffer);

	*pstDCIAttr = &shared_buffer->stDciAttr;

	return ret;
}

CVI_S32 isp_dci_ctrl_set_dci_attr(VI_PIPE ViPipe, const ISP_DCI_ATTR_S *pstDCIAttr)
{
	if (pstDCIAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_dci_ctrl_runtime *runtime = _get_dci_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_dci_ctrl_check_dci_attr_valid(pstDCIAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_DCI_ATTR_S *p = CVI_NULL;

	isp_dci_ctrl_get_dci_attr(ViPipe, &p);

	memcpy((CVI_VOID *) p, pstDCIAttr, sizeof(*pstDCIAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}


