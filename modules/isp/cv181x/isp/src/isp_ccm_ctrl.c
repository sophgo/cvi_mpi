/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_ccm_ctrl.c
 * Description:
 *
 */

#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "cvi_comm_isp.h"

#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"
#include "isp_interpolate.h"

#include "isp_ccm_ctrl.h"
#include "isp_mgr_buf.h"

static struct isp_ccm_ctrl_runtime  *_get_ccm_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_ccm_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CCM, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_ccm_ctrl_check_ccm_attr_valid(const ISP_CCM_ATTR_S *pstCCMAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstCCMAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstCCMAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstCCMAttr, UpdateInterval, 0, 0xff);

	// Manual
	const ISP_CCM_MANUAL_ATTR_S *ptCCMManual = &(pstCCMAttr->stManual);

	// CHECK_VALID_CONST(ptCCMManual, SatEnable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_ARRAY_1D(ptCCMManual, CCM, 9, -8192, 8191);

	// Auto
	const ISP_CCM_AUTO_ATTR_S *ptCCMAuto = &(pstCCMAttr->stAuto);

	// CHECK_VALID_CONST(ptCCMAuto, ISOActEnable, CVI_FALSE, CVI_TRUE);
	// CHECK_VALID_CONST(ptCCMAuto, TempActEnable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(ptCCMAuto, CCMTabNum, 0x3, 0x7);

	for (CVI_U32 tempIdx = 0; tempIdx < ptCCMAuto->CCMTabNum; ++tempIdx) {
		const ISP_COLORMATRIX_ATTR_S *pstColorMatrix = &(ptCCMAuto->CCMTab[tempIdx]);

		CHECK_VALID_CONST(pstColorMatrix, ColorTemp, 0x1f4, 0x7530);
		CHECK_VALID_ARRAY_1D(pstColorMatrix, CCM, 9, -8192, 8191);
	}

	return ret;
}

static CVI_S32 isp_ccm_ctrl_check_saturation_attr_valid(const ISP_SATURATION_ATTR_S *pstSaturationAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	CHECK_VALID_CONST(pstSaturationAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);

	return ret;
}

static CVI_S32 isp_ccm_ctrl_check_ccm_saturation_attr_valid(const ISP_CCM_SATURATION_ATTR_S *pstCCMSaturationAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_AUTO_ISO_1D(pstCCMSaturationAttr, SaturationLE, 0x0, 0xff);
	// CHECK_VALID_AUTO_ISO_1D(pstCCMSaturationAttr, SaturationSE, 0x0, 0xff);

	UNUSED(pstCCMSaturationAttr);

	return ret;
}


//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_ccm_ctrl_get_ccm_attr(VI_PIPE ViPipe, const ISP_CCM_ATTR_S **pstCCMAttr)
{
	if (pstCCMAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_ccm_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CCM, (CVI_VOID *) &shared_buffer);
	*pstCCMAttr = &shared_buffer->stCCMAttr;

	// CVI_ISP_PrintCCMAttr(pstCCMAttr);

	return ret;
}

CVI_S32 isp_ccm_ctrl_set_ccm_attr(VI_PIPE ViPipe, const ISP_CCM_ATTR_S *pstCCMAttr)
{
	if (pstCCMAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ccm_ctrl_runtime *runtime = _get_ccm_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_ccm_ctrl_check_ccm_attr_valid(pstCCMAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_CCM_ATTR_S *p = CVI_NULL;

	isp_ccm_ctrl_get_ccm_attr(ViPipe, &p);
	memcpy((void *)p, pstCCMAttr, sizeof(*pstCCMAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

CVI_S32 isp_ccm_ctrl_get_ccm_saturation_attr(VI_PIPE ViPipe, const ISP_CCM_SATURATION_ATTR_S **pstCCMSaturationAttr)
{
	if (pstCCMSaturationAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_ccm_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CCM, (CVI_VOID *) &shared_buffer);
	*pstCCMSaturationAttr = &shared_buffer->stCCMSaturationAttr;

	return ret;
}

CVI_S32 isp_ccm_ctrl_set_ccm_saturation_attr(VI_PIPE ViPipe, const ISP_CCM_SATURATION_ATTR_S *pstCCMSaturationAttr)
{
	if (pstCCMSaturationAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ccm_ctrl_runtime *runtime = _get_ccm_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_ccm_ctrl_check_ccm_saturation_attr_valid(pstCCMSaturationAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_CCM_SATURATION_ATTR_S *p = CVI_NULL;

	isp_ccm_ctrl_get_ccm_saturation_attr(ViPipe, &p);
	memcpy((void *)p, pstCCMSaturationAttr, sizeof(*pstCCMSaturationAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

CVI_S32 isp_ccm_ctrl_get_saturation_attr(VI_PIPE ViPipe, const ISP_SATURATION_ATTR_S **pstSaturationAttr)
{
	if (pstSaturationAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_ccm_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CCM, (CVI_VOID *) &shared_buffer);
	*pstSaturationAttr = &shared_buffer->stSaturationAttr;

	return ret;
}

CVI_S32 isp_ccm_ctrl_set_saturation_attr(VI_PIPE ViPipe, const ISP_SATURATION_ATTR_S *pstSaturationAttr)
{
	if (pstSaturationAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ccm_ctrl_runtime *runtime = _get_ccm_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_ccm_ctrl_check_saturation_attr_valid(pstSaturationAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_SATURATION_ATTR_S *p = CVI_NULL;

	isp_ccm_ctrl_get_saturation_attr(ViPipe, &p);
	memcpy((void *)p, pstSaturationAttr, sizeof(*pstSaturationAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return CVI_SUCCESS;
}

CVI_S32 isp_ccm_ctrl_get_ccm_info(VI_PIPE ViPipe, struct ccm_info *info)
{
	CVI_S32 ret = CVI_SUCCESS;
	struct isp_ccm_ctrl_runtime *runtime = _get_ccm_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	const ISP_CCM_ATTR_S *ccm_attr = NULL;

	isp_ccm_ctrl_get_ccm_attr(ViPipe, &ccm_attr);
	info->ccm_en = ccm_attr->Enable;
	for (CVI_U32 i = 0 ; i < 9 ; i++) {
		//info->CCM[i] = ISP_PRT_CAST_U16(runtime->ccm_param_out.CCM[0])[i];
		info->CCM[i] = runtime->ccm_attr[ISP_CHANNEL_LE].stManual.CCM[i];
	}

	return ret;
}

