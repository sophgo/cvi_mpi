/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_clut_ctrl.c
 * Description:
 *
 */

#include "cvi_sys.h"

#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"
#include "isp_ioctl.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"

#include "isp_clut_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_clut_ctrl_runtime  *_get_clut_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_clut_shared_buffer *shared_buf = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CLUT, (CVI_VOID *) &shared_buf);

	return &shared_buf->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_clut_ctrl_check_clut_attr_valid(const ISP_CLUT_ATTR_S *pstCLUTAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstCLUTAttr, Enable, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(pstCLUTAttr, UpdateInterval, 0, 0xff);
	CHECK_VALID_ARRAY_1D(pstCLUTAttr, ClutR, ISP_CLUT_LUT_LENGTH, 0x0, 0x3ff);
	CHECK_VALID_ARRAY_1D(pstCLUTAttr, ClutG, ISP_CLUT_LUT_LENGTH, 0x0, 0x3ff);
	CHECK_VALID_ARRAY_1D(pstCLUTAttr, ClutB, ISP_CLUT_LUT_LENGTH, 0x0, 0x3ff);

	return ret;
}

static CVI_S32 isp_clut_ctrl_check_clut_saturation_attr_valid(const ISP_CLUT_SATURATION_ATTR_S *pstClutSaturationAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstClutSaturationAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_AUTO_ISO_2D(pstClutSaturationAttr, SatIn, 4, 0x0, 0x2000);
	CHECK_VALID_AUTO_ISO_2D(pstClutSaturationAttr, SatOut, 4, 0x0, 0x2000);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_clut_ctrl_get_clut_attr(VI_PIPE ViPipe, const ISP_CLUT_ATTR_S **pstCLUTAttr)
{
	if (pstCLUTAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_clut_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CLUT, (CVI_VOID *) &shared_buffer);
	*pstCLUTAttr = &shared_buffer->stCLUTAttr;

	return ret;
}

CVI_S32 isp_clut_ctrl_set_clut_attr(VI_PIPE ViPipe, const ISP_CLUT_ATTR_S *pstCLUTAttr)
{
	if (pstCLUTAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_clut_ctrl_runtime *runtime = _get_clut_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_clut_ctrl_check_clut_attr_valid(pstCLUTAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_CLUT_ATTR_S *p = CVI_NULL;

	isp_clut_ctrl_get_clut_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstCLUTAttr, sizeof(*pstCLUTAttr));

	runtime->preprocess_table_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

CVI_S32 isp_clut_ctrl_get_clut_saturation_attr(VI_PIPE ViPipe,
	const ISP_CLUT_SATURATION_ATTR_S **pstClutSaturationAttr)
{
	if (pstClutSaturationAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_clut_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CLUT, (CVI_VOID *) &shared_buffer);
	*pstClutSaturationAttr = &shared_buffer->stClutSaturationAttr;

	return ret;
}

CVI_S32 isp_clut_ctrl_set_clut_saturation_attr(VI_PIPE ViPipe,
	const ISP_CLUT_SATURATION_ATTR_S *pstClutSaturationAttr)
{
	if (pstClutSaturationAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_clut_ctrl_runtime *runtime = _get_clut_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_clut_ctrl_check_clut_saturation_attr_valid(pstClutSaturationAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_CLUT_SATURATION_ATTR_S *p = CVI_NULL;

	isp_clut_ctrl_get_clut_saturation_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstClutSaturationAttr, sizeof(*pstClutSaturationAttr));

	runtime->preprocess_lut_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

