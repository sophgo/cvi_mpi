/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_demosaic_ctrl.c
 * Description:
 *
 */

#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"
#include "cvi_isp.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"

#include "isp_demosaic_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_demosaic_ctrl_runtime  *_get_demosaic_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_demosaic_shared_buffer *shared_buf = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DEMOSAIC, (CVI_VOID *) &shared_buf);

	return &shared_buf->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_demosaic_ctrl_check_demosaic_attr_valid(const ISP_DEMOSAIC_ATTR_S *pstDemosaicAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstDemosaicAttr, Enable, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstDemosaicAttr, TuningMode, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstDemosaicAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstDemosaicAttr, UpdateInterval, 0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstDemosaicAttr, CoarseEdgeThr, 0x0, 0xfff);
	CHECK_VALID_AUTO_ISO_1D(pstDemosaicAttr, CoarseStr, 0x0, 0xfff);
	CHECK_VALID_AUTO_ISO_1D(pstDemosaicAttr, FineEdgeThr, 0x0, 0xfff);
	CHECK_VALID_AUTO_ISO_1D(pstDemosaicAttr, FineStr, 0x0, 0xfff);
	CHECK_VALID_AUTO_ISO_1D(pstDemosaicAttr, FilterMode, 0x0, 0x1);

	return ret;
}

static CVI_S32 isp_demosaic_ctrl_check_demosaic_demoire_attr_valid(
	const ISP_DEMOSAIC_DEMOIRE_ATTR_S *pstDemosaicDemoireAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_AUTO_ISO_1D(pstDemosaicDemoireAttr, DetailSmoothEnable, 0x0, 0x1);
	// CHECK_VALID_AUTO_ISO_1D(pstDemosaicDemoireAttr, DetailSmoothStr, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstDemosaicDemoireAttr, EdgeWgtStr, 0x0, 0xff);

	UNUSED(pstDemosaicDemoireAttr);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_demosaic_ctrl_get_demosaic_attr(VI_PIPE ViPipe,
	const ISP_DEMOSAIC_ATTR_S **pstDemosaicAttr)
{
	if (pstDemosaicAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_demosaic_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DEMOSAIC, (CVI_VOID *) &shared_buffer);
	*pstDemosaicAttr = &shared_buffer->stDemosaicAttr;

	return ret;
}

CVI_S32 isp_demosaic_ctrl_set_demosaic_attr(VI_PIPE ViPipe,
	const ISP_DEMOSAIC_ATTR_S *pstDemosaicAttr)
{
	if (pstDemosaicAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_demosaic_ctrl_runtime *runtime = _get_demosaic_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_demosaic_ctrl_check_demosaic_attr_valid(pstDemosaicAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_DEMOSAIC_ATTR_S *p = CVI_NULL;

	isp_demosaic_ctrl_get_demosaic_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstDemosaicAttr, sizeof(*pstDemosaicAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

CVI_S32 isp_demosaic_ctrl_get_demosaic_demoire_attr(VI_PIPE ViPipe,
	const ISP_DEMOSAIC_DEMOIRE_ATTR_S **pstDemosaicDemoireAttr)
{
	if (pstDemosaicDemoireAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_demosaic_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_DEMOSAIC, (CVI_VOID *) &shared_buffer);
	*pstDemosaicDemoireAttr = &shared_buffer->stDemoireAttr;

	return ret;
}

CVI_S32 isp_demosaic_ctrl_set_demosaic_demoire_attr(VI_PIPE ViPipe,
	const ISP_DEMOSAIC_DEMOIRE_ATTR_S *pstDemosaicDemoireAttr)
{
	if (pstDemosaicDemoireAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_demosaic_ctrl_runtime *runtime = _get_demosaic_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_demosaic_ctrl_check_demosaic_demoire_attr_valid(pstDemosaicDemoireAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_DEMOSAIC_DEMOIRE_ATTR_S *p = CVI_NULL;

	isp_demosaic_ctrl_get_demosaic_demoire_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstDemosaicDemoireAttr, sizeof(*pstDemosaicDemoireAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}


