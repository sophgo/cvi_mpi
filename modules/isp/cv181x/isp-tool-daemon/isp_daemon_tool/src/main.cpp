/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: main.cpp
 * Description:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <csignal>
#include <syslog.h>
#include <unistd.h>

#ifndef CONFIG_FAST_BOOT_MODE
#include "cvi_streamer.hpp"
#endif

#include "isp_tool_daemon_comm.hpp"

extern "C" {
#include "cvi_type.h"
#include "cvi_sys.h"
#include "cvi_bin.h"
#include "cvi_isp.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "3A_internal.h"
#include "cvi_ispd2.h"
#include "raw_dump.h"
#include "cvi_venc.h"
#include "cvi_msg_client.h"
#include "raw_dump_internal.h"
#include <string.h>
}

#ifndef CONFIG_FAST_BOOT_MODE
using std::shared_ptr;
using cvitek::system::CVIStreamer;
#endif

static CVI_U32 gLoopRun;
const char *gpisRtspEnabl;
static void signalHandler(int iSigNo)
{
	switch (iSigNo) {
		case SIGINT:
		case SIGTERM:
			gLoopRun = 0;
			break;

		default:
			break;
	}
}

static void initSignal()
{
	struct sigaction sigaInst;

	sigaInst.sa_flags = 0;
	sigemptyset(&sigaInst.sa_mask);
	sigaddset(&sigaInst.sa_mask, SIGINT);
	sigaddset(&sigaInst.sa_mask, SIGTERM);

	sigaInst.sa_handler = signalHandler;
	sigaction(SIGINT, &sigaInst, NULL);
	sigaction(SIGTERM, &sigaInst, NULL);
}

static int getRTSPMode()
{
	const char *input = getenv("CVI_RTSP_MODE");

	int single_output = 1;		// default is single output (for internal application)

	if (input) {
		int rtspMode = atoi(input);
		if (rtspMode != 0) {
			single_output = 0;
		}
	}

	return single_output;
}

//#define ENABLE_IPC_TEST 1

#ifdef ENABLE_IPC_TEST

static uint32_t getTimeDiffus(const struct timeval *tv1, const struct timeval *tv2)
{
	return ((tv2->tv_sec - tv1->tv_sec) * 1000000 + (tv2->tv_usec - tv1->tv_usec));
}

static void dumpLogTest(void)
{
	CVI_U32 logBufSize = 0;
	CVI_U8 *pbuf;
	FILE *fp;

	CVI_ISP_GetAELogBufSize(0, &logBufSize);
	pbuf = (CVI_U8 *) malloc(logBufSize);
	CVI_ISP_GetAELogBuf(0, pbuf, logBufSize);
	fp = fopen("aelog0.txt", "w");
	fwrite(pbuf, logBufSize, 1, fp);
	fclose(fp);
	free(pbuf);

	printf("write aelog0.txt, %d\n", logBufSize);

	sleep(2);

	CVI_ISP_GetAELogBufSize(0, &logBufSize);
	pbuf = (CVI_U8 *) malloc(logBufSize);
	CVI_ISP_GetAELogBuf(0, pbuf, logBufSize);
	fp = fopen("aelog1.txt", "w");
	fwrite(pbuf, logBufSize, 1, fp);
	fclose(fp);
	free(pbuf);

	printf("write aelog1.txt, %d\n", logBufSize);

	CVI_ISP_GetAEBinBufSize(0, &logBufSize);
	pbuf = (CVI_U8 *) malloc(logBufSize);
	CVI_ISP_GetAEBinBuf(0, pbuf, logBufSize);
	fp = fopen("aedbg.bin", "w");
	fwrite(pbuf, logBufSize, 1, fp);
	fclose(fp);
	free(pbuf);

	printf("write aedbg.bin, %d\n", logBufSize);

	logBufSize = AWB_SNAP_LOG_BUFF_SIZE;
	pbuf = (CVI_U8 *) malloc(logBufSize);
	CVI_ISP_GetAWBSnapLogBuf(0, pbuf, logBufSize);
	fp = fopen("awblog.txt", "w");
	fwrite(pbuf, logBufSize, 1, fp);
	fclose(fp);
	free(pbuf);

	printf("write awblog.txt, %d\n", logBufSize);

	logBufSize = CVI_ISP_GetAWBDbgBinSize();
	pbuf = (CVI_U8 *) malloc(logBufSize);
	CVI_ISP_GetAWBDbgBinBuf(0, pbuf, logBufSize);
	fp = fopen("awbdbg.bin", "w");
	fwrite(pbuf, logBufSize, 1, fp);
	fclose(fp);
	free(pbuf);

	printf("write awbdbg.bin, %d\n", logBufSize);
}

static void runIpcTest(void)
{
	struct timeval tv1, tv2;

	for (uint32_t i = 0; i < 2; i++) {
		gettimeofday(&tv1, NULL);
		CVI_ISP_GetVDTimeOut(0, ISP_VD_BE_END, 200);
		gettimeofday(&tv2, NULL);
		printf("getvdtimeout: %d\n", getTimeDiffus(&tv1, &tv2));
	}

	dumpLogTest();

	ISP_SMART_INFO_S stSmartInfo;

	gettimeofday(&tv1, NULL);
	CVI_ISP_GetSmartInfo(0, &stSmartInfo);
	gettimeofday(&tv2, NULL);
	printf("getsmartinfo: %d\n", getTimeDiffus(&tv1, &tv2));

	gettimeofday(&tv1, NULL);
	CVI_ISP_SetSmartInfo(0, &stSmartInfo, 60);
	gettimeofday(&tv2, NULL);
	printf("setsmartinfo: %d\n", getTimeDiffus(&tv1, &tv2));

	{
		CVI_U32 frameID;

		CVI_ISP_GetFrameID(0, &frameID);
		printf("get frameid: %d\n", frameID);
	}

	{
		CVI_FLOAT fps;

		CVI_ISP_QueryFps(0, &fps);
		printf("get fps: %f\n", fps);
	}

	{
		CVI_S16 s16Lv;

		CVI_ISP_GetCurrentLvX100(0, &s16Lv);
		printf("get lv: %d\n", s16Lv);
	}
}

static void pqbinTest(void)
{
	struct timeval tv1, tv2;

	CVI_U32 u32BinLen = CVI_BIN_GetBinTotalLen();
	CVI_U8 *pu8Buffer = (CVI_U8 *)  malloc(sizeof(CVI_U8) * u32BinLen);

	printf("run pqbinTest %p,%d\n", pu8Buffer, u32BinLen);

	gettimeofday(&tv1, NULL);
	CVI_S32 s32Ret = CVI_BIN_ExportBinData(pu8Buffer, u32BinLen);
	gettimeofday(&tv2, NULL);
	printf("%d, %d\n", s32Ret, getTimeDiffus(&tv1, &tv2));

	gettimeofday(&tv1, NULL);
	s32Ret = CVI_BIN_ImportBinData(pu8Buffer, u32BinLen);
	gettimeofday(&tv2, NULL);
	printf("%d, %d\n", s32Ret, getTimeDiffus(&tv1, &tv2));
}
#endif



#define UNUSED_VARIABLE(x) ((void)(x))
#define APP_CHECK_RET(actual, fmt, arg...)                                   \
	do {																		 \
		if ((actual) != 0) {													 \
			printf("[%d]:%s():ret=%d \n" fmt, __LINE__, __func__, actual, ## arg); \
		}																		 \
	} while (0)

#ifdef CONFIG_FAST_BOOT_MODE
#include "rtsp.h"

#define __VENC_CHN 0
#define RTSP_MAX_LIVE 2

static CVI_RTSP_CTX *ctx = NULL;
static CVI_RTSP_SESSION *pSession[RTSP_MAX_LIVE] = {NULL};
static int rtsp_init_flag = 0;
static int rtsp_session_init_flag[RTSP_MAX_LIVE] = {0};

static CVI_VOID rtsp_connect(const char *ip, CVI_VOID *arg)
{
	UNUSED_VARIABLE(arg);
	printf("rtsp connect: %s\n", ip);
}

static CVI_VOID disconnect(const char *ip, CVI_VOID *arg)
{
	UNUSED_VARIABLE(arg);
	printf("rtsp disconnect: %s\n", ip);
}

CVI_S32 SendToRtsp(int live, VENC_STREAM_S *pstStream)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_PACK_S *ppack;
	CVI_RTSP_DATA data;

	if (0 == rtsp_init_flag || 0 == rtsp_session_init_flag[live]) {
		return -1;
	}

	memset(&data, 0, sizeof(CVI_RTSP_DATA));

	data.blockCnt = pstStream->u32PackCount;
	for (CVI_U32 i = 0; i < pstStream->u32PackCount; i++) {
		ppack = &pstStream->pstPack[i];
		data.dataPtr[i] = ppack->pu8Addr + ppack->u32Offset;
		data.dataLen[i] = ppack->u32Len - ppack->u32Offset;
	}

	s32Ret = CVI_RTSP_WriteFrame(ctx, pSession[live]->video, &data);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RTSP_WriteFrame failed\n");
		return -1;
	}

	return s32Ret;
}

CVI_VOID rtsp_setup(int live, int type)
{
	CVI_RTSP_SESSION_ATTR attr;
	CVI_RTSP_CONFIG config;

	memset(&attr, 0, sizeof(CVI_RTSP_SESSION_ATTR));
	memset(&config, 0,sizeof(CVI_RTSP_CONFIG));

	if (live >= RTSP_MAX_LIVE) {
		printf("live too max:%d \n", live);
	}

	if (type != RTSP_VIDEO_H264 && type != RTSP_VIDEO_H265 && type != RTSP_VIDEO_JPEG) {
		printf("type:%d not support \n", type);
	}

	if (0 == rtsp_init_flag) {
		config.port = 8554;
		if (CVI_RTSP_Create(&ctx, &config) < 0) {
			printf("fail to create rtsp\n");
		}
	}
	rtsp_init_flag = 1;

	attr.video.codec = (CVI_RTSP_VIDEO_CODEC)type;
	sprintf(attr.name, "stream%d", live);

	int s32Ret = CVI_RTSP_CreateSession(ctx, &attr, &pSession[live]);
	if (s32Ret != 0) {
		printf("CVI_RTSP_CreateSession failed with %#x\n", s32Ret);
	}

	CVI_RTSP_STATE_LISTENER listener;

	memset(&listener, 0, sizeof(CVI_RTSP_STATE_LISTENER));

	listener.onConnect = rtsp_connect;
	listener.argConn = ctx;
	listener.onDisconnect = disconnect;
	CVI_RTSP_SetListener(ctx, &listener);

	if (CVI_RTSP_Start(ctx) < 0) {
		printf("fail to rtsp start\n");
	}

	rtsp_session_init_flag[live] = 1;
}

void* run_rtsp_thread(void *pArgs)
{
	VENC_RECV_PIC_PARAM_S stRecvParam;
	VENC_CHN_STATUS_S stStatus;
	VENC_STREAM_S stStream;
	VENC_CHN_ATTR_S stChnAttr;
	CVI_S32 s32Ret;
	VENC_CHN VeChn = *(int*)pArgs;

	memset(&stStatus, 0, sizeof(VENC_CHN_STATUS_S));
	memset(&stChnAttr, 0, sizeof(VENC_CHN_ATTR_S));

	APP_CHECK_RET(CVI_VENC_GetChnAttr(VeChn, &stChnAttr), "CVI_VENC_GetChnAttr");

	printf("type:%d, chn:%d\n", stChnAttr.stVencAttr.enType, VeChn);

	switch(stChnAttr.stVencAttr.enType) {
		case PT_H264:
			rtsp_setup(VeChn, RTSP_VIDEO_H264);
		break;

		case PT_H265:
			rtsp_setup(VeChn, RTSP_VIDEO_H265);
		break;

		default:
			printf("type:%d not support\n",stChnAttr.stVencAttr.enType);
			return NULL;
		break;
	}

	stRecvParam.s32RecvPicNum = -1;
	APP_CHECK_RET(CVI_VENC_StartRecvFrame(VeChn, &stRecvParam), "CVI_VENC_StartRecvFrame");

	while (gLoopRun) {
		s32Ret = CVI_VENC_QueryStatus(VeChn, &stStatus);
		if(s32Ret != CVI_SUCCESS) {
			sleep(1);
			continue;
		}
		stStream.pstPack = (VENC_PACK_S *) malloc(sizeof(VENC_PACK_S) * stStatus.u32CurPacks);
		if (!stStream.pstPack) {
			printf("malloc pack fail \n");
			return NULL;
		}
RETRY:
		s32Ret = CVI_VENC_GetStream(VeChn, &stStream, 2000);
		if (s32Ret == CVI_ERR_VENC_BUSY || s32Ret == CVI_ERR_VENC_NOBUF)  {
			usleep(1000 * 40);
			goto RETRY;
		}
		else if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VENC_GetStream fail, ret:%x \n", s32Ret);
			free(stStream.pstPack);
			stStream.pstPack = NULL;
			return NULL;
		}

		SendToRtsp(VeChn, &stStream);

		APP_CHECK_RET(CVI_VENC_ReleaseStream(VeChn, &stStream), "CVI_VENC_ReleaseStream");
		if (stStream.pstPack != NULL) {
			free(stStream.pstPack);
			stStream.pstPack = NULL;
		}
	}
	APP_CHECK_RET(CVI_VENC_StopRecvFrame(VeChn), "CVI_VENC_StopRecvFrame chn =%d \n", VeChn);
	return NULL;
}

#endif

#define JSONRPC_PORT	(5566)

#define ENV_RAW_DUMP_SAVE_PATH  "RAW_DUMP_SAVE_PATH"
#define ENV_RAW_DUMP_PARAM      "RAW_DUMP_PARAM"
#define ENV_RAW_DUMP_REPEAT     "RAW_DUMP_REPEAT"

static int gRawDumpRepeat;
static RAW_DUMP_INFO_S gstRawDumpInfo;
static bool init_raw_dump(void);

#define MAX_RAW_DUMP_PATH_LEN 128
static char gRawDumpPath[MAX_RAW_DUMP_PATH_LEN];
static char *get_raw_dump_path(void);
static CVI_BOOL bauto_dump_raw_run = CVI_FALSE;

static void *auto_dump_raw_thread(void *arg)
{
	bauto_dump_raw_run = CVI_TRUE;
	while (bauto_dump_raw_run) {
		if (gRawDumpRepeat > 0 || gRawDumpRepeat == -1) {
			gstRawDumpInfo.pathPrefix = get_raw_dump_path();
			cvi_raw_dump(0, &gstRawDumpInfo);
			if (gRawDumpRepeat > 0) {
				gRawDumpRepeat--;
			}
		}
		sleep(1);
	}
	bauto_dump_raw_run = CVI_FALSE;
	UNUSED_VARIABLE(arg);
	return 0;
}

int main(void)
{
	CVI_S32 s32Ret;
	pthread_t pthread_id;
	openlog("ISP Tool Daemon", 0, LOG_USER);

	if (CVI_MSG_Init()) {
		printf("CVI_MSG_Init fail\n");
		return -1;
	}

#ifndef CONFIG_FAST_BOOT_MODE
	shared_ptr<CVIStreamer> cvi_streamer =
		make_shared<CVIStreamer>();
#endif

	//sample_common_platform.c will sigaction _SAMPLE_PLAT_SYS_HandleSig
	//we substitute signalHander for _SAMPLE_PLAT_SYS_HandleSig
	initSignal();

	isp_daemon2_init(JSONRPC_PORT);
	cvi_raw_dump_init();

	int single_output = getRTSPMode();
	isp_daemon2_enable_device_bind_control(single_output ? 1 : 0);

#ifdef CONFIG_FAST_BOOT_MODE
	CVI_SYS_Init();

	CVI_ISP_Init(0);

	if (!single_output) {
		CVI_ISP_Init(1);
	}
#endif

	bool bEnableRawDump = init_raw_dump();
	if (bEnableRawDump) {
		s32Ret = pthread_create(&pthread_id, NULL, auto_dump_raw_thread, NULL);
		if (s32Ret != 0) {
			printf("pthread_create error(%d)\n", s32Ret);
			return -1;
		}
		pthread_detach(pthread_id);
	}

#ifdef ENABLE_IPC_TEST
	runIpcTest();
	pqbinTest();
#endif

	gLoopRun = 1;

#ifdef CONFIG_FAST_BOOT_MODE
	pthread_t rtsp_pthread_id[2];
	int videoChn = 0, videoMaxChn = 1;
	if (!single_output) {
		videoMaxChn = RTSP_MAX_LIVE;
	}
	gpisRtspEnabl = getenv("CVI_RTSP_ENABLE");
	if (gpisRtspEnabl || !single_output) {
		for (videoChn = 0; videoChn < videoMaxChn; videoChn++) {
			s32Ret = pthread_create(&rtsp_pthread_id[videoChn], NULL, run_rtsp_thread, &videoChn);
			if (s32Ret != 0) {
				printf("pthread_create error(%d)\n", s32Ret);
				return -1;
			}
			usleep(100 * 1000);
		}
	}
#endif

	while (gLoopRun) {
		sleep(2);
	}

#ifdef CONFIG_FAST_BOOT_MODE
	for (videoChn = 0; videoChn < videoMaxChn; videoChn++) {
		pthread_join(rtsp_pthread_id[videoChn], NULL);
	}
	bauto_dump_raw_run = CVI_FALSE;
#endif

	cvi_raw_dump_uninit();

	isp_daemon2_uninit();
	// CVI_SYS_Exit();
#ifdef CONFIG_FAST_BOOT_MODE
	CVI_MSG_Deinit();
#endif

	closelog();
	return 0;
}

static char *get_raw_dump_path(void)
{
	struct tm *t;
	time_t tt;
	char cmd[MAX_RAW_DUMP_PATH_LEN + 32];
	const char *path = getenv(ENV_RAW_DUMP_SAVE_PATH);

	time(&tt);
	t = localtime(&tt);

	snprintf(gRawDumpPath, MAX_RAW_DUMP_PATH_LEN, "%s/%04d%02d%02d%02d%02d%02d",
		path,
		t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec);


	snprintf(cmd, (MAX_RAW_DUMP_PATH_LEN + 32), "mkdir %s;sync", gRawDumpPath);
	system(cmd);

	return gRawDumpPath;
}

static bool init_raw_dump(void)
{
	const char *path = getenv(ENV_RAW_DUMP_SAVE_PATH);
	const char *param = getenv(ENV_RAW_DUMP_PARAM);
	const char *repeat = getenv(ENV_RAW_DUMP_REPEAT);

	if (path == NULL) {
		return false;
	}

	if (param == NULL || repeat == NULL) {
		printf("run raw dump fail, parameter missing...\n");
		return false;
	}

	printf("init raw dump, %s %s %s\n", path, param, repeat);

	memset(&gstRawDumpInfo, 0, sizeof(gstRawDumpInfo));

	if (access(path, F_OK) == 0) {
		gstRawDumpInfo.pathPrefix = (char *) path;
	} else {
		printf("%s not exist...\n", path);
		return false;
	}

	sscanf(param, "%d,%d,%d,%d,%d",
		&gstRawDumpInfo.u32TotalFrameCnt,
		&gstRawDumpInfo.stRoiRect.s32X,
		&gstRawDumpInfo.stRoiRect.s32Y,
		&gstRawDumpInfo.stRoiRect.u32Width,
		&gstRawDumpInfo.stRoiRect.u32Height);

	printf("raw dump count: %d, roi: %d,%d,%d,%d\n",
		gstRawDumpInfo.u32TotalFrameCnt,
		gstRawDumpInfo.stRoiRect.s32X,
		gstRawDumpInfo.stRoiRect.s32Y,
		gstRawDumpInfo.stRoiRect.u32Width,
		gstRawDumpInfo.stRoiRect.u32Height);

	gRawDumpRepeat = atoi(repeat);

	return true;
}
