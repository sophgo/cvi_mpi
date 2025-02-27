/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_mlsc_ctrl.c
 * Description:
 *
 */

#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"
#include "isp_ioctl.h"
#include "isp_mw_compat.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"

#include "isp_mlsc_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_mlsc_ctrl_runtime  *_get_mlsc_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_mlsc_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_MLSC, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_mlsc_ctrl_check_mlsc_attr_valid(const ISP_MESH_SHADING_ATTR_S *pstMeshShadingAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstMeshShadingAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstMeshShadingAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstMeshShadingAttr, UpdateInterval, 0, 0xff);
	// CHECK_VALID_CONST(pstMeshShadingAttr, OverflowProtection, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_AUTO_ISO_1D(pstMeshShadingAttr, MeshStr, 0x0, 0xfff);

	return ret;
}

static CVI_S32 isp_mlsc_ctrl_check_mlsc_lut_attr_valid(
	const ISP_MESH_SHADING_GAIN_LUT_ATTR_S * pstMeshShadingGainLutAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	CHECK_VALID_CONST(pstMeshShadingGainLutAttr, Size, 0x1, 0x7);

	for (CVI_U32 tempIdx = 0; tempIdx < pstMeshShadingGainLutAttr->Size; ++tempIdx) {
		const ISP_MESH_SHADING_GAIN_LUT_S *pstMeshShadingGainLut
			= &(pstMeshShadingGainLutAttr->LscGainLut[tempIdx]);

		CHECK_VALID_CONST(pstMeshShadingGainLut, ColorTemperature, 0x0, 0x7530);
		CHECK_VALID_ARRAY_1D(pstMeshShadingGainLut, RGain, CVI_ISP_LSC_GRID_POINTS, 0x0, 0xfff);
		CHECK_VALID_ARRAY_1D(pstMeshShadingGainLut, GGain, CVI_ISP_LSC_GRID_POINTS, 0x0, 0xfff);
		CHECK_VALID_ARRAY_1D(pstMeshShadingGainLut, BGain, CVI_ISP_LSC_GRID_POINTS, 0x0, 0xfff);
	}

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_mlsc_ctrl_get_mlsc_attr(VI_PIPE ViPipe, const ISP_MESH_SHADING_ATTR_S **pstMeshShadingAttr)
{
	if (pstMeshShadingAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_mlsc_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_MLSC, (CVI_VOID *) &shared_buffer);

	*pstMeshShadingAttr = &shared_buffer->mlsc;

	return ret;
}

CVI_S32 isp_mlsc_ctrl_set_mlsc_attr(VI_PIPE ViPipe, const ISP_MESH_SHADING_ATTR_S *pstMeshShadingAttr)
{
	if (pstMeshShadingAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_mlsc_ctrl_runtime *runtime = _get_mlsc_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_mlsc_ctrl_check_mlsc_attr_valid(pstMeshShadingAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_MESH_SHADING_ATTR_S *p = CVI_NULL;

	isp_mlsc_ctrl_get_mlsc_attr(ViPipe, &p);

	memcpy((CVI_VOID *) p, pstMeshShadingAttr, sizeof(*pstMeshShadingAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_mlsc_ctrl_get_mlsc_lut_attr(VI_PIPE ViPipe,
			const ISP_MESH_SHADING_GAIN_LUT_ATTR_S **pstMeshShadingGainLutAttr)
{
	if (pstMeshShadingGainLutAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_mlsc_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_MLSC, (CVI_VOID *) &shared_buffer);

	*pstMeshShadingGainLutAttr = &shared_buffer->mlscLUT;

	return ret;
}

CVI_S32 isp_mlsc_ctrl_set_mlsc_lut_attr(VI_PIPE ViPipe
	, const ISP_MESH_SHADING_GAIN_LUT_ATTR_S *pstMeshShadingGainLutAttr)
{
	if (pstMeshShadingGainLutAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_mlsc_ctrl_runtime *runtime = _get_mlsc_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_mlsc_ctrl_check_mlsc_lut_attr_valid(pstMeshShadingGainLutAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_MESH_SHADING_GAIN_LUT_ATTR_S *p = CVI_NULL;

	isp_mlsc_ctrl_get_mlsc_lut_attr(ViPipe, &p);

	memcpy((CVI_VOID *) p, pstMeshShadingGainLutAttr, sizeof(*pstMeshShadingGainLutAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

CVI_S32 isp_mlsc_ctrl_get_mlsc_info(VI_PIPE ViPipe, struct mlsc_info *info)
{
	CVI_S32 ret = CVI_SUCCESS;
	struct isp_mlsc_ctrl_runtime *runtime = _get_mlsc_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	const ISP_MESH_SHADING_ATTR_S *mlsc_attr = NULL;

	isp_mlsc_ctrl_get_mlsc_attr(ViPipe, &mlsc_attr);

	if (mlsc_attr->Enable && !runtime->is_module_bypass) {
		info->lut_r = runtime->mlsc_lut_attr.RGain;
		info->lut_g = runtime->mlsc_lut_attr.GGain;
		info->lut_b = runtime->mlsc_lut_attr.BGain;
		info->mlsc_compensate_gain = runtime->mlsc_param_out.mlsc_compensate_gain;
	} else {
		info->lut_r = runtime->unit_gain_table;
		info->lut_g = runtime->unit_gain_table;
		info->lut_b = runtime->unit_gain_table;
		info->mlsc_compensate_gain = 1.0;
	}

	UNUSED(ViPipe);

	return ret;
}
