/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_sensor.c
 * Description:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "cvi_comm_sns.h"
#include "isp_main_local.h"
#include "isp_ioctl.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "isp_sts_ctrl.h"
#include "../../../algo/ae/aealgo.h"

// #include <linux/vi_uapi.h>
// #include <linux/vi_isp.h>
// #include <linux/vi_snsr.h>

typedef struct _ISP_SNS_CTX_S {
	ISP_CMOS_DEFAULT_S stCmosDft;
	ISP_CMOS_BLACK_LEVEL_S stSnsBlackLevel;
	ISP_SNS_SYNC_INFO_S stSnsSyncCfg;
	ISP_SNS_ATTR_INFO_S snsAttr;
	ISP_SENSOR_REGISTER_S snsRegFunc;
} ISP_SNS_CTX_S;

ISP_SNS_CTX_S gSnsCtx[VI_MAX_PIPE_NUM];

#ifdef ARCH_RTOS_CV181X
#define RTOS_I2C_DELAY_MAX 4
#define RTOS_I2C_DELAY_FRM 0
CVI_U32 i2c_widx;
CVI_U32 i2c_ridx;
CVI_U32 i2c_preridx;
ISP_SNS_SYNC_INFO_S gSnsCfg[VI_MAX_PIPE_NUM][RTOS_I2C_DELAY_MAX];
#endif

#define SNS_CTX_GET(pipeIdx, pSnsCtx) (pSnsCtx = &(gSnsCtx[pipeIdx]))

static CVI_S32 isp_sensor_updDefault(VI_PIPE ViPipe);

CVI_S32 isp_sensor_get_crop_info(CVI_S32 ViPipe, ISP_SNS_SYNC_INFO_S *snsCropInfo)
{
	ISP_SNS_CTX_S *pSnsInfo;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	SNS_CTX_GET(ViPipe, pSnsInfo);

	memcpy(snsCropInfo, &(pSnsInfo->stSnsSyncCfg), sizeof(ISP_SNS_SYNC_INFO_S));

	return 0;
}

CVI_S32 isp_sensor_register(CVI_S32 ViPipe, ISP_SNS_ATTR_INFO_S *pstSnsAttrInfo, ISP_SENSOR_REGISTER_S *pstRegister)
{
	ISP_SNS_CTX_S *pSnsInfo;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	SNS_CTX_GET(ViPipe, pSnsInfo);
	if (pstSnsAttrInfo == CVI_NULL) {
		return CVI_FAILURE;
	}

	if (pstRegister == CVI_NULL) {
		return CVI_FAILURE;
	}

	memcpy(&(pSnsInfo->snsAttr), pstSnsAttrInfo, sizeof(ISP_SNS_ATTR_INFO_S));
	memcpy(&(pSnsInfo->snsRegFunc), pstRegister, sizeof(ISP_SENSOR_REGISTER_S));

	if (pstRegister->stSnsExp.pfn_cmos_sensor_global_init != CVI_NULL)
		pstRegister->stSnsExp.pfn_cmos_sensor_global_init(ViPipe);

	isp_sensor_updDefault(ViPipe);
	return CVI_SUCCESS;
}

CVI_S32 isp_sensor_unRegister(CVI_S32 ViPipe)
{
	ISP_SNS_CTX_S *pSnsInfo;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	SNS_CTX_GET(ViPipe, pSnsInfo);

	memset(&(pSnsInfo->snsAttr), 0, sizeof(ISP_SNS_ATTR_INFO_S));
	memset(&(pSnsInfo->snsRegFunc), 0, sizeof(ISP_SENSOR_REGISTER_S));
	return 0;
}

CVI_S32 isp_sensor_init(CVI_S32 ViPipe)
{
	ISP_SNS_CTX_S *pSnsInfo;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	SNS_CTX_GET(ViPipe, pSnsInfo);

	if (pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_sensor_init != CVI_NULL)
		pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_sensor_init(ViPipe);

	return 0;
}

static CVI_S32 isp_sensor_updDefault(VI_PIPE ViPipe)
{
	ISP_SNS_CTX_S *pSnsInfo = CVI_NULL;
	ISP_CTX_S *pstIspCtx = CVI_NULL;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	ISP_GET_CTX(ViPipe, pstIspCtx);
	SNS_CTX_GET(ViPipe, pSnsInfo);
	if (pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_get_isp_default != CVI_NULL) {
		pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_get_isp_default(ViPipe, &pSnsInfo->stCmosDft);
		/*TODO@CF. This value will give from sensor init. Write constant first.*/
		pstIspCtx->u8AEWaitFrame = 8;
	}

	return CVI_SUCCESS;
}

CVI_S32 isp_sensor_updateBlc(VI_PIPE ViPipe, ISP_BLACK_LEVEL_ATTR_S **ppstSnsBlackLevel)
{
	ISP_SNS_CTX_S *pSnsInfo = CVI_NULL;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	SNS_CTX_GET(ViPipe, pSnsInfo);
	if (ppstSnsBlackLevel == CVI_NULL) {
		return CVI_FAILURE;
	}

	if (pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_get_isp_black_level != NULL)
		pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_get_isp_black_level(ViPipe, &pSnsInfo->stSnsBlackLevel);

	*ppstSnsBlackLevel = &(pSnsInfo->stSnsBlackLevel.blcAttr);

	return CVI_SUCCESS;
}

CVI_S32 isp_sensor_default_get(VI_PIPE ViPipe, ISP_CMOS_DEFAULT_S **ppstSnsDft)
{
	ISP_SNS_CTX_S *pSnsInfo = CVI_NULL;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	if (ppstSnsDft == CVI_NULL) {
		return CVI_FAILURE;
	}

	SNS_CTX_GET(ViPipe, pSnsInfo);
	if (pSnsInfo == CVI_NULL) {
		return CVI_FAILURE;
	}

	*ppstSnsDft = &pSnsInfo->stCmosDft;

	return CVI_SUCCESS;
}

CVI_S32 isp_sensor_regCfg_get(VI_PIPE ViPipe)
{
	ISP_SNS_CTX_S *pSnsInfo = CVI_NULL;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	SNS_CTX_GET(ViPipe, pSnsInfo);
	if (pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_get_sns_reg_info != CVI_NULL) {
		pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_get_sns_reg_info(ViPipe, &pSnsInfo->stSnsSyncCfg);
		pSnsInfo->stSnsSyncCfg.snsCfg.bConfig = CVI_TRUE;
	}

	return CVI_SUCCESS;
}

/*TODO@CF. This function not complete.*/
CVI_S32 isp_sensor_switchMode(CVI_S32 ViPipe)
{
	ISP_SNS_CTX_S *pSnsInfo;
	ISP_CMOS_SENSOR_IMAGE_MODE_S mode;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	SNS_CTX_GET(ViPipe, pSnsInfo);

	if (pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_set_image_mode != NULL)
		pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_set_image_mode(ViPipe, &mode);

	return 0;
}

CVI_S32 isp_sensor_setWdrMode(CVI_S32 ViPipe, WDR_MODE_E wdrMode)
{
	ISP_SNS_CTX_S *pSnsInfo;
	CVI_U8 mode = wdrMode;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	SNS_CTX_GET(ViPipe, pSnsInfo);

	if (pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_set_wdr_mode != NULL)
		pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_set_wdr_mode(ViPipe, mode);

	return 0;
}

CVI_S32 isp_sensor_exit(CVI_S32 ViPipe)
{
	ISP_SNS_CTX_S *pSnsInfo;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	SNS_CTX_GET(ViPipe, pSnsInfo);

	if (pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_sensor_exit != NULL)
		pSnsInfo->snsRegFunc.stSnsExp.pfn_cmos_sensor_exit(ViPipe);

	return CVI_SUCCESS;
}

CVI_S32 isp_sensor_getId(VI_PIPE ViPipe, SENSOR_ID *pSensorId)
{
	ISP_SNS_CTX_S *pSnsInfo;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	SNS_CTX_GET(ViPipe, pSnsInfo);

	*pSensorId = pSnsInfo->snsAttr.eSensorId;

	return CVI_SUCCESS;
}

