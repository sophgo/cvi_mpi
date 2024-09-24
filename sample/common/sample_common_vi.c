/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample/common/sample_common_vi.c
 * Description:
 *   Common sample code for video input.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include "cvi_mipi.h"
#include "cvi_sns_ctrl.h"
#include <cvi_defines.h>
#include <cvi_common.h>
#include "cvi_awb_comm.h"
#include "cvi_af_comm.h"
#include "cvi_comm_isp.h"
#include "sample_comm.h"
#include "cvi_isp.h"
#include "ini.h"

#define SNSCFGPATH_SIZE 100
static CVI_CHAR g_snsCfgPath[SNSCFGPATH_SIZE];
int cur_sns_num;

// default is MIPI-CSI Bayer format sensor
VI_DEV_ATTR_S DEV_ATTR_SENSOR_BASE = {
	VI_MODE_MIPI,
	VI_WORK_MODE_1Multiplex,
	VI_SCAN_PROGRESSIVE,
	{-1, -1, -1, -1},
	VI_DATA_SEQ_YUYV,

	{
	/*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
	VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH,
	VI_VSYNC_VALID_SIGNAL, VI_VSYNC_VALID_NEG_HIGH,

	/*hsync_hfb    hsync_act    hsync_hhb*/
	{0,            1920,        0,
	/*vsync0_vhb vsync0_act vsync0_hhb*/
	 0,            1080,        0,
	/*vsync1_vhb vsync1_act vsync1_hhb*/
	 0,            0,            0}
	},
	VI_DATA_TYPE_RGB,
	{1920, 1080},
	{
		WDR_MODE_NONE,
		1080,
		0
	},
	.enBayerFormat = BAYER_FORMAT_BG,
};

// default is output YUV420 image
VI_CHN_ATTR_S CHN_ATTR_420_SDR8 = {
	{1920, 1080},
	PIXEL_FORMAT_YUV_PLANAR_420,
	DYNAMIC_RANGE_SDR8,
	VIDEO_FORMAT_LINEAR,
	COMPRESS_MODE_NONE,
	CVI_FALSE, CVI_FALSE,
	0,
	{ -1, -1},
	-1
};

/*
 * Brief: get picture size(w*h), according enPicSize
 */
CVI_S32 SAMPLE_COMM_SYS_GetPicSize(PIC_SIZE_E enPicSize, SIZE_S *pstSize)
{
	switch (enPicSize) {
	case PIC_CIF:   /* 352 * 288 */
		pstSize->u32Width  = 352;
		pstSize->u32Height = 288;
		break;

	case PIC_D1_PAL:   /* 720 * 576 */
		pstSize->u32Width  = 720;
		pstSize->u32Height = 576;
		break;

	case PIC_D1_NTSC:   /* 720 * 480 */
		pstSize->u32Width  = 720;
		pstSize->u32Height = 480;
		break;

	case PIC_720P:   /* 1280 * 720 */
		pstSize->u32Width  = 1280;
		pstSize->u32Height = 720;
		break;

	case PIC_1600x1200:
		pstSize->u32Width  = 1600;
		pstSize->u32Height = 1200;
		break;

	case PIC_1080P:  /* 1920 * 1080 */
		pstSize->u32Width  = 1920;
		pstSize->u32Height = 1080;
		break;

	case PIC_1088:  /* 1920 * 1088*/
		pstSize->u32Width  = 1920;
		pstSize->u32Height = 1088;
		break;

	case PIC_1440P:  /* 2560 * 1440 */
		pstSize->u32Width  = 2560;
		pstSize->u32Height = 1440;
		break;

	case PIC_1536P:  /* 2048 * 1536 */
		pstSize->u32Width  = 2048;
		pstSize->u32Height = 1536;
		break;

	case PIC_2304x1296:
		pstSize->u32Width  = 2304;
		pstSize->u32Height = 1296;
		break;

	case PIC_2048x1536:
		pstSize->u32Width  = 2048;
		pstSize->u32Height = 1536;
		break;

	case PIC_2592x1520:
		pstSize->u32Width  = 2592;
		pstSize->u32Height = 1520;
		break;

	case PIC_2560x1600:
		pstSize->u32Width  = 2560;
		pstSize->u32Height = 1600;
		break;

	case PIC_2560x1944:
		pstSize->u32Width  = 2560;
		pstSize->u32Height = 1944;
		break;

	case PIC_2592x1944:
		pstSize->u32Width  = 2592;
		pstSize->u32Height = 1944;
		break;

	case PIC_2592x1536:
		pstSize->u32Width  = 2592;
		pstSize->u32Height = 1536;
		break;

	case PIC_2688x1520:
		pstSize->u32Width  = 2688;
		pstSize->u32Height = 1520;
		break;

	case PIC_2716x1524:
		pstSize->u32Width  = 2716;
		pstSize->u32Height = 1524;
		break;

	case PIC_2880x1620:
		pstSize->u32Width  = 2880;
		pstSize->u32Height = 1620;
		break;

	case PIC_3844x1124:
		pstSize->u32Width  = 3844;
		pstSize->u32Height = 1124;
		break;

	case PIC_3840x2160:
		pstSize->u32Width  = 3840;
		pstSize->u32Height = 2160;
		break;

	case PIC_3000x3000:
		pstSize->u32Width  = 3000;
		pstSize->u32Height = 3000;
		break;

	case PIC_4000x3000:
		pstSize->u32Width  = 4000;
		pstSize->u32Height = 3000;
		break;

	case PIC_4096x2160:
		pstSize->u32Width  = 4096;
		pstSize->u32Height = 2160;
		break;

	case PIC_3840x8640:
		pstSize->u32Width = 3840;
		pstSize->u32Height = 8640;
		break;

	case PIC_7688x1124:
		pstSize->u32Width = 7688;
		pstSize->u32Height = 1124;
		break;

	case PIC_640x480:
		pstSize->u32Width = 640;
		pstSize->u32Height = 480;
		break;
	case PIC_479P:  /* 632 * 479 */
		pstSize->u32Width  = 632;
		pstSize->u32Height = 479;
		break;
	case PIC_400x400:
		pstSize->u32Width  = 400;
		pstSize->u32Height = 400;
		break;
	case PIC_288P:  /* 384 * 288 */
		pstSize->u32Width  = 384;
		pstSize->u32Height = 288;
		break;
	case PIC_1280x1024:
		pstSize->u32Width  = 1280;
		pstSize->u32Height = 1024;
		break;
	default:
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void SAMPLE_COMM_VI_GetSensorInfo(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 i;

	memset(pstViConfig, 0, sizeof(*pstViConfig));

	for (i = 0; i < VI_MAX_DEV_NUM; i++) {
		pstViConfig->astViInfo[i].stSnsInfo.s32SnsId = i;
		pstViConfig->astViInfo[i].stSnsInfo.s32BusId = i;
		pstViConfig->astViInfo[i].stSnsInfo.MipiDev  = i;
		memset(&pstViConfig->astViInfo[i].stSnapInfo, 0, sizeof(SAMPLE_SNAP_INFO_S));
		pstViConfig->astViInfo[i].stPipeInfo.bMultiPipe = CVI_FALSE;
		pstViConfig->astViInfo[i].stPipeInfo.bVcNumCfged = CVI_FALSE;
	}

	pstViConfig->astViInfo[0].stSnsInfo.enSnsType = SONY_IMX307_MIPI_2M_30FPS_12BIT;
	pstViConfig->astViInfo[1].stSnsInfo.enSnsType = SONY_IMX307_MIPI_2M_30FPS_12BIT;
}

CVI_S32 SAMPLE_COMM_VI_GetDevAttrBySns(SNS_TYPE_E enSnsType, VI_DEV_ATTR_S *pstViDevAttr)
{
	PIC_SIZE_E enPicSize;
	SIZE_S stSize;

	memcpy(pstViDevAttr, &DEV_ATTR_SENSOR_BASE, sizeof(VI_DEV_ATTR_S));

	SAMPLE_COMM_VI_GetSizeBySensor(enSnsType, &enPicSize);
	SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);

	pstViDevAttr->stSize.u32Width = stSize.u32Width;
	pstViDevAttr->stSize.u32Height = stSize.u32Height;
	pstViDevAttr->stWDRAttr.u32CacheLine = stSize.u32Height;

	// WDR mode
	if (enSnsType >= SNS_TYPE_LINEAR_BUTT)
		pstViDevAttr->stWDRAttr.enWDRMode = WDR_MODE_2To1_LINE;
	// set synthetic wdr mode
	switch (enSnsType) {
	case SMS_SC1336_1L_MIPI_1M_30FPS_10BIT_WDR2TO1:
	case SMS_SC1336_1L_MIPI_1M_60FPS_10BIT_WDR2TO1:
	case SMS_SC1336_2L_MIPI_1M_30FPS_10BIT_WDR2TO1:
	case SMS_SC1336_2L_MIPI_1M_60FPS_10BIT_WDR2TO1:
	case SMS_SC1346_1L_MIPI_1M_30FPS_10BIT_WDR2TO1:
	case SMS_SC1346_1L_MIPI_1M_60FPS_10BIT_WDR2TO1:
		pstViDevAttr->stWDRAttr.bSyntheticWDR = 1;
		break;
	default:
		pstViDevAttr->stWDRAttr.bSyntheticWDR = 0;
		break;
	}
	// YUV Sensor
	switch (enSnsType) {
	case PIXELPLUS_PR2020_1M_25FPS_8BIT:
	case PIXELPLUS_PR2020_1M_30FPS_8BIT:
	case PIXELPLUS_PR2020_2M_25FPS_8BIT:
	case PIXELPLUS_PR2020_2M_30FPS_8BIT:
		pstViDevAttr->enDataSeq = VI_DATA_SEQ_YUYV;
		pstViDevAttr->enInputDataType = VI_DATA_TYPE_YUV;
		pstViDevAttr->enIntfMode = VI_MODE_MIPI_YUV422;
		break;
	default:
		break;
	};

	// TODO add BT601 sensor
	switch (enSnsType) {
	case GCORE_GC0308_MIPI_1M_30FPS_8BIT:
	case GCORE_GC2145_MIPI_2M_12FPS_8BIT:
		pstViDevAttr->enDataSeq = VI_DATA_SEQ_YUYV;
		pstViDevAttr->enInputDataType = VI_DATA_TYPE_YUV;
		pstViDevAttr->enIntfMode = VI_MODE_BT601;
		break;
	default:
		break;
	}
	// BT656
	switch (enSnsType) {
	case PIXELPLUS_PR2020_1M_25FPS_8BIT:
	case PIXELPLUS_PR2020_1M_30FPS_8BIT:
	case PIXELPLUS_PR2020_2M_25FPS_8BIT:
	case PIXELPLUS_PR2020_2M_30FPS_8BIT:
		pstViDevAttr->enIntfMode = VI_MODE_BT656;
		break;
	default:
		break;
	};

	//TODO Add BT1120 sensor

	//TODO Add subLVDS sensor

	switch (enSnsType) {
	// Sony
	case SONY_IMX307_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
	case SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT_WDR2TO1:
	case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
	case SONY_IMX327_MIPI_2M_30FPS_12BIT:
	case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
	case SONY_IMX675_MIPI_5M_30FPS_12BIT:
	case SONY_IMX675_MIPI_5M_25FPS_12BIT_WDR2TO1:
	// GalaxyCore
	case GCORE_GC02M1_MIPI_2M_30FPS_10BIT:
	case GCORE_GC1054_MIPI_1M_30FPS_10BIT:
	case GCORE_GC2053_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2093_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2093_MIPI_2M_30FPS_10BIT_WDR2TO1:
	case GCORE_GC4023_MIPI_4M_30FPS_10BIT:
		pstViDevAttr->enBayerFormat = BAYER_FORMAT_RG;
		break;
	// brigates
	case OV_OG01A1B_MIPI_2M_30FPS_10BIT:
	case OV_OG01A10_MIPI_2M_30FPS_10BIT:
		pstViDevAttr->enBayerFormat = BAYER_FORMAT_GB;
		break;
	case GCORE_GC4653_MIPI_4M_30FPS_10BIT:
		pstViDevAttr->enBayerFormat = BAYER_FORMAT_GR;
		break;
	default:
		pstViDevAttr->enBayerFormat = BAYER_FORMAT_BG;
		break;
	};

	// virtual channel for multi-ch
	switch (enSnsType) {
	default:
		pstViDevAttr->enWorkMode = VI_WORK_MODE_1Multiplex;
		break;
	}

	// Is enable vi sbm depend on sensor timing
	switch (enSnsType) {
	case GCORE_GC4653_MIPI_4M_30FPS_10BIT:
	case SONY_IMX675_MIPI_5M_30FPS_12BIT:
	case SONY_IMX675_MIPI_5M_25FPS_12BIT_WDR2TO1:
		pstViDevAttr->disEnableSbm = 1;
		break;
	default:
		pstViDevAttr->disEnableSbm = 0;
		break;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_GetChnAttrBySns(SNS_TYPE_E enSnsType, VI_CHN_ATTR_S *pstChnAttr)
{
	VI_DEV_ATTR_S stViDevAttr;

	memcpy(pstChnAttr, &CHN_ATTR_420_SDR8, sizeof(VI_CHN_ATTR_S));

	SAMPLE_COMM_VI_GetDevAttrBySns(enSnsType, &stViDevAttr);
	if (stViDevAttr.enInputDataType == VI_DATA_TYPE_YUV)
		pstChnAttr->enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_422;

	pstChnAttr->stSize.u32Width = stViDevAttr.stSize.u32Width;
	pstChnAttr->stSize.u32Height = stViDevAttr.stSize.u32Height;

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_ResetSensor(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = 0, i;
	CVI_U32 devno = 0;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		devno = pstViInfo->stSnsInfo.MipiDev;
		if (pstViConfig->astViInfo[i].stPipeInfo.aPipe[0] >= VI_MAX_PHY_DEV_NUM)
			return CVI_SUCCESS;
		s32Ret = CVI_SENSOR_SetSnsGpioInit(devno, pstViInfo->stSnsInfo.s32RstPort,
			pstViInfo->stSnsInfo.s32RstPin, pstViInfo->stSnsInfo.s32RstPol);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d GPIO init failed!\n", i);
			return s32Ret;
		}
		s32Ret = CVI_SENSOR_RstSnsGpio(devno, 1);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d reset failed!\n", i);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_ResetMipi(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = 0, i;
	CVI_U32 devno = 0;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		devno = pstViInfo->stSnsInfo.MipiDev;
		if (pstViConfig->astViInfo[i].stPipeInfo.aPipe[0] >= VI_MAX_PHY_DEV_NUM)
			return CVI_SUCCESS;
		s32Ret = CVI_SENSOR_RstMipi(devno, 1);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "mipi %d reset failed!\n", i);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_UnresetSensor(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = 0, i;
	CVI_U32 devno = 0;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		devno = pstViInfo->stSnsInfo.MipiDev;
		if (pstViConfig->astViInfo[i].stPipeInfo.aPipe[0] >= VI_MAX_PHY_DEV_NUM)
			return CVI_SUCCESS;
		s32Ret = CVI_SENSOR_RstSnsGpio(devno, 0);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d unreset failed!\n", i);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_UnresetMipi(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = 0, i;
	CVI_U32 devno = 0;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		devno = pstViInfo->stSnsInfo.MipiDev;
		if (pstViConfig->astViInfo[i].stPipeInfo.aPipe[0] >= VI_MAX_PHY_DEV_NUM)
			return CVI_SUCCESS;
		s32Ret = CVI_MIPI_SetMipiReset(devno, 0);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("mipi %d unreset failed!\n", i);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_SetMipiAttr(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = 0, i;
	VI_PIPE ViPipe;
	CVI_U32 u32SnsId;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;
	SNS_TYPE_E enSnsType;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		ViPipe = pstViInfo->stPipeInfo.aPipe[0];
		u32SnsId = pstViInfo->stSnsInfo.s32SnsId;
		enSnsType = g_enSnsType[u32SnsId];
		/* need to invert the clk for timnig issue. */
		//if (enSnsType == VIVO_MCS369Q_4M_30FPS_12BIT ||
		//		enSnsType == VIVO_MCS369_2M_30FPS_12BIT)
		//	CVI_MIPI_SetClkEdge(ViPipe, 0);
		s32Ret = CVI_SENSOR_SetMipiAttr(ViPipe, enSnsType);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "set mipi attr sensor(%d) failed!\n", u32SnsId);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_EnableSensorClock(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = 0, i;
	CVI_U32 devno = 0;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		devno = pstViInfo->stSnsInfo.MipiDev;
		if (pstViConfig->astViInfo[i].stPipeInfo.aPipe[0] >= VI_MAX_PHY_DEV_NUM)
			return CVI_SUCCESS;
		s32Ret = CVI_SENSOR_EnableSnsClk(devno, 1);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d clock enable failed!\n", i);
			return s32Ret;
		}
	}

	return CVI_SUCCESS;
}

struct VI_PM_DATA_S {
	VI_PIPE ViPipe;
	CVI_U32 u32SnsId;
	CVI_S32 s32DevNo;
};

static struct VI_PM_DATA_S ViPmData[VI_MAX_DEV_NUM] = { 0 };

static CVI_S32 SAMPLE_COMM_VI_SuspendSensor(CVI_VOID *pvData)
{
	struct VI_PM_DATA_S *pstPmData = (struct VI_PM_DATA_S *)pvData;
	CVI_U32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe;
	CVI_U32 u32SnsId;

	if (!pvData) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "pvData is NULL!\n");
		return CVI_FAILURE;
	}
	ViPipe = pstPmData->ViPipe;
	u32SnsId = pstPmData->u32SnsId;

	s32Ret = CVI_SENSOR_SetSnsType(u32SnsId, g_enSnsType[u32SnsId]);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor type(%d) failed!\n", u32SnsId);
		return s32Ret;
	}
	s32Ret = CVI_SENSOR_SetSnsStandby(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor sdandby failed!\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 SAMPLE_COMM_VI_ResumeSensor(CVI_VOID *pvData)
{
	struct VI_PM_DATA_S *pstPmData = (struct VI_PM_DATA_S *)pvData;
	CVI_U32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe;
	CVI_U32 u32SnsId;

	if (!pvData) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "pvData is NULL!\n");
		return CVI_FAILURE;
	}
	ViPipe = pstPmData->ViPipe;
	u32SnsId = pstPmData->u32SnsId;

	s32Ret = CVI_SENSOR_SetSnsType(u32SnsId, g_enSnsType[u32SnsId]);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor type(%d) failed!\n", u32SnsId);
		return s32Ret;
	}
	s32Ret = CVI_SENSOR_SetSnsInit(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor resume failed!\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 SAMPLE_COMM_VI_SuspendMipi(CVI_VOID *pvData)
{
	struct VI_PM_DATA_S *pstPmData = (struct VI_PM_DATA_S *)pvData;
	CVI_S32 devno;
	CVI_S32 s32Ret = 0;

	if (!pvData) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "pvData is NULL!\n");
		return CVI_FAILURE;
	}
	devno = pstPmData->s32DevNo;

	s32Ret = CVI_SENSOR_EnableSnsClk(devno, 0);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "dev %d clock enable failed!\n", devno);
	}

	return s32Ret;
}

static CVI_S32 SAMPLE_COMM_VI_ResumeMipi(CVI_VOID *pvData)
{
	struct VI_PM_DATA_S *pstPmData = (struct VI_PM_DATA_S *)pvData;
	VI_PIPE ViPipe;
	CVI_U32 u32SnsId;
	CVI_S32 devno;
	SNS_TYPE_E enSnsType;
	CVI_S32 s32Ret = 0;

	if (!pvData) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "pvData is NULL!\n");
		return CVI_FAILURE;
	}

	ViPipe = pstPmData->ViPipe;
	u32SnsId = pstPmData->u32SnsId;
	devno = pstPmData->s32DevNo;

	/* sensor reset */
	s32Ret = CVI_SENSOR_RstSnsGpio(devno, 1);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d reset failed!\n", u32SnsId);
		return s32Ret;
	}
	/* mipi-rx reset */
	s32Ret = CVI_SENSOR_RstMipi(devno, 1);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "mipi %d reset failed!\n", u32SnsId);
		return s32Ret;
	}
	/* set mipi-rx attribute */
	enSnsType = g_enSnsType[u32SnsId];
	/* need to invert the clk for timnig issue. */
	//if (enSnsType == VIVO_MCS369Q_4M_30FPS_12BIT ||
	//		enSnsType == VIVO_MCS369_2M_30FPS_12BIT)
	//	CVI_MIPI_SetClkEdge(ViPipe, 0);
	CVI_SENSOR_SetMipiAttr(ViPipe, enSnsType);
	/* enable sensor clock. */
	s32Ret = CVI_SENSOR_EnableSnsClk(devno, 1);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d clock enable failed!\n", u32SnsId);
		return s32Ret;
	}
	usleep(20);
	/* sensor unreset */
	s32Ret = CVI_SENSOR_RstSnsGpio(devno, 0);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d reset failed!\n", u32SnsId);
	}
	usleep(10000);

	return s32Ret;
}

static VI_PM_OPS_S vi_ops = {
	.pfnSnsSuspend = SAMPLE_COMM_VI_SuspendSensor,
	.pfnSnsResume = SAMPLE_COMM_VI_ResumeSensor,
	.pfnMipiSuspend = SAMPLE_COMM_VI_SuspendMipi,
	.pfnMipiResume = SAMPLE_COMM_VI_ResumeMipi,
};

CVI_S32 SAMPLE_COMM_VI_StartSensor(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret, i;
	CVI_U32 u32SnsId;
	VI_PIPE ViPipe;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		ViPipe = pstViInfo->stPipeInfo.aPipe[0];
		u32SnsId = pstViInfo->stSnsInfo.s32SnsId;

		s32Ret = CVI_SENSOR_SetSnsType(ViPipe, pstViInfo->stSnsInfo.enSnsType);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor type(%d) failed!\n", ViPipe);
			return s32Ret;
		}

		s32Ret = SAMPLE_COMM_ISP_SetSnsObj(u32SnsId, pstViInfo->stSnsInfo.enSnsType);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "update sensor obj(%d) failed!\n", u32SnsId);
			return s32Ret;
		}

		s32Ret = SAMPLE_COMM_ISP_SetSnsInit(u32SnsId, pstViInfo->stSnsInfo.u8HwSync);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "update sensor(%d) hwsync failed !\n", u32SnsId);
			return s32Ret;
		}

		s32Ret = SAMPLE_COMM_ISP_PatchSnsObj(ViPipe, &pstViInfo->stSnsInfo);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "patch rx attr(%d) failed!\n", ViPipe);
			return s32Ret;
		}

		s32Ret = SAMPLE_COMM_ISP_Sensor_Regiter_callback(ViPipe, u32SnsId, pstViInfo->stSnsInfo.s32BusId,
								pstViInfo->stSnsInfo.s32SnsI2cAddr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d register callback failed!\n", i);
			return s32Ret;
		}
	}

	s32Ret = SAMPLE_COMM_ISP_SetSensorMode(pstViConfig, pstViConfig->s32WorkingViNum);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d register callback failed!\n", i);
		return s32Ret;
	}
	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_StartMIPI(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = CVI_SUCCESS, i;
	VI_PIPE ViPipe;
#if (CONFIG_CVI_LOG_TRACE_SUPPORT == 1)
	CVI_U32 u32SnsId;
#endif
	SNS_COMBO_DEV_ATTR_S stDevAttr;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	/*TODO@CF. Need add sample function.*/
	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		ViPipe = pstViInfo->stPipeInfo.aPipe[0];
#if (CONFIG_CVI_LOG_TRACE_SUPPORT == 1)
		u32SnsId = pstViInfo->stSnsInfo.s32SnsId;
#endif
		s32Ret = CVI_SENSOR_SetSnsType(ViPipe, pstViInfo->stSnsInfo.enSnsType);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor type(%d) failed!\n", u32SnsId);
			return s32Ret;
		}
		s32Ret = CVI_SENSOR_GetSnsRxAttr(ViPipe, &stDevAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sensor get rx attr failed!\n");
			return s32Ret;
		}
		SAMPLE_PRT("sensor %d stDevAttr.devno %d\n", i, stDevAttr.devno);
		pstViInfo->stSnsInfo.MipiDev = stDevAttr.devno;
	}
	s32Ret = SAMPLE_COMM_VI_ResetSensor(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "ResetSensor failed! with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VI_ResetMipi(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "ResetMipi failed! with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VI_SetMipiAttr(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_SetMipiAttr failed! with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VI_EnableSensorClock(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "EnableSensorClock failed! with %#x!\n", s32Ret);
		return s32Ret;
	}

	usleep(20);
	s32Ret = SAMPLE_COMM_VI_UnresetSensor(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "UnresetSensor failed! with %#x!\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_SensorProbe(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = CVI_SUCCESS, i;
	VI_PIPE ViPipe;
#if (CONFIG_CVI_LOG_TRACE_SUPPORT == 1)
	CVI_U32 u32SnsId;
#endif
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		ViPipe = pstViInfo->stPipeInfo.aPipe[0];
#if (CONFIG_CVI_LOG_TRACE_SUPPORT == 1)
		u32SnsId = pstViInfo->stSnsInfo.s32SnsId;
#endif
		s32Ret = CVI_SENSOR_SetSnsType(ViPipe, pstViInfo->stSnsInfo.enSnsType);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor type(%d) failed!\n", u32SnsId);
			return s32Ret;
		}
		s32Ret = CVI_SENSOR_SetSnsProbe(ViPipe);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "cannot probe the SnsId(%d)\n", u32SnsId);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_StartDev(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32             s32Ret;
	VI_DEV              ViDev;
	SNS_TYPE_E   enSnsType;
	VI_DEV_ATTR_S       stViDevAttr;
	ISP_PUB_ATTR_S      pstPubAttr;

	ViDev       = pstViInfo->stDevInfo.ViDev;
	enSnsType   = pstViInfo->stSnsInfo.enSnsType;

	SAMPLE_COMM_VI_GetDevAttrBySns(enSnsType, &stViDevAttr);
	SAMPLE_COMM_ISP_GetIspAttrBySns(enSnsType, &pstPubAttr);
	pstPubAttr.f32FrameRate = pstViInfo->stSnsInfo.f32FpsRate;
	stViDevAttr.stWDRAttr.enWDRMode = pstViInfo->stDevInfo.enWDRMode;
	stViDevAttr.snrFps = (CVI_U32)pstPubAttr.f32FrameRate;

	if (pstViInfo->stSnsInfo.u8MuxDev) {
		stViDevAttr.isMux = true;
		stViDevAttr.switchGpioIdx = pstViInfo->stSnsInfo.s16SwitchPort;
		stViDevAttr.switchGpioPin = pstViInfo->stSnsInfo.s16SwitchGpio;
		stViDevAttr.switchGPioPol = pstViInfo->stSnsInfo.s16SwitchPol;
		stViDevAttr.isFrmCtrl = false;
	}

	s32Ret = CVI_VI_SetDevAttr(ViDev, &stViDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_SetDevAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_EnableDev(ViDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_EnableDev failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_StopDev(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret;
	VI_DEV ViDev;

	ViDev   = pstViInfo->stDevInfo.ViDev;
	s32Ret  = CVI_VI_DisableDev(ViDev);

	CVI_VI_UnRegChnFlipMirrorCallBack(0, ViDev);
	CVI_VI_UnRegPmCallBack(ViDev);
	memset(&ViPmData[ViDev], 0, sizeof(struct VI_PM_DATA_S));

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_DisableDev failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_BindPipeDev(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32             i;
	CVI_S32             s32PipeCnt = 0;
	CVI_S32             s32Ret;
	VI_DEV_BIND_PIPE_S  stDevBindPipe = {0};

	for (i = 0; i < 4; i++) {
		if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM) {
			stDevBindPipe.PipeId[s32PipeCnt] = pstViInfo->stPipeInfo.aPipe[i];
			s32PipeCnt++;
			stDevBindPipe.u32Num = s32PipeCnt;
		}
	}

	s32Ret = CVI_VI_SetDevBindPipe(pstViInfo->stDevInfo.ViDev, &stDevBindPipe);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetDevBindPipe failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_GetYuvBypassSts(SNS_TYPE_E enSnsType)
{
	CVI_S32 s32Ret = 0;
	//Set YUV sensor need bypass isp
	switch (enSnsType) {
	case GCORE_GC2145_MIPI_2M_12FPS_8BIT:
	case PIXELPLUS_PR2020_1M_25FPS_8BIT:
	case PIXELPLUS_PR2020_1M_30FPS_8BIT:
	case PIXELPLUS_PR2020_2M_25FPS_8BIT:
	case PIXELPLUS_PR2020_2M_30FPS_8BIT:
	case GCORE_GC0308_MIPI_1M_30FPS_8BIT:
		s32Ret = 1;
		break;
	default:
		break;
	}
	return s32Ret;
}
/******************************************************************************
 * funciton : Get enSize by diffrent sensor
 ******************************************************************************/
CVI_S32 SAMPLE_COMM_VI_GetSizeBySensor(SNS_TYPE_E enMode, PIC_SIZE_E *penSize)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (!penSize)
		return CVI_FAILURE;

	switch (enMode) {
	case GCORE_GC1054_MIPI_1M_30FPS_10BIT:
	case PIXELPLUS_PR2020_1M_25FPS_8BIT:
	case PIXELPLUS_PR2020_1M_30FPS_8BIT:
	case SMS_SC1336_1L_MIPI_1M_30FPS_10BIT:
	case SMS_SC1336_1L_MIPI_1M_30FPS_10BIT_WDR2TO1:
	case SMS_SC1336_1L_MIPI_1M_60FPS_10BIT:
	case SMS_SC1336_1L_MIPI_1M_60FPS_10BIT_WDR2TO1:
	case SMS_SC1336_1L_SLAVE_MIPI_1M_30FPS_10BIT:
	case SMS_SC1336_2L_MIPI_1M_30FPS_10BIT:
	case SMS_SC1336_2L_MIPI_1M_30FPS_10BIT_WDR2TO1:
	case SMS_SC1336_2L_MIPI_1M_60FPS_10BIT:
	case SMS_SC1336_2L_MIPI_1M_60FPS_10BIT_WDR2TO1:
	case SMS_SC1336_2L_SLAVE_MIPI_1M_30FPS_10BIT:
	case SMS_SC1346_1L_MIPI_1M_30FPS_10BIT:
	case SMS_SC1346_1L_MIPI_1M_30FPS_10BIT_WDR2TO1:
	case SMS_SC1346_1L_MIPI_1M_60FPS_10BIT:
	case SMS_SC1346_1L_MIPI_1M_60FPS_10BIT_WDR2TO1:
	case SMS_SC1346_1L_SLAVE_MIPI_1M_30FPS_10BIT:
	case SMS_SC1346_1L_SLAVE_MIPI_1M_60FPS_10BIT:
		*penSize = PIC_720P;
		break;
	case GCORE_GC02M1_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2145_MIPI_2M_12FPS_8BIT:
	case BYD_BF2253L_MIPI_1200P_30FPS_10BIT:
		*penSize = PIC_1600x1200;
		break;
	case GCORE_GC2053_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2093_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2093_MIPI_2M_30FPS_10BIT_WDR2TO1:
	case PIXELPLUS_PR2020_2M_25FPS_8BIT:
	case PIXELPLUS_PR2020_2M_30FPS_8BIT:
	case SONY_IMX307_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
	case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
	case SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT_WDR2TO1:
	case SONY_IMX327_MIPI_2M_30FPS_12BIT:
	case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
	case SMS_SC230AI_2L_MIPI_2M_30FPS_10BIT:
	case SMS_SC230AI_2L_SLAVE_MIPI_2M_30FPS_10BIT:
	case SMS_SC2336_SLAVE_MIPI_2M_30FPS_10BIT:
	case SMS_SC2336_SLAVE1_MIPI_2M_30FPS_10BIT:
	case SMS_SC2331_1L_MIPI_2M_30FPS_10BIT:
	case SMS_SC2336_MIPI_2M_30FPS_10BIT:
		*penSize = PIC_1080P;
		break;
	case GCORE_GC4023_MIPI_4M_30FPS_10BIT:
	case GCORE_GC4653_MIPI_4M_30FPS_10BIT:
		*penSize = PIC_1440P;
		break;
	case SMS_SC301IOT_MIPI_3M_30FPS_10BIT:
		*penSize = PIC_1536P;
		break;
	case GCORE_GC0308_MIPI_1M_30FPS_8BIT:
	case SMS_SC035HGS_1L_MIPI_480P_120FPS_10BIT:
	case SMS_SC035HGS_1L_SLAVE_MIPI_480P_120FPS_10BIT:
		*penSize = PIC_640x480;
		break;
	case OV_OG01A1B_MIPI_2M_30FPS_10BIT:
	case OV_OG01A10_MIPI_2M_30FPS_10BIT:
		*penSize = PIC_1280x1024;
		break;
	case SONY_IMX675_MIPI_5M_30FPS_12BIT:
	case SONY_IMX675_MIPI_5M_25FPS_12BIT_WDR2TO1:
		*penSize = PIC_2560x1944;
		break;
	default:
		s32Ret = CVI_FAILURE;
		break;
	}
	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_StartViChn(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32             i;
	CVI_S32             s32Ret = CVI_SUCCESS;
	VI_PIPE             ViPipe = 0;
	VI_CHN              ViChn = 0;
	VI_DEV              ViDev = 0;
	CVI_U32             u32SnsId = 0;
	VI_CHN_ATTR_S       stChnAttr;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		if (i < VI_MAX_DEV_NUM) {
			ViPipe	    = pstViConfig->astViInfo[i].stPipeInfo.aPipe[0];
			ViChn	    = pstViConfig->astViInfo[i].stChnInfo.ViChn;
			ViDev	    = pstViConfig->astViInfo[i].stDevInfo.ViDev;
			u32SnsId    = pstViConfig->astViInfo[i].stSnsInfo.s32SnsId;

			SAMPLE_COMM_VI_GetChnAttrBySns(pstViConfig->astViInfo[i].stSnsInfo.enSnsType, &stChnAttr);
			stChnAttr.enDynamicRange = pstViConfig->astViInfo[i].stChnInfo.enDynamicRange;
			stChnAttr.enVideoFormat  = pstViConfig->astViInfo[i].stChnInfo.enVideoFormat;
			stChnAttr.enCompressMode = pstViConfig->astViInfo[i].stChnInfo.enCompressMode;
			stChnAttr.enPixelFormat = pstViConfig->astViInfo[i].stChnInfo.enPixFormat;
			/* fill the sensor orientation */
			if (pstViConfig->astViInfo[i].stSnsInfo.u8Orien <= 3) {
				stChnAttr.bMirror = pstViConfig->astViInfo[i].stSnsInfo.u8Orien & 0x1;
				stChnAttr.bFlip = pstViConfig->astViInfo[i].stSnsInfo.u8Orien & 0x2;
			}

			s32Ret = CVI_VI_SetChnAttr(ViPipe, ViChn, &stChnAttr);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_SetChnAttr failed with %#x!\n", s32Ret);
				return CVI_FAILURE;
			}
			s32Ret = CVI_SENSOR_SetSnsType(u32SnsId, g_enSnsType[u32SnsId]);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor type(%d) failed!\n", u32SnsId);
				return s32Ret;
			}
			s32Ret = CVI_SENSOR_SetVIFlipMirrorCB(ViPipe, ViDev);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "set vi mirror and filp callback failed!\n");
				return s32Ret;
			}

			/* register the power management ops. */
			ViPmData[ViDev].ViPipe = ViPipe;
			ViPmData[ViDev].u32SnsId = u32SnsId;
			ViPmData[ViDev].s32DevNo = pstViConfig->astViInfo[i].stSnsInfo.MipiDev;
			s32Ret = CVI_VI_RegPmCallBack(ViDev, &vi_ops, (CVI_VOID *)&ViPmData[ViDev]);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_RegPmCallBack failed with %#x!\n", s32Ret);
				return CVI_FAILURE;
			}

			s32Ret = CVI_VI_EnableChn(ViPipe, ViChn);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_EnableChn failed with %#x!\n", s32Ret);
				return CVI_FAILURE;
			}
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_StopViChn(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32             s32Ret = CVI_SUCCESS;
	VI_PIPE             ViPipe = 0;
	VI_CHN              ViChn;
	VI_VPSS_MODE_E      enMastPipeMode;

	ViChn  = pstViInfo->stChnInfo.ViChn;

	if (ViChn < VI_MAX_CHN_NUM) {
		enMastPipeMode = pstViInfo->stPipeInfo.enMastPipeMode;

		if (enMastPipeMode == VI_OFFLINE_VPSS_OFFLINE
		    || enMastPipeMode == VI_ONLINE_VPSS_OFFLINE) {
			s32Ret = CVI_VI_DisableChn(ViPipe, ViChn);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_DisableChn failed with %#x!\n",
								s32Ret);
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_CreateIsp(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	#define USE_OLDAPI_LOADPARA		0

	CVI_S32 i;
	CVI_S32 s32ViNum;
	VI_PIPE ViPipe;
	CVI_S32 s32Ret = CVI_SUCCESS;

	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	if (!pstViConfig) {
		SAMPLE_PRT("%s: null ptr\n", __func__);
		return CVI_FAILURE;
	}

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		s32ViNum  = pstViConfig->as32WorkingViId[i];
		pstViInfo = &pstViConfig->astViInfo[s32ViNum];
		ViPipe = pstViInfo->stPipeInfo.aPipe[0];

		s32Ret = CVI_ISP_Init(ViPipe);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_ISP_Init ViPipe_%d failed !\n", ViPipe);
			return CVI_FAILURE;
		}
		s32Ret = SAMPLE_COMM_VI_StartIsp(pstViInfo);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VI_StartIsp failed !\n");
			return CVI_FAILURE;
		}

		#if USE_OLDAPI_LOADPARA
		s32Ret = SAMPLE_COMM_BIN_ReadBlockParaFrombin(CVI_BIN_ID_ISP0 + i);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_WARN, "read block(%d) para fail: %#x!\n", CVI_BIN_ID_ISP0 + i, s32Ret);
		}
		#else
		if (i == (pstViConfig->s32WorkingViNum - 1)) {
			s32Ret = SAMPLE_COMM_BIN_ReadParaFrombin();
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_WARN, "read para fail: %#x,use default para!\n", s32Ret);
			}
		}
		#endif

		s32Ret = SAMPLE_COMM_ISP_Run(ViPipe);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "ISP_Run failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_StartIsp(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = 0, i;
	VI_PIPE ViPipe = 0;
	ISP_PUB_ATTR_S stPubAttr;
	ISP_BIND_ATTR_S stBindAttr;

	for (i = 0; i < WDR_MAX_PIPE_NUM; i++) {
		if (pstViInfo->stPipeInfo.aPipe[i] >= 0  &&
			pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[0];

			SAMPLE_COMM_ISP_Aelib_Callback(ViPipe);
			SAMPLE_COMM_ISP_Awblib_Callback(ViPipe);
			#if ENABLE_AF_LIB
			SAMPLE_COMM_ISP_Aflib_Callback(ViPipe);
			#endif

			snprintf(stBindAttr.stAeLib.acLibName, sizeof(CVI_AE_LIB_NAME), "%s", CVI_AE_LIB_NAME);
			stBindAttr.stAeLib.s32Id = ViPipe;
			stBindAttr.sensorId = 0;
			snprintf(stBindAttr.stAwbLib.acLibName, sizeof(CVI_AWB_LIB_NAME), "%s", CVI_AWB_LIB_NAME);
			stBindAttr.stAwbLib.s32Id = ViPipe;
			#if ENABLE_AF_LIB
			snprintf(stBindAttr.stAfLib.acLibName, sizeof(CVI_AF_LIB_NAME), "%s", CVI_AF_LIB_NAME);
			stBindAttr.stAfLib.s32Id = ViPipe;
			#endif
			s32Ret = CVI_ISP_SetBindAttr(ViPipe, &stBindAttr);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "Bind Algo failed with %#x!\n", s32Ret);
			}
			s32Ret = CVI_ISP_MemInit(ViPipe);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "Init Ext memory failed with %#x!\n", s32Ret);
				return s32Ret;
			}
			SAMPLE_COMM_ISP_GetIspAttrBySns(pstViInfo->stSnsInfo.enSnsType, &stPubAttr);
			stPubAttr.f32FrameRate = pstViInfo->stSnsInfo.f32FpsRate;
			s32Ret = CVI_ISP_SetPubAttr(ViPipe, &stPubAttr);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "SetPubAttr failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_DestroyIsp(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 i;
	CVI_S32 s32ViNum;
	CVI_S32 s32Ret = CVI_SUCCESS;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	if (!pstViConfig) {
		SAMPLE_PRT("%s: null ptr\n", __func__);
		return CVI_FAILURE;
	}

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		s32ViNum  = pstViConfig->as32WorkingViId[i];
		pstViInfo = &pstViConfig->astViInfo[s32ViNum];

		s32Ret = SAMPLE_COMM_VI_StopIsp(pstViInfo);

		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VI_StopIsp failed !\n");
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_StopIsp(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32 ret = CVI_SUCCESS;
	VI_PIPE ViPipe;

	for (CVI_U32 i = 0; i < WDR_MAX_PIPE_NUM; i++) {
		if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[i];
			SAMPLE_COMM_ISP_Stop(ViPipe);
		}
	}

	return ret;
}

static CVI_S32 SAMPLE_COMM_VI_StopSingleViPipe(VI_PIPE ViPipe)
{
	CVI_S32  s32Ret;

	s32Ret = CVI_VI_StopPipe(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_StopPipe failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_DestroyPipe(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_DestroyPipe failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_StopViPipe(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32  i, ret = CVI_SUCCESS;
	VI_PIPE ViPipe;

	for (i = 0; i < WDR_MAX_PIPE_NUM; i++) {
		if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[i];
			ret = SAMPLE_COMM_VI_StopSingleViPipe(ViPipe);
			if (ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_StopViPipe is fail\n");
				return ret;
			}
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 SAMPLE_COMM_VI_DestroySingleVi(SAMPLE_VI_INFO_S *pstViInfo)
{
	SAMPLE_COMM_VI_StopViChn(pstViInfo);

	SAMPLE_COMM_VI_StopViPipe(pstViInfo);

	SAMPLE_COMM_VI_StopDev(pstViInfo);

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_DestroyVi(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32           i;
	CVI_S32           s32ViNum;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	if (!pstViConfig) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "null ptr\n");
		return CVI_FAILURE;
	}

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		s32ViNum  = pstViConfig->as32WorkingViId[i];
		pstViInfo = &pstViConfig->astViInfo[s32ViNum];

		SAMPLE_COMM_VI_DestroySingleVi(pstViInfo);
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_OPEN(CVI_VOID)
{
	CVI_S32 s32ret = CVI_SUCCESS;

	return s32ret;
}

CVI_S32 SAMPLE_COMM_VI_CLOSE(CVI_VOID)
{
	CVI_S32 s32ret = CVI_SUCCESS;

	return s32ret;
}

const char *snsr_type_name[SNS_TYPE_WDR_BUTT] = {
	"SNS_TYPE_NONE",
	/* ------ LINEAR BEGIN ------*/
	"CVSENS_CV2003_1L_MIPI_2M_30FPS_10BIT",
	"CVSENS_CV2003_1L_SLAVE_MIPI_2M_30FPS_10BIT",
	"CVSENS_CV2003_1L_SLAVE1_MIPI_2M_30FPS_10BIT",
	"SONY_IMX327_MIPI_2M_30FPS_12BIT",
	"SONY_IMX307_MIPI_2M_30FPS_12BIT",
	"SONY_IMX307_2L_MIPI_2M_30FPS_12BIT",
	"SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT",
	"SONY_IMX675_MIPI_5M_30FPS_12BIT",
	"GCORE_GC0308_MIPI_1M_30FPS_8BIT",
	"GCORE_GC1054_MIPI_1M_30FPS_10BIT",
	"GCORE_GC2053_MIPI_2M_30FPS_10BIT",
	"GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT",
	"GCORE_GC2093_MIPI_2M_30FPS_10BIT",
	"GCORE_GC2145_MIPI_2M_12FPS_8BIT",
	"GCORE_GC02M1_MIPI_2M_30FPS_10BIT",
	"GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT",
	"GCORE_GC4023_MIPI_4M_30FPS_10BIT",
	"GCORE_GC4653_MIPI_4M_30FPS_10BIT",
	"OV_OG01A1B_MIPI_2M_30FPS_10BIT",
	"OV_OG01A10_MIPI_2M_30FPS_10BIT",
	"PIXELPLUS_PR2020_1M_25FPS_8BIT",
	"PIXELPLUS_PR2020_1M_30FPS_8BIT",
	"PIXELPLUS_PR2020_2M_25FPS_8BIT",
	"PIXELPLUS_PR2020_2M_30FPS_8BIT",
	"TECHPOINT_TP9950_1M_30FPS_8BIT",
	"TECHPOINT_TP9950_2M_30FPS_8BIT",
	"TECHPOINT_TP9950_1M_25FPS_8BIT",
	"TECHPOINT_TP9950_2M_25FPS_8BIT",
	"SMS_SC035HGS_1L_MIPI_480P_120FPS_10BIT",
	"SMS_SC035HGS_1L_SLAVE_MIPI_480P_120FPS_10BIT",
	"SMS_SC230AI_2L_MIPI_2M_30FPS_10BIT",
	"SMS_SC230AI_2L_SLAVE_MIPI_2M_30FPS_10BIT",
	"SMS_SC1336_1L_MIPI_1M_30FPS_10BIT",
	"SMS_SC1336_1L_MIPI_1M_60FPS_10BIT",
	"SMS_SC1336_1L_SLAVE_MIPI_1M_30FPS_10BIT",
	"SMS_SC1336_2L_MIPI_1M_30FPS_10BIT",
	"SMS_SC1336_2L_MIPI_1M_60FPS_10BIT",
	"SMS_SC1336_2L_SLAVE_MIPI_1M_30FPS_10BIT",
	"SMS_SC1346_1L_MIPI_1M_30FPS_10BIT",
	"SMS_SC1346_1L_MIPI_1M_60FPS_10BIT",
	"SMS_SC1346_1L_SLAVE_MIPI_1M_30FPS_10BIT",
	"SMS_SC1346_1L_SLAVE_MIPI_1M_60FPS_10BIT",
	"SMS_SC2331_1L_MIPI_2M_30FPS_10BIT",
	"SMS_SC2331_1L_SLAVE_MIPI_2M_30FPS_10BIT",
	"SMS_SC2331_1L_SLAVE1_MIPI_2M_30FPS_10BIT",
	"SMS_SC2336_MIPI_2M_30FPS_10BIT",
	"SMS_SC2336_SLAVE_MIPI_2M_30FPS_10BIT",
	"SMS_SC2336_SLAVE1_MIPI_2M_30FPS_10BIT",
	"SMS_SC301IOT_MIPI_3M_30FPS_10BIT",
	"CISTA_C4390_MIPI_4M_30FPS_10BIT",
	"BYD_BF2253L_MIPI_1200P_30FPS_10BIT",
	/* ------ LINEAR END ------*/
	/* ------ WDR 2TO1 BEGIN ------*/
	"SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1",
	"SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1",
	"SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1",
	"SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT_WDR2TO1",
	"SONY_IMX675_MIPI_5M_25FPS_12BIT_WDR2TO1",
	"GCORE_GC2093_MIPI_2M_30FPS_10BIT_WDR2TO1",
	"SMS_SC1336_1L_MIPI_1M_30FPS_10BIT_WDR2TO1",
	"SMS_SC1336_1L_MIPI_1M_60FPS_10BIT_WDR2TO1",
	"SMS_SC1336_2L_MIPI_1M_30FPS_10BIT_WDR2TO1",
	"SMS_SC1336_2L_MIPI_1M_60FPS_10BIT_WDR2TO1",
	"SMS_SC1346_1L_MIPI_1M_30FPS_10BIT_WDR2TO1",
	"SMS_SC1346_1L_MIPI_1M_60FPS_10BIT_WDR2TO1",
	/* ------ WDR 2TO1 END ------*/
};
static SAMPLE_INI_CFG_S	stDefIniCfg = {
	.enSource  = VI_PIPE_FRAME_SOURCE_DEV,
	.devNum    = 1,
	.enSnsType[0] = SONY_IMX327_MIPI_2M_30FPS_12BIT,
	.enWDRMode[0] = WDR_MODE_NONE,
	.s32BusId[0]  = 3,
	.s32SnsI2cAddr[0] = -1,
	.s32SnsI2cAddr[1] = -1,
	.MipiDev[0]   = 0xFF,
	.MipiDev[1]   = 0xFF,
	.u8UseMultiSns = 0,
};

/*=== Source section parser handler begin === */
static void parse_source_type(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	(CVI_VOID) param0;
	(CVI_VOID) param1;
	(CVI_VOID) param2;

	SAMPLE_PRT("source type =  %s\n", value);
	if (strcmp(value, "SOURCE_USER_FE") == 0) {
		cfg->enSource = VI_PIPE_FRAME_SOURCE_USER_FE;
	}
}

static void parse_source_devnum(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	int devno = atoi(value);

	(CVI_VOID) param0;
	(CVI_VOID) param1;
	(CVI_VOID) param2;

	SAMPLE_PRT("devNum =  %s\n", value);

	if (devno >= 1 && devno <= VI_MAX_DEV_NUM)
		cfg->devNum = devno;
	else
		cfg->devNum = 1;
}
/* === Source section parser handler end === */

/* === Sensor section parser handler begin === */
static int parse_lane_id(CVI_S16 *LaneId, const char *value)
{
	char buf[8];
	int offset = 0, idx = 0, k;

	for (k = 0; k < 30; k++) {
		/* find next ',' */
		if (value[k] == ',' || value[k] == '\0') {
			if (k == offset) {
				SAMPLE_PRT("lane_id parse error, is the format correct?\n");
				return -1;
			}
			memset(buf, 0, sizeof(buf));
			memcpy(buf, &value[offset], k - offset);
			buf[k-offset] = '\0';
			LaneId[idx++] = atoi(buf);
			offset = k + 1;
		}

		if (value[k] == '\0' || idx == 5)
			break;
	}

	if (k == 30) {
		SAMPLE_PRT("lane_id parse error, is the format correct?\n");
		return -1;
	}

	return 0;
}

static int parse_pn_swap(CVI_S8 *PNSwap, const char *value)
{
	char buf[8];
	int offset = 0, idx = 0, k;

	for (k = 0; k < 30; k++) {
		/* find next ',' */
		if (value[k] == ',' || value[k] == '\0') {
			if (k == offset) {
				SAMPLE_PRT("lane_id parse error, is the format correct?\n");
				return -1;
			}
			memset(buf, 0, sizeof(buf));
			memcpy(buf, &value[offset], k - offset);
			buf[k-offset] = '\0';
			PNSwap[idx++] = atoi(buf);
			offset = k + 1;
		}

		if (value[k] == '\0' || idx == 5)
			break;
	}

	if (k == 30) {
		SAMPLE_PRT("lane_id parse error, is the format correct?\n");
		return -1;
	}

	return 0;
}

static void parse_sensor_name(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
#define NAME_SIZE 20
	CVI_U32 index = param0;
	CVI_U32 i;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("sensor =  %s\n", value);
	char sensorNameEnv[NAME_SIZE];

	snprintf(sensorNameEnv, NAME_SIZE, "SENSORNAME%d", index);
	setenv(sensorNameEnv, value, 1);

	for (i = 0; i < SNS_TYPE_WDR_BUTT; i++) {
		if (strcmp(value, snsr_type_name[i]) == 0) {
			cfg->enSnsType[index] = i;
			cfg->enWDRMode[index] = (i < SNS_TYPE_LINEAR_BUTT) ?
				WDR_MODE_NONE : WDR_MODE_2To1_LINE;
			break;
		}
	}
	if (i == SNS_TYPE_WDR_BUTT) {
		cfg->enSnsType[index] = SNS_TYPE_WDR_BUTT;
		cfg->enWDRMode[index] = WDR_MODE_NONE;
		cfg->u8UseMultiSns = index;
		SAMPLE_PRT("ERROR: can not find sensor ini in /mnt/data/\n");
	}
}

static void parse_sensor_busid(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("bus_id =  %s\n", value);
	cfg->s32BusId[index] = atoi(value);
}

static void parse_sensor_i2caddr(SAMPLE_INI_CFG_S *cfg, const char *value,
					CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("sns_i2c_addr =  %s\n", value);
	cfg->s32SnsI2cAddr[index] = atoi(value);
}

static void parse_sensor_mipidev(SAMPLE_INI_CFG_S *cfg, const char *value,
					CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("mipi_dev =  %s\n", value);
	cfg->MipiDev[index] = atoi(value);
}

static void parse_sensor_laneid(SAMPLE_INI_CFG_S *cfg, const char *value,
					CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("Lane_id =  %s\n", value);
	parse_lane_id(cfg->as16LaneId[index], value);
}

static void parse_sensor_pnswap(SAMPLE_INI_CFG_S *cfg, const char *value,
					CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("pn_swap =  %s\n", value);
	parse_pn_swap(cfg->as8PNSwap[index], value);
}

static void parse_sensor_hwsync(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("hw_sync =  %s\n", value);
	cfg->u8HwSync[index] = atoi(value);
}

static void parse_sensor_mclken(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("mclk_en =  %s\n", value);
	cfg->stMclkAttr[index].bMclkEn = atoi(value);
}

static void parse_sensor_mclk(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("mclk =  %s\n", value);
	cfg->stMclkAttr[index].u8Mclk = atoi(value);
}

static void parse_sensor_orien(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("orien =  %s\n", value);
	cfg->u8Orien[index] = atoi(value);
}

static void parse_sensor_muxdev(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("muxdev =  %s\n", value);
	cfg->u8MuxDev[index] = atoi(value);
}

static void parse_sensor_attachdev(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("attach dev =  %s\n", value);
	cfg->u8AttachDev[index] = atoi(value);
}

static void parse_sensor_switchport(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("switch port =  %c\n", 'A' + atoi(value));
	cfg->s16SwitchPort[index] = atoi(value);
}

static void parse_sensor_switchgpio(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("switch gpio =  %s\n", value);
	cfg->s16SwitchGpio[index] = atoi(value);
}

static void parse_sensor_switchpol(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("switch pol =  %s\n", value);
	cfg->s16SwitchPol[index] = atoi(value);
}

static void parse_sensor_rstport(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	if (atoi(value) == 0) {
		SAMPLE_PRT("RST port A\n");
	} else if (atoi(value) == 1) {
		SAMPLE_PRT("RST port B\n");
	} else if (atoi(value) == 2) {
		SAMPLE_PRT("RST port C\n");
	}
	cfg->s32RstPort[index] = atoi(value);
}

static void parse_sensor_rstpin(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("RST pin =  %s\n", value);
	cfg->s32RstPin[index] = atoi(value);
}

static void parse_sensor_rstpol(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("RST pol =  %s\n", value);
	cfg->s32RstPol[index] = atoi(value);
}

static void parse_sensor_fpsrate(SAMPLE_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SAMPLE_PRT("Fps rate =  %s\n", value);
	cfg->f32FpsRate[index] = atoi(value);
}
/* === Sensor section parser handler end === */
typedef CVI_VOID(*parser)(SAMPLE_INI_CFG_S *cfg, const char *value,
		CVI_U32 param0, CVI_U32 param1, CVI_U32 param2);

typedef struct _INI_HDLR_S {
	const char name[16];
	CVI_U32 param0;
	CVI_U32 param1;
	CVI_U32 param2;
	parser pfnJob;
} INI_HDLR_S;

typedef enum _INI_SOURCE_NAME_E {
	INI_SOURCE_TYPE = 0,
	INI_SOURCE_DEVNUM,
	INI_SOURCE_NUM,
} INI_SOURCE_NAME_E;

typedef enum _INI_SENSOR_NAME_E {
	INI_SENSOR_NAME = 0,
	INI_SENSOR_BUSID,
	INI_SENSOR_I2CADDR,
	INI_SENSOR_MIPIDEV,
	INI_SENSOR_LANEID,
	INI_SENSOR_PNSWAP,
	INI_SENSOR_HWSYNC,
	INI_SENSOR_MCLKEN,
	INI_SENSOR_MCLK,
	INI_SENSOR_ORIEN,
	INI_SENSOR_MUXDEV,
	INI_SENSOR_ATTACHDEV,
	INI_SENSOR_SWITCHPORT,
	INI_SENSOR_SWITCHGPIO,
	INI_SENSOR_SWITCHPOL,
	INI_SENSOR_RSTPORT,
	INI_SENSOR_RSTPIN,
	INI_SENSOR_RSTPOL,
	INI_SENSOR_FPSRATE,
	INI_SENSOR_NUM,
} INI_SENSOR_NAME_E;

INI_HDLR_S stSectionSource[INI_SOURCE_NUM] = {
	[INI_SOURCE_TYPE] = {"type", 0, 0, 0, parse_source_type},
	[INI_SOURCE_DEVNUM] = {"dev_num", 0, 0, 0, parse_source_devnum},
};

INI_HDLR_S stSectionSensor[INI_SENSOR_NUM] = {
	[INI_SENSOR_NAME] = {"name", 0, 0, 0, parse_sensor_name},
	[INI_SENSOR_BUSID] = {"bus_id", 0, 0, 0, parse_sensor_busid},
	[INI_SENSOR_I2CADDR] = {"sns_i2c_addr", 0, 0, 0, parse_sensor_i2caddr},
	[INI_SENSOR_MIPIDEV] = {"mipi_dev", 0, 0, 0, parse_sensor_mipidev},
	[INI_SENSOR_LANEID] = {"lane_id", 0, 0, 0, parse_sensor_laneid},
	[INI_SENSOR_PNSWAP] = {"pn_swap", 0, 0, 0, parse_sensor_pnswap},
	[INI_SENSOR_HWSYNC] = {"hw_sync", 0, 0, 0, parse_sensor_hwsync},
	[INI_SENSOR_MCLKEN] = {"mclk_en", 0, 0, 0, parse_sensor_mclken},
	[INI_SENSOR_MCLK] = {"mclk", 0, 0, 0, parse_sensor_mclk},
	[INI_SENSOR_ORIEN] = {"orien", 0, 0, 0, parse_sensor_orien},
	[INI_SENSOR_MUXDEV] = {"mux_dev", 0, 0, 0, parse_sensor_muxdev},
	[INI_SENSOR_ATTACHDEV] = {"attach_dev", 0, 0, 0, parse_sensor_attachdev},
	[INI_SENSOR_SWITCHPORT] = {"switch_port", 0, 0, 0, parse_sensor_switchport},
	[INI_SENSOR_SWITCHGPIO] = {"switch_gpio", 0, 0, 0, parse_sensor_switchgpio},
	[INI_SENSOR_SWITCHPOL] = {"switch_pol", 0, 0, 0, parse_sensor_switchpol},
	[INI_SENSOR_RSTPORT] = {"port", 0, 0, 0, parse_sensor_rstport},
	[INI_SENSOR_RSTPIN] = {"pin", 0, 0, 0, parse_sensor_rstpin},
	[INI_SENSOR_RSTPOL] = {"pol", 0, 0, 0, parse_sensor_rstpol},
	[INI_SENSOR_FPSRATE] = {"fps", 0, 0, 0, parse_sensor_fpsrate},
};

static int parse_handler(void *user, const char *section, const char *name, const char *value)
{
	SAMPLE_INI_CFG_S *cfg = (SAMPLE_INI_CFG_S *)user;
	const INI_HDLR_S *hdler;
	int i, size, index = 0;

	if (strcmp(section, "source") == 0) {
		hdler = stSectionSource;
		size = INI_SOURCE_NUM;
	} else if (strcmp(section, "sensor") == 0) {
		hdler = stSectionSensor;
		size = INI_SENSOR_NUM;
		index = 0;
	} else if (strcmp(section, "sensor2") == 0) {
		hdler = stSectionSensor;
		size = INI_SENSOR_NUM;
		index = 1;
	} else if (strcmp(section, "sensor3") == 0) {
		hdler = stSectionSensor;
		size = INI_SENSOR_NUM;
		index = 2;
	} else {
		/* unknown section/name */
		return 1;
	}

	if (hdler == stSectionSensor) {
		for (i = 0; i < size; i++) {
			stSectionSensor[i].param0 = index;
		}
	}

	for (i = 0; i < size; i++) {
		if (strcmp(name, hdler[i].name) == 0) {
			hdler[i].pfnJob(cfg, value, hdler[i].param0,
					hdler[i].param1, hdler[i].param2);
			break;
		}
	}

	return 1;
}

CVI_S32 SAMPLE_COMM_VI_SetIniPath(const CVI_CHAR *iniPath)
{
	int ret;

	if (iniPath == NULL) {
		SAMPLE_PRT("%s: null ptr\n", __func__);
		ret = CVI_FAILURE;
	} else if (strlen(iniPath) >= SNSCFGPATH_SIZE) {
		SAMPLE_PRT("%s: SNSCFGPATH_SIZE is too small\n", __func__);
		ret = CVI_FAILURE;
	} else {
		strncpy(g_snsCfgPath, iniPath, SNSCFGPATH_SIZE);
		ret = CVI_SUCCESS;
	}

	return ret;
}

CVI_S32 SAMPLE_COMM_VI_ParseIni(SAMPLE_INI_CFG_S *pstIniCfg)
{
	int ret;
#define INI_FILE_PATH	"/mnt/data/sensor_cfg.ini"
#define INI_DEF_PATH	"/mnt/system/usr/bin/sensor_cfg.ini"

	memcpy(pstIniCfg, &stDefIniCfg, sizeof(*pstIniCfg));
	if (g_snsCfgPath[0] != 0) {
		SAMPLE_PRT("Parse %s\n", g_snsCfgPath);
		ret = ini_parse(g_snsCfgPath, parse_handler, pstIniCfg);
		if (ret >= 0) {
			return CVI_SUCCESS;
		}
		if (ret != -1) {
			SAMPLE_PRT("Parse %s incomplete, use default cfg\n", INI_FILE_PATH);
			return CVI_FAILURE;
		}

		SAMPLE_PRT("%s Not Found\n", g_snsCfgPath);
	}
	SAMPLE_PRT("Parse %s\n", INI_FILE_PATH);
	ret = ini_parse(INI_FILE_PATH, parse_handler, pstIniCfg);
	if (ret >= 0) {
		return CVI_SUCCESS;
	}
	if (ret != -1) {
		SAMPLE_PRT("Parse %s incomplete, use default cfg\n", INI_FILE_PATH);
		return CVI_FAILURE;
	}
	SAMPLE_PRT("%s Not Found\n", INI_FILE_PATH);
	SAMPLE_PRT("Parse %s\n", INI_DEF_PATH);

	ret = ini_parse(INI_DEF_PATH, parse_handler, pstIniCfg);
	if (ret < 0) {
		if (ret == -1) {
			SAMPLE_PRT("%s not exist, use default cfg\n", INI_DEF_PATH);
		} else {
			SAMPLE_PRT("Parse %s incomplete, use default cfg\n", INI_DEF_PATH);
		}

		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

/* Helper API to fill the stViConfig according to the pstIniCfg. */
CVI_S32 SAMPLE_COMM_VI_IniToViCfg(SAMPLE_INI_CFG_S *pstIniCfg, SAMPLE_VI_CONFIG_S *pstViConfig)
{
	DYNAMIC_RANGE_E    enDynamicRange	= DYNAMIC_RANGE_SDR8;
	PIXEL_FORMAT_E	   enPixFormat		= VI_PIXEL_FORMAT;
	VIDEO_FORMAT_E	   enVideoFormat	= VIDEO_FORMAT_LINEAR;
	COMPRESS_MODE_E    enCompressMode	= COMPRESS_MODE_TILE;
	VI_VPSS_MODE_E	   enMastPipeMode	= VI_OFFLINE_VPSS_OFFLINE;
	CVI_S32 s32WorkSnsId = 0;

	if (!pstIniCfg) {
		SAMPLE_PRT("%s: null ptr\n", __func__);
		return CVI_FAILURE;
	}

	if (!pstViConfig) {
		SAMPLE_PRT("%s: null ptr\n", __func__);
		return CVI_FAILURE;
	}

	SAMPLE_COMM_VI_GetSensorInfo(pstViConfig);

	for (; s32WorkSnsId < pstIniCfg->devNum; s32WorkSnsId++) {
		pstViConfig->s32WorkingViNum					= 1 + s32WorkSnsId;
		pstViConfig->as32WorkingViId[s32WorkSnsId]			= s32WorkSnsId;
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.enSnsType	= pstIniCfg->enSnsType[s32WorkSnsId];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.MipiDev		= pstIniCfg->MipiDev[s32WorkSnsId];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.stMclkAttr.bMclkEn =
			pstIniCfg->stMclkAttr[s32WorkSnsId].bMclkEn;
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.stMclkAttr.u8Mclk =
			pstIniCfg->stMclkAttr[s32WorkSnsId].u8Mclk;
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s32BusId		= pstIniCfg->s32BusId[s32WorkSnsId];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s32SnsI2cAddr	=
			pstIniCfg->s32SnsI2cAddr[s32WorkSnsId];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.u8HwSync		= pstIniCfg->u8HwSync[s32WorkSnsId];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.u8Orien		= pstIniCfg->u8Orien[s32WorkSnsId];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.u8MuxDev		= pstIniCfg->u8MuxDev[s32WorkSnsId];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s16SwitchPort	= pstIniCfg->s16SwitchPort[s32WorkSnsId];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s16SwitchGpio	= pstIniCfg->s16SwitchGpio[s32WorkSnsId];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s16SwitchPol	= pstIniCfg->s16SwitchPol[s32WorkSnsId];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s32RstPort	= pstIniCfg->s32RstPort[s32WorkSnsId];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s32RstPin	= pstIniCfg->s32RstPin[s32WorkSnsId];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s32RstPol	= pstIniCfg->s32RstPol[s32WorkSnsId];
		if (pstIniCfg->f32FpsRate[s32WorkSnsId] != 0) {
			pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.f32FpsRate	= pstIniCfg->f32FpsRate[s32WorkSnsId];
		} else {
			pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.f32FpsRate = 25;
		}
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[0]	=
			pstIniCfg->as16LaneId[s32WorkSnsId][0];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[1]	=
			pstIniCfg->as16LaneId[s32WorkSnsId][1];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[2]	=
			pstIniCfg->as16LaneId[s32WorkSnsId][2];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[3]	=
			pstIniCfg->as16LaneId[s32WorkSnsId][3];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[4]	=
			pstIniCfg->as16LaneId[s32WorkSnsId][4];

		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[0]	=
			pstIniCfg->as8PNSwap[s32WorkSnsId][0];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[1]	=
			pstIniCfg->as8PNSwap[s32WorkSnsId][1];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[2]	=
			pstIniCfg->as8PNSwap[s32WorkSnsId][2];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[3]	=
			pstIniCfg->as8PNSwap[s32WorkSnsId][3];
		pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[4]	=
			pstIniCfg->as8PNSwap[s32WorkSnsId][4];

		if (pstIniCfg->u8MuxDev[s32WorkSnsId] && pstIniCfg->u8AttachDev[s32WorkSnsId] > 0) {
			pstViConfig->astViInfo[s32WorkSnsId].stDevInfo.ViDev		=
				pstIniCfg->u8AttachDev[s32WorkSnsId] + VI_MAX_PHY_DEV_NUM - 1;
			pstViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[0]	=
				pstIniCfg->u8AttachDev[s32WorkSnsId] + VI_MAX_PHY_DEV_NUM - 1;
		} else {
			pstViConfig->astViInfo[s32WorkSnsId].stDevInfo.ViDev		= s32WorkSnsId;
			pstViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[0]	= s32WorkSnsId;
		}

		pstViConfig->astViInfo[s32WorkSnsId].stDevInfo.enWDRMode	= pstIniCfg->enWDRMode[s32WorkSnsId];

		pstViConfig->astViInfo[s32WorkSnsId].stPipeInfo.enMastPipeMode = enMastPipeMode;

		pstViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[1]	= -1;
		pstViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[2]	= -1;
		pstViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[3]	= -1;
		pstViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[4]	= -1;
		pstViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[5]	= -1;

		pstViConfig->astViInfo[s32WorkSnsId].stChnInfo.ViChn		= s32WorkSnsId;
		pstViConfig->astViInfo[s32WorkSnsId].stChnInfo.enPixFormat	= enPixFormat;
		pstViConfig->astViInfo[s32WorkSnsId].stChnInfo.enDynamicRange	= enDynamicRange;
		pstViConfig->astViInfo[s32WorkSnsId].stChnInfo.enVideoFormat	= enVideoFormat;
		pstViConfig->astViInfo[s32WorkSnsId].stChnInfo.enCompressMode	= enCompressMode;
	}

	cur_sns_num = pstViConfig->s32WorkingViNum;

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_DefaultConfig(CVI_VOID)
{
	SAMPLE_INI_CFG_S stIniCfg = {0};
	SAMPLE_VI_CONFIG_S stViConfig;
	PIC_SIZE_E enPicSize;
	SIZE_S stSize;
	CVI_S32 s32Ret = CVI_SUCCESS;

	stIniCfg = (SAMPLE_INI_CFG_S) {
		.enSource  = VI_PIPE_FRAME_SOURCE_DEV,
		.devNum    = 1,
		.enSnsType[0] = SONY_IMX327_MIPI_2M_30FPS_12BIT,
		.enWDRMode[0] = WDR_MODE_NONE,
		.s32BusId[0]  = 3,
		.s32SnsI2cAddr[0] = -1,
		.MipiDev[0]   = 0xFF,
		.u8UseMultiSns = 0,
	};

	// Get config from ini if found.
	s32Ret = SAMPLE_COMM_VI_ParseIni(&stIniCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Parse fail\n");
	} else {
		SAMPLE_PRT("Parse complete\n");
	}

	/************************************************
	 * step1:  Config VI
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	/************************************************
	 * step2:  Get input size
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[0], &enPicSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init modules
	 ************************************************/
	s32Ret = SAMPLE_PLAT_SYS_INIT(stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "sys init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (stIniCfg.enSource == VI_PIPE_FRAME_SOURCE_DEV) {
		s32Ret = SAMPLE_PLAT_VI_INIT(&stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_CHAR *SAMPLE_COMM_VI_GetSnsrTypeName(void)
{
	return (CVI_CHAR *)snsr_type_name;
}

