/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_dpc_ctrl.c
 * Description:
 *
 */

#include "cvi_sys.h"
#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"
#include "isp_ioctl.h"
#include "cvi_vi.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"
#include "cvi_isp.h"
#include "dpcm_api.h"
#include <sys/time.h>

#include "isp_dpc_ctrl.h"

#include "isp_mgr_buf.h"

static struct isp_dpc_ctrl_runtime  *_get_dpc_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_dpc_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DPC, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_dpc_ctrl_check_dpc_dynamic_attr_valid(const ISP_DP_DYNAMIC_ATTR_S *pstDPCDynamicAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstDPCDynamicAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstDPCDynamicAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstDPCDynamicAttr, UpdateInterval, 0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstDPCDynamicAttr, ClusterSize, 0x0, 0x3);

	return ret;
}

static CVI_S32 isp_dpc_ctrl_check_dpc_static_attr_valid(const ISP_DP_STATIC_ATTR_S *pstDPStaticAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstDPStaticAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstDPStaticAttr, BrightCount, 0x0, 0xFFF);
	CHECK_VALID_CONST(pstDPStaticAttr, DarkCount, 0x0, 0xFFF);
	CHECK_VALID_ARRAY_1D(pstDPStaticAttr, BrightTable, STATIC_DP_COUNT_MAX, 0x0, 0x1fff1fff);
	CHECK_VALID_ARRAY_1D(pstDPStaticAttr, DarkTable, STATIC_DP_COUNT_MAX, 0x0, 0x1fff1fff);

	return ret;
}

static CVI_S32 isp_dpc_ctrl_check_dpc_calibrate_valid(const ISP_DP_CALIB_ATTR_S *pstDPCalibAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstDPCalibAttr, EnableDetect, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstDPCalibAttr, StaticDPType, ISP_STATIC_DP_BRIGHT, ISP_STATIC_DP_DARK);
	// CHECK_VALID_CONST(pstDPCalibAttr, StartThresh, 0x0, 0xff);
	CHECK_VALID_CONST(pstDPCalibAttr, CountMax, 0x0, 0xfff);
	CHECK_VALID_CONST(pstDPCalibAttr, CountMin, 0x0, pstDPCalibAttr->CountMax);
	CHECK_VALID_CONST(pstDPCalibAttr, TimeLimit, 0x0, 0x640);
	// CHECK_VALID_CONST(pstDPCalibAttr, saveFileEn, CVI_FALSE, CVI_TRUE);

	// read only
	// CHECK_VALID_ARRAY_1D(pstDPCalibAttr, Table, STATIC_DP_COUNT_MAX, 0x0, 0x1fff1fff);
	// CHECK_VALID_CONST(pstDPCalibAttr, FinishThresh, 0x0, 0xff);
	// CHECK_VALID_CONST(pstDPCalibAttr, Count, 0x0, 0xfff);
	// CHECK_VALID_CONST(pstDPCalibAttr, Status, 0x0, 0x2);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_dpc_ctrl_get_dpc_dynamic_attr(VI_PIPE ViPipe, const ISP_DP_DYNAMIC_ATTR_S **pstDPCDynamicAttr)
{
	if (pstDPCDynamicAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_dpc_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DPC, (CVI_VOID *) &shared_buffer);

	*pstDPCDynamicAttr = &shared_buffer->stDPCDynamicAttr;

	return ret;
}

CVI_S32 isp_dpc_ctrl_set_dpc_dynamic_attr(VI_PIPE ViPipe, const ISP_DP_DYNAMIC_ATTR_S *pstDPCDynamicAttr)
{
	if (pstDPCDynamicAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_dpc_ctrl_runtime *runtime = _get_dpc_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_dpc_ctrl_check_dpc_dynamic_attr_valid(pstDPCDynamicAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_DP_DYNAMIC_ATTR_S *p = CVI_NULL;

	isp_dpc_ctrl_get_dpc_dynamic_attr(ViPipe, &p);

	memcpy((CVI_VOID *) p, pstDPCDynamicAttr, sizeof(*pstDPCDynamicAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_dpc_ctrl_get_dpc_static_attr(VI_PIPE ViPipe, const ISP_DP_STATIC_ATTR_S **pstDPStaticAttr)
{
	if (pstDPStaticAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_dpc_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DPC, (CVI_VOID *) &shared_buffer);

	*pstDPStaticAttr = &shared_buffer->stDPStaticAttr;

	return ret;
}

CVI_S32 isp_dpc_ctrl_set_dpc_static_attr(VI_PIPE ViPipe, const ISP_DP_STATIC_ATTR_S *pstDPStaticAttr)
{
	if (pstDPStaticAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_dpc_ctrl_runtime *runtime = _get_dpc_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_dpc_ctrl_check_dpc_static_attr_valid(pstDPStaticAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_DP_STATIC_ATTR_S *p = CVI_NULL;

	isp_dpc_ctrl_get_dpc_static_attr(ViPipe, &p);

	memcpy((CVI_VOID *) p, pstDPStaticAttr, sizeof(*pstDPStaticAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

CVI_S32 isp_dpc_ctrl_get_dpc_calibrate(VI_PIPE ViPipe, const ISP_DP_CALIB_ATTR_S **pstDPCalibAttr)
{
	if (pstDPCalibAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_dpc_ctrl_runtime *runtime = _get_dpc_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	struct isp_dpc_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DPC, (CVI_VOID *) &shared_buffer);

	*pstDPCalibAttr = &shared_buffer->stDPCalibAttr;

	return ret;
}

CVI_S32 isp_dpc_ctrl_set_dpc_calibrate(VI_PIPE ViPipe, const ISP_DP_CALIB_ATTR_S *pstDPCalibAttr)
{
	if (pstDPCalibAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_dpc_ctrl_runtime *runtime = _get_dpc_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_dpc_ctrl_check_dpc_calibrate_valid(pstDPCalibAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	if (pstDPCalibAttr->EnableDetect == CVI_FALSE)
		return ret;

	const ISP_DP_CALIB_ATTR_S *p = CVI_NULL;

	isp_dpc_ctrl_get_dpc_calibrate(ViPipe, &p);

	memcpy((CVI_VOID *) p, pstDPCalibAttr, sizeof(*pstDPCalibAttr));

	return CVI_SUCCESS;
}
