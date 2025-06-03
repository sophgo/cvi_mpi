/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample/common/sample_common_isp.c
 * Description:
 *   Common ctrl code for isp.
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/time.h>

#include <cvi_defines.h>
#include "sample_comm.h"
#include "cvi_awb.h"
#include "cvi_af.h"

#include "cvi_sns_ctrl.h"
#include "cvi_ae.h"
#include "cvi_isp.h"

#ifdef SUPPORT_ISP_PQTOOL
#include <dlfcn.h>
static CVI_BOOL g_ISPDaemon = CVI_FALSE;
static void *g_ISPDHandle;
#define ISPD_LIBNAME "libcvi_ispd2.so"
#define ISPD_CONNECT_PORT 5566
#endif //

static pthread_t g_IspPid[VI_MAX_DEV_NUM];
static CVI_U32 g_au32IspSnsId[VI_MAX_DEV_NUM] = { 0 };

CVI_SNS_TYPE_E g_enSnsType[VI_MAX_DEV_NUM] = {
	SONY_IMX327_MIPI_2M_30FPS_12BIT
};

static ISP_INIT_ATTR_S gstInitAttr[VI_MAX_DEV_NUM];

 /* snsr_size, action_size, fps, bayer_format, wdr_mode, snsr_mode, data_lane_num, master_or_slave_mode */
ISP_PUB_ATTR_S ISP_PUB_ATTR_SAMPLE =
						{ { 0, 0, 1920, 1080 }, { 1920, 1080 }, 30, BAYER_RGGB, WDR_MODE_NONE, 0, 4, 2};

void callback_FPS(int fps)
{
	static CVI_FLOAT uMaxFPS[VI_MAX_DEV_NUM] = {0};
	int i;

	for (i = 0; i < VI_MAX_DEV_NUM && g_IspPid[i]; i++) {
		ISP_PUB_ATTR_S pubAttr = {0};

		CVI_ISP_GetPubAttr(i, &pubAttr);
		if (uMaxFPS[i] == 0) {
			uMaxFPS[i] = pubAttr.f32FrameRate;
		}
		if (fps == 0) {
			pubAttr.f32FrameRate = uMaxFPS[i];
		} else {
			pubAttr.f32FrameRate = (CVI_FLOAT) fps;
		}
		CVI_ISP_SetPubAttr(i, &pubAttr);
	}
}

static CVI_VOID *SAMPLE_COMM_ISP_Thread(void *arg)
{
	CVI_S32 s32Ret = 0;
	CVI_U8 IspDev = *(CVI_U8 *)arg;
	char szThreadName[20];

	free(arg);
	snprintf(szThreadName, sizeof(szThreadName), "ISP%d_RUN", IspDev);
	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);

	if (IspDev > 0) {
		SAMPLE_PRT("ISP Dev %d return\n", IspDev);
		return NULL;
	}

	// TODO: HGJ
	//CVI_SYS_RegisterThermalCallback(callback_FPS);

	SAMPLE_PRT("ISP Dev %d running!\n", IspDev);
	s32Ret = CVI_ISP_Run(IspDev);
	if (s32Ret != 0)
		SAMPLE_PRT("CVI_ISP_Run failed with %#x!\n", s32Ret);

	return NULL;
}

CVI_S32 SAMPLE_COMM_ISP_Run(CVI_U8 IspDev)
{
	CVI_S32 s32Ret = 0;
	CVI_U8 *arg = malloc(sizeof(*arg));
	struct sched_param param;
	pthread_attr_t attr;

	if (arg == NULL) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "malloc failed\n");
		goto out;
	}

	*arg = IspDev;
	param.sched_priority = 80;

	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	s32Ret = pthread_create(&g_IspPid[IspDev], &attr, SAMPLE_COMM_ISP_Thread, arg);
	if (s32Ret != 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "create isp running thread failed!, error: %d, %s\r\n",
					s32Ret, strerror(s32Ret));
		goto out;
	}

#ifdef SUPPORT_ISP_PQTOOL
	if (!g_ISPDaemon) {
		g_ISPDHandle = dlopen(ISPD_LIBNAME, RTLD_NOW);

		if (g_ISPDHandle) {
			char *error = NULL;
			void (*daemon_init)(unsigned int port);

			printf("Load dynamic library %s success\n", ISPD_LIBNAME);

			dlerror();
			daemon_init = dlsym(g_ISPDHandle, "isp_daemon2_init");
			error = dlerror();
			if (error == NULL) {
				(*daemon_init)(ISPD_CONNECT_PORT);
				g_ISPDaemon = CVI_TRUE;
			} else {
				printf("Run daemon initial fail\n");
				dlclose(g_ISPDHandle);
			}
		} else {
			printf("Load dynamic library %s fail\n", ISPD_LIBNAME);
		}
	}
#endif //

out:

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_ISP_GetIspPubAttr(ISP_DEV IspDev, SAMPLE_VI_CONFIG_S *pstViConfig, ISP_PUB_ATTR_S *pstPubAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S stSize;
	CVI_S32 dev_num = IspDev;

	memcpy(pstPubAttr, &ISP_PUB_ATTR_SAMPLE, sizeof(ISP_PUB_ATTR_S));

	stSize.u32Width = pstViConfig->stSnsCfg.u32ImageWigth[dev_num];
	stSize.u32Height = pstViConfig->stSnsCfg.u32ImageHeight[dev_num];

	pstPubAttr->stSnsSize.u32Width = stSize.u32Width;
	pstPubAttr->stSnsSize.u32Height = stSize.u32Height;
	pstPubAttr->stWndRect.u32Width = stSize.u32Width;
	pstPubAttr->stWndRect.u32Height = stSize.u32Height;

	pstPubAttr->enWDRMode = pstViConfig->stSnsCfg.enWDRMode[dev_num];
	pstPubAttr->enBayer = (ISP_BAYER_FORMAT_E)pstViConfig->stSnsCfg.enBayerFormat[dev_num];
	pstPubAttr->f32FrameRate = pstViConfig->stSnsCfg.f32FrameRate[dev_num];
	pstPubAttr->u8LaneNum = pstViConfig->stSnsCfg.u8LaneNumber[dev_num];
	pstPubAttr->u8EnableMaster = pstViConfig->stSnsCfg.u8EnMasterMode[dev_num];

	return s32Ret;
}

/******************************************************************************
 * funciton : stop ISP, and stop isp thread
 ******************************************************************************/
CVI_VOID SAMPLE_COMM_ISP_Stop(CVI_U8 IspDev)
{
	CVI_S32 s32Ret = CVI_FAILURE;
#ifdef SUPPORT_ISP_PQTOOL
	if (g_ISPDaemon) {
		char *error = NULL;
		void (*daemon_uninit)(void);

		daemon_uninit = dlsym(g_ISPDHandle, "isp_daemon2_uninit");
		error = dlerror();
		if (error == NULL)
			(*daemon_uninit)();

		dlclose(g_ISPDHandle);
		g_ISPDHandle = NULL;
		g_ISPDaemon = CVI_FALSE;
	}
#endif //

	if (g_IspPid[IspDev]) {
		s32Ret = CVI_ISP_Exit(IspDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_ISP_Exit fail with %#x!\n", s32Ret);
			return;
		}
		pthread_join(g_IspPid[IspDev], NULL);
		g_IspPid[IspDev] = 0;
		SAMPLE_COMM_ISP_Sensor_UnRegiter_callback(IspDev);
		SAMPLE_COMM_ISP_Aelib_UnCallback(IspDev);
		SAMPLE_COMM_ISP_Awblib_UnCallback(IspDev);
		#if ENABLE_AF_LIB
		SAMPLE_COMM_ISP_Aflib_UnCallback(IspDev);
		#endif
	}
}

CVI_VOID SAMPLE_COMM_All_ISP_Stop(CVI_VOID)
{
	for (ISP_DEV IspDev = 0; IspDev < VI_MAX_DEV_NUM; IspDev++)
		SAMPLE_COMM_ISP_Stop(IspDev);
}

CVI_S32 SAMPLE_COMM_ISP_Awblib_Callback(ISP_DEV IspDev)
{
	ALG_LIB_S stAwbLib;
	CVI_S32 s32Ret = 0;

	stAwbLib.s32Id = IspDev;
	strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
	s32Ret = CVI_AWB_Register(IspDev, &stAwbLib);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "AWB Algo register failed!, error: %d\n",	s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_Awblib_UnCallback(ISP_DEV IspDev)
{
	CVI_S32 s32Ret = 0;
	ALG_LIB_S stAwbLib;

	stAwbLib.s32Id = IspDev;
	strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
	s32Ret = CVI_AWB_UnRegister(IspDev, &stAwbLib);
	if (s32Ret) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "AWB Algo unRegister failed!, error: %d\n",	s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_Aelib_Callback(ISP_DEV IspDev)
{
	CVI_S32 s32Ret = 0;
	ALG_LIB_S stAeLib;

	stAeLib.s32Id = IspDev;
	strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
	s32Ret = CVI_AE_Register(IspDev, &stAeLib);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "AE Algo register failed!, error: %d\n",	s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_Aelib_UnCallback(ISP_DEV IspDev)
{
	CVI_S32 s32Ret = 0;
	ALG_LIB_S stAeLib;

	stAeLib.s32Id = IspDev;
	strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
	s32Ret = CVI_AE_UnRegister(IspDev, &stAeLib);
	if (s32Ret) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "AE Algo unRegister failed!, error: %d\n",	s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_Aflib_Callback(ISP_DEV IspDev)
{
	ALG_LIB_S stAfLib;
	CVI_S32 s32Ret = 0;

	stAfLib.s32Id = IspDev;
	strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, sizeof(stAfLib.acLibName));
	s32Ret = CVI_AF_Register(IspDev, &stAfLib);

	if (s32Ret != CVI_SUCCESS) {
		printf("AF Algo register failed!, error: %d\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_Aflib_UnCallback(ISP_DEV IspDev)
{
	CVI_S32 s32Ret = 0;
	ALG_LIB_S stAfLib;

	stAfLib.s32Id = IspDev;
	strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, sizeof(stAfLib.acLibName));
	s32Ret = CVI_AF_UnRegister(IspDev, &stAfLib);
	if (s32Ret) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "AF Algo unRegister failed!, error: %d\n",	s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_SetSnsObj(CVI_U32 u32SnsId, CVI_SNS_TYPE_E enSnsType)
{
	if (u32SnsId >= ARRAY_SIZE(g_enSnsType))
		return CVI_FAILURE;

	g_enSnsType[u32SnsId] = enSnsType;
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_SetSnsInit(CVI_U32 u32SnsId, CVI_U8 u8HwSync)
{
	if (u32SnsId >= ARRAY_SIZE(g_enSnsType))
		return CVI_FAILURE;

	gstInitAttr[u32SnsId].u16UseHwSync = u8HwSync;

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_PatchSnsObj(CVI_U32 u32SnsId, SAMPLE_SENSOR_INFO_S *pstSnsInfo)
{
	RX_INIT_ATTR_S stRxInitAttr;
	unsigned int i;

	memset(&stRxInitAttr, 0, sizeof(RX_INIT_ATTR_S));

	stRxInitAttr.MipiDev = pstSnsInfo->MipiDev;
	if (pstSnsInfo->bHsettlen) {
		stRxInitAttr.hsettlen = CVI_TRUE;
		stRxInitAttr.hsettle = pstSnsInfo->u8Hsettle;
	}
	if (pstSnsInfo->stMclkAttr.bMclkEn) {
		stRxInitAttr.stMclkAttr.bMclkEn = CVI_TRUE;
		stRxInitAttr.stMclkAttr.u8Mclk  = pstSnsInfo->stMclkAttr.u8Mclk;
	}

	stRxInitAttr.MipiMode = pstSnsInfo->s32ModeId;

	for (i = 0; i < sizeof(stRxInitAttr.as16LaneId)/sizeof(CVI_S16); i++) {
		stRxInitAttr.as16LaneId[i] = pstSnsInfo->as16LaneId[i];
	}
	for (i = 0; i < sizeof(stRxInitAttr.as16FuncId)/sizeof(CVI_S16); i++) {
		stRxInitAttr.as16FuncId[i] = pstSnsInfo->as16FuncId[i];
	}
	for (i = 0; i < sizeof(stRxInitAttr.as8PNSwap)/sizeof(CVI_S8); i++) {
		stRxInitAttr.as8PNSwap[i] = pstSnsInfo->as8PNSwap[i];
	}

	return CVI_SNS_SetSnsRxAttr(u32SnsId, &stRxInitAttr);
}

CVI_S32 SAMPLE_COMM_ISP_SetSensorMode(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = CVI_SUCCESS, i;
	CVI_U32 u32SnsId;
	WDR_MODE_E wdrMode;
	ISP_PUB_ATTR_S stPubAttr;
	ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsrMode;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		wdrMode = pstViInfo->stDevInfo.enWDRMode;
		u32SnsId = pstViInfo->stSnsInfo.s32SnsId;

		if (SAMPLE_COMM_ISP_GetIspPubAttr(i, pstViConfig, &stPubAttr) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "Can't get isp pub attr!\n");
			return s32Ret;
		}
		stSnsrMode.u16Width = stPubAttr.stSnsSize.u32Width;
		stSnsrMode.u16Height = stPubAttr.stSnsSize.u32Height;
		stSnsrMode.f32Fps = stPubAttr.f32FrameRate;
		stSnsrMode.u8LaneNum = stPubAttr.u8LaneNum;
		stSnsrMode.u8EnableMaster = stPubAttr.u8EnableMaster;
		printf("sensor_type %d, lane_num %d, master mode, %d\n", pstViInfo->stSnsInfo.enSnsType,
				stSnsrMode.u8LaneNum, stSnsrMode.u8EnableMaster);
		printf("stSnsrMode.u16Width %d stSnsrMode.u16Height %d %f wdrMode %d \n",
				stSnsrMode.u16Width, stSnsrMode.u16Height, stSnsrMode.f32Fps, wdrMode);

		s32Ret = CVI_SNS_SetSnsImgMode(u32SnsId, &stSnsrMode);
		if (s32Ret!= CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sensor set image mode failed!\n");
			return s32Ret;
		}

		s32Ret = CVI_SNS_SetSnsWdrMode(u32SnsId, wdrMode);
		if (s32Ret!= CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sensor set wdr mode failed!\n");
			return s32Ret;
		}

	}
	return s32Ret;
}

CVI_S32 SAMPLE_COMM_ISP_Sensor_Regiter_callback(ISP_DEV IspDev, CVI_U32 u32SnsId, CVI_S32 s32BusId,
						CVI_S32 s32I2cAddr)
{
	UNUSED(s32BusId);
	UNUSED(s32I2cAddr);
	CVI_S32 s32Ret = -1;

	s32Ret = CVI_SNS_RegCallback(u32SnsId, IspDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SNS_RegCallback error id: %d s32Ret %d\n", IspDev, s32Ret);
	}

	g_au32IspSnsId[IspDev] = u32SnsId;

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_Sensor_UnRegiter_callback(ISP_DEV IspDev)
{
	CVI_U32 u32SnsId;
	CVI_S32 s32Ret = -1;

	u32SnsId = g_au32IspSnsId[IspDev];

	if (u32SnsId > VI_MAX_DEV_NUM) {
		SAMPLE_PRT("%s: invalid sensor id: %d\n", __func__, u32SnsId);
		return CVI_FAILURE;
	}

	s32Ret = CVI_SNS_UnRegCallback(u32SnsId, IspDev);
	if (s32Ret!= CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SNS_UnRegCallback error id: %d s32Ret %d\n", IspDev, s32Ret);
	}

	return CVI_SUCCESS;
}
