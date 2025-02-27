/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_blc_ctrl.c
 * Description:
 *
 */

#include <math.h>
#include "cvi_comm_isp.h"

#include "isp_main_local.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "isp_interpolate.h"
#include "isp_blc_ctrl.h"
#include "isp_mgr_buf.h"

#include "isp_tun_buf_ctrl.h"

static struct isp_blc_ctrl_runtime  *_get_blc_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_blc_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_BLC, (CVI_VOID *) &shared_buffer);

	return &shared_buffer->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_blc_ctrl_check_param_valid(const ISP_BLACK_LEVEL_ATTR_S *pstBlackLevelAttr)
{
	CVI_S32 ret = CVI_SUCCESS;

	// CHECK_VALID_CONST(pstBlackLevelAttr, Enable, CVI_FALSE, CVI_TRUE);
	CHECK_VALID_CONST(pstBlackLevelAttr, enOpType, OP_TYPE_AUTO, OP_TYPE_MANUAL);
	// CHECK_VALID_CONST(pstBlackLevelAttr, UpdateInterval, 0, 0xff);
	CHECK_VALID_AUTO_ISO_1D(pstBlackLevelAttr, OffsetR, 0, 0xfff);
	CHECK_VALID_AUTO_ISO_1D(pstBlackLevelAttr, OffsetGr, 0, 0xfff);
	CHECK_VALID_AUTO_ISO_1D(pstBlackLevelAttr, OffsetGb, 0, 0xfff);
	CHECK_VALID_AUTO_ISO_1D(pstBlackLevelAttr, OffsetB, 0, 0xfff);
	CHECK_VALID_AUTO_ISO_1D(pstBlackLevelAttr, OffsetR2, 0, 0xfff);
	CHECK_VALID_AUTO_ISO_1D(pstBlackLevelAttr, OffsetGr2, 0, 0xfff);
	CHECK_VALID_AUTO_ISO_1D(pstBlackLevelAttr, OffsetGb2, 0, 0xfff);
	CHECK_VALID_AUTO_ISO_1D(pstBlackLevelAttr, OffsetB2, 0, 0xfff);

	return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_blc_ctrl_set_blc_attr(VI_PIPE ViPipe, const ISP_BLACK_LEVEL_ATTR_S *pstBlackLevelAttr)
{
	if (pstBlackLevelAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;
	struct isp_blc_ctrl_runtime *runtime = _get_blc_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	ret = isp_blc_ctrl_check_param_valid(pstBlackLevelAttr);
	if (ret != CVI_SUCCESS)
		return ret;

	const ISP_BLACK_LEVEL_ATTR_S *p = CVI_NULL;

	isp_blc_ctrl_get_blc_attr(ViPipe, &p);
	memcpy((void *)p, pstBlackLevelAttr, sizeof(*pstBlackLevelAttr));

	runtime->preprocess_updated = CVI_TRUE;

	return ret;
}

CVI_S32 isp_blc_ctrl_get_blc_attr(VI_PIPE ViPipe, const ISP_BLACK_LEVEL_ATTR_S **pstBlackLevelAttr)
{
	if (pstBlackLevelAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 ret = CVI_SUCCESS;

	struct isp_blc_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_BLC, (CVI_VOID *) &shared_buffer);

	*pstBlackLevelAttr = &shared_buffer->stBlackLevelAttr;

	return ret;
}

CVI_S32 isp_blc_ctrl_get_blc_info(VI_PIPE ViPipe, struct blc_info *info)
{
    CVI_S32 ret = CVI_SUCCESS;
    struct isp_blc_ctrl_runtime *runtime = _get_blc_ctrl_runtime(ViPipe);
 
    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }
 
    info->OffsetR = runtime->pre_fe_blc_attr[0].OffsetR;
    info->OffsetGr = runtime->pre_fe_blc_attr[0].OffsetGr;
    info->OffsetGb = runtime->pre_fe_blc_attr[0].OffsetGb;
    info->OffsetB = runtime->pre_fe_blc_attr[0].OffsetB;
    info->OffsetR2 = runtime->pre_fe_blc_attr[0].OffsetR2;
    info->OffsetGr2 = runtime->pre_fe_blc_attr[0].OffsetGr2;
    info->OffsetGb2 = runtime->pre_fe_blc_attr[0].OffsetGb2;
    info->OffsetB2 = runtime->pre_fe_blc_attr[0].OffsetB2;
    info->GainR = runtime->pre_fe_blc_attr[0].GainR;
    info->GainGr = runtime->pre_fe_blc_attr[0].GainGr;
    info->GainGb = runtime->pre_fe_blc_attr[0].GainGb;
    info->GainB = runtime->pre_fe_blc_attr[0].GainB;
 
    return ret;
}
