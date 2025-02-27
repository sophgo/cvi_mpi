/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_dis_ctrl.c
 * Description:
 *
 */

#include "stdio.h"
#include "stdlib.h"

#include "cvi_sys_base.h"
#include "cvi_isp.h"
#include "cvi_comm_isp.h"

#include "isp_debug.h"
#include "isp_defines.h"
#include "isp_control.h"
#include "isp_sts_ctrl.h"
#include "isp_dis_ctrl.h"
#include "isp_mgr_buf.h"
//#include "vi_ioctl.h"

#ifdef DIS_PC_PLATFORM
#include "dis_platform.h"
#endif

#define NEW_DIS_CTRL_ARCH


#ifndef NEW_DIS_CTRL_ARCH

CVI_S32 isp_dis_get_gms_attr(VI_PIPE ViPipe, struct cvi_vip_isp_gms_config *gmsCfg)
{
	CVI_S32 ret = CVI_SUCCESS;

	ret = isp_dis_ctrl_get_gms_attr(ViPipe, gmsCfg);
	return ret;
}
#endif

static struct isp_dis_ctrl_runtime *_get_dis_ctrl_runtime(VI_PIPE ViPipe)
{
	CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

	if (!isVipipeValid) {
		ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
		return NULL;
	}

	struct isp_dis_shared_buffer *shared_buf = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_GMS, (CVI_VOID *) &shared_buf);

	return &shared_buf->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_dis_ctrl_set_dis_attr(VI_PIPE ViPipe, const ISP_DIS_ATTR_S *pstDisAttr)
{
	ISP_LOG_INFO("+\n");
	CVI_S32 ret = CVI_SUCCESS;
#ifdef NEW_DIS_CTRL_ARCH
	struct isp_dis_ctrl_runtime *runtime = _get_dis_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	if (pstDisAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	const ISP_DIS_ATTR_S *p = CVI_NULL;

	isp_dis_ctrl_get_dis_attr(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstDisAttr, sizeof(*pstDisAttr));

	runtime->preprocess_updated = CVI_TRUE;
#else
	UNUSED(ViPipe);
	UNUSED(pstDisAttr);

#endif
	return ret;
}

CVI_S32 isp_dis_ctrl_get_dis_attr(VI_PIPE ViPipe, const ISP_DIS_ATTR_S **pstDisAttr)
{
	CVI_S32 ret = CVI_SUCCESS;
#ifdef NEW_DIS_CTRL_ARCH

	if (pstDisAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	struct isp_dis_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_GMS, (CVI_VOID *) &shared_buffer);
	*pstDisAttr = &shared_buffer->stDisAttr;
#else
	UNUSED(ViPipe);
	UNUSED(pstDisAttr);
#endif
	return ret;
}

CVI_S32 isp_dis_ctrl_set_dis_config(VI_PIPE ViPipe, const ISP_DIS_CONFIG_S *pstDisConfig)
{
	ISP_LOG_INFO("+\n");
	CVI_S32 ret = CVI_SUCCESS;
#ifdef NEW_DIS_CTRL_ARCH
	struct isp_dis_ctrl_runtime *runtime = _get_dis_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}

	if (pstDisConfig == CVI_NULL) {
		return CVI_FAILURE;
	}

	const ISP_DIS_CONFIG_S *p = CVI_NULL;

	isp_dis_ctrl_get_dis_config(ViPipe, &p);
	memcpy((CVI_VOID *) p, pstDisConfig, sizeof(*pstDisConfig));

	runtime->preprocess_updated = CVI_TRUE;
#else
	UNUSED(ViPipe);
	UNUSED(pstDisConfig);
#endif
	return ret;
}

CVI_S32 isp_dis_ctrl_get_dis_config(VI_PIPE ViPipe, const ISP_DIS_CONFIG_S **pstDisConfig)
{
	CVI_S32 ret = CVI_SUCCESS;
#ifdef NEW_DIS_CTRL_ARCH

	if (pstDisConfig == CVI_NULL) {
		return CVI_FAILURE;
	}

	struct isp_dis_shared_buffer *shared_buffer = CVI_NULL;

	isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_GMS, (CVI_VOID *) &shared_buffer);
	*pstDisConfig = &shared_buffer->stDisConfig;
#else
	UNUSED(ViPipe);
	UNUSED(pstDisConfig);
#endif
	return ret;
}

CVI_S32 isp_dis_ctrl_get_gms_attr(VI_PIPE ViPipe, struct cvi_vip_isp_gms_config *gmsCfg)
{
#define MAX_GAP (255)
#define MIN_GAP (10)
	//gap >= 10 && gap <=255 (overflow)
	//(x_section_size +1)*3 + x_gap*2 + offset_x + 4 < imgW // 4 cycle by RTL limit
	//(y_section_size +1)*3 + y_gap*2 + offset_y  < imgH

	CVI_U32 imgW, imgH;
	CVI_S32 ret = CVI_SUCCESS;
	CVI_U32 curSize, gap, limitW, limitH;

	if (ViPipe > (DIS_SENSOR_NUM - 1))
		return CVI_SUCCESS;

	struct isp_dis_ctrl_runtime *runtime = _get_dis_ctrl_runtime(ViPipe);

	if (runtime == CVI_NULL) {
		return CVI_FAILURE;
	}


	imgW = runtime->pstPubAttr.stSnsSize.u32Width;
	imgH = runtime->pstPubAttr.stSnsSize.u32Height;

	limitW = imgW - 4;//4 cycle by RTL limit
	limitH = imgH - 0;

	gmsCfg->offset_x = 0;
	gmsCfg->x_gap = MIN_GAP;
	gmsCfg->x_section_size = XHIST_LENGTH - 1;

	curSize = gmsCfg->offset_x + (gmsCfg->x_section_size + 1) * DIS_MAX_WINDOW_X_NUM + gmsCfg->x_gap * 2;
	//curSize = 770;
	if (limitW < curSize) {
		gmsCfg->x_section_size = (limitW - gmsCfg->x_gap * 2) / DIS_MAX_WINDOW_X_NUM - 1;
	} else {
		curSize = (gmsCfg->x_section_size + 1) * DIS_MAX_WINDOW_X_NUM;
		gap = (limitW - curSize) / (DIS_MAX_WINDOW_X_NUM + 1);
		if (gap < MIN_GAP) {
			gmsCfg->offset_x = gap;
			gmsCfg->x_gap = MIN_GAP;
		} else if (gap <= MAX_GAP) {
			gmsCfg->offset_x = gap;
			gmsCfg->x_gap = gap;
		} else {//limitW =1794 ,gap =256
			gap = MAX_GAP;
			gmsCfg->x_gap = gap;
			curSize = gmsCfg->offset_x + (gmsCfg->x_section_size + 1) * DIS_MAX_WINDOW_X_NUM
				+ gmsCfg->x_gap * 2;
			//1278
			gmsCfg->offset_x = (imgW - curSize) / 2;
		}
	}

	gmsCfg->offset_y = 0;
	gmsCfg->y_gap = MIN_GAP;
	gmsCfg->y_section_size = YHIST_LENGTH - 1;

	curSize = gmsCfg->offset_y + (gmsCfg->y_section_size + 1) * DIS_MAX_WINDOW_Y_NUM + gmsCfg->y_gap * 2;
	//curSize = 770;
	if (limitH < curSize) {
		gmsCfg->y_section_size = (limitH - gmsCfg->y_gap * 2) / DIS_MAX_WINDOW_Y_NUM - 1;
	} else {
		curSize = (gmsCfg->y_section_size + 1) * DIS_MAX_WINDOW_Y_NUM;
		gap = (limitH - curSize) / (DIS_MAX_WINDOW_Y_NUM + 1);
		if (gap < MIN_GAP) {
			gmsCfg->offset_y = gap;
			gmsCfg->y_gap = MIN_GAP;
		} else if (gap <= MAX_GAP) {
			gmsCfg->offset_y = gap;
			gmsCfg->y_gap = gap;
		} else {//limitH =1794 ,gap =256
			gap = MAX_GAP;
			gmsCfg->y_gap = gap;
			curSize = gmsCfg->offset_y + (gmsCfg->y_section_size + 1) * DIS_MAX_WINDOW_Y_NUM
				+ gmsCfg->y_gap * 2;
			//1278
			gmsCfg->offset_y = (imgH - curSize) / 2;
		}
	}

	return ret;
}

