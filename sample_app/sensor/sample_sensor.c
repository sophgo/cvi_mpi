#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/param.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <inttypes.h>

#include <fcntl.h>		/* low-level i/o */

#include "cvi_sns_ctrl.h"
#include "sensor_cfg.h"
#include "cvi_comm_cif.h"
#include "cvi_comm_sns.h"
#include "cvi_mipi.h"
#include "cvi_sys.h"
#include "cvi_vi.h"
#include "cvi_vb.h"
#include "cvi_vb.h"
#include "cvi_math.h"
#include "cvi_buffer.h"
#include "cvi_sensor.h"
#include "cvi_isp.h"
#include "cvi_ae.h"


#include "ae_test.h"
#include "sample_sensor.h"
#include <signal.h>

#ifdef SUPPORT_ISP_PQTOOL
#include <dlfcn.h>
static CVI_BOOL g_ISPDaemon = CVI_FALSE;
static void *g_ISPDHandle;
#define ISPD_LIBNAME "libcvi_ispd2.so"
#define ISPD_CONNECT_PORT 5566
#endif //

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

static TEST_VI_CONFIG_S	g_stViConfig;
SENSOR_CFG_S g_stSensorCfg;
static CVI_U32 g_au32IspSnsId[VI_MAX_DEV_NUM] = { 0 };
// static CVI_BOOL g_ISPDaemon = CVI_FALSE;
static pthread_t g_IspPid[VI_MAX_PIPE_NUM];

ISP_PUB_ATTR_S ISP_PUB_ATTR_SAMPLE =
						{ { 0, 0, 1920, 1080 }, { 1920, 1080 }, 30, BAYER_RGGB, WDR_MODE_NONE, 0, 4, 2};

static long diff_in_us(struct timespec t1, struct timespec t2)
{
	struct timespec diff;

	if (t2.tv_nsec-t1.tv_nsec < 0) {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
	} else {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
	}
	return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}

void _PLAT_ERR_Exit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32SnsId;
	ALG_LIB_S stAeLib;
	ALG_LIB_S stAwbLib;
	int i, j;

	if (g_stViConfig.s32WorkingViNum != 0) {
		// SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);
		for (i = 0; i < g_stViConfig.s32WorkingViNum; i++) {
			if (g_stViConfig.astViInfo[i].stChnInfo.ViChn < VI_MAX_CHN_NUM) {
				if (g_stViConfig.astViInfo[i].stPipeInfo.enMastPipeMode == VI_OFFLINE_VPSS_OFFLINE
				    || g_stViConfig.astViInfo[i].stPipeInfo.enMastPipeMode == VI_ONLINE_VPSS_OFFLINE) {
					s32Ret = CVI_VI_DisableChn(0, g_stViConfig.astViInfo[i].stChnInfo.ViChn);
					if (s32Ret != CVI_SUCCESS) {
						printf("[ERROR] CVI_VI_DisableChn failed with %#x!\n",
										s32Ret);
					}
				}
			}
			for (j = 0; j < WDR_MAX_PIPE_NUM; j++) {
				if (g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j] >= 0 &&
					g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
					s32Ret = CVI_VI_StopPipe(g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j]);
					if (s32Ret != CVI_SUCCESS) {
						printf("[ERROR] CVI_VI_StopPipe failed with %#x!\n", s32Ret);
					}
					s32Ret = CVI_VI_DestroyPipe(g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j]);
					if (s32Ret != CVI_SUCCESS) {
						printf("[ERROR] CVI_VI_DestroyPipe failed with %#x!\n", s32Ret);
					}
				}
			}
			s32Ret  = CVI_VI_DisableDev(g_stViConfig.astViInfo[i].stDevInfo.ViDev);
			if (s32Ret != CVI_SUCCESS) {
				printf("[ERROR] CVI_VI_DisableDev failed with %#x!\n", s32Ret);
			}
			CVI_VI_UnRegChnFlipMirrorCallBack(0, g_stViConfig.astViInfo[i].stDevInfo.ViDev);
			CVI_VI_UnRegPmCallBack(g_stViConfig.astViInfo[i].stDevInfo.ViDev);
		//isp exit
			if (g_IspPid[i]) {
				s32Ret = CVI_ISP_Exit(i);
				if (s32Ret != CVI_SUCCESS) {
					printf("CVI_ISP_Exit fail with %#x!\n", s32Ret);
					return;
				}
				pthread_join(g_IspPid[i], NULL);
				g_IspPid[i] = 0;
				u32SnsId = g_au32IspSnsId[i];

				if (u32SnsId > VI_MAX_PIPE_NUM) {
					printf("%s: invalid sensor id: %d\n", __func__, u32SnsId);
					return;
				}

				s32Ret = CVI_SNS_UnRegCallback(u32SnsId, i);
				if (s32Ret!= CVI_SUCCESS) {
					printf("CVI_SNS_UnRegCallback error id: %d s32Ret %d\n", i, s32Ret);
				}

				stAeLib.s32Id = i;
				strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
				s32Ret = CVI_AE_UnRegister(i, &stAeLib);
				if (s32Ret) {
					printf("AE Algo unRegister failed!, error: %d\n",	s32Ret);
					return;
				}

				stAwbLib.s32Id = i;
				strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
				s32Ret = CVI_AWB_UnRegister(i, &stAwbLib);
				if (s32Ret) {
					printf("AWB Algo unRegister failed!, error: %d\n",	s32Ret);
					return;
				}
#if ENABLE_AF_LIB
				ALG_LIB_S stAfLib;
				stAfLib.s32Id = i;
				strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, sizeof(stAfLib.acLibName));
				s32Ret = CVI_AF_UnRegister(i, &stAfLib);
				if (s32Ret) {
					printf("AF Algo unRegister failed!, error: %d\n",	s32Ret);
					return;
				}
#endif
			}
		}
	}
	CVI_VB_Exit();
	CVI_SYS_Exit();
}

static void sys_handle_signal(int nSignal, siginfo_t *si, void *arg)
{
	UNUSED(nSignal);
	UNUSED(si);
	UNUSED(arg);

	_PLAT_ERR_Exit();

	exit(1);
}


static CVI_VOID *ISP_Thread(void *arg)
{
	CVI_S32 s32Ret = 0;
	CVI_U8 IspDev = *(CVI_U8 *)arg;
	char szThreadName[20];

	free(arg);
	snprintf(szThreadName, sizeof(szThreadName), "ISP%d_RUN", IspDev);
	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);

	if (IspDev > 0) {
		printf("ISP Dev %d return\n", IspDev);
		return NULL;
	}

	// TODO: HGJ
	//CVI_SYS_RegisterThermalCallback(callback_FPS);

	printf("ISP Dev %d running!\n", IspDev);
	s32Ret = CVI_ISP_Run(IspDev);
	if (s32Ret != 0)
		printf("CVI_ISP_Run failed with %#x!\n", s32Ret);

	return NULL;
}

CVI_S32 sys_vi_init(SENSOR_CFG_S *sensor_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SNS_COMBO_DEV_ATTR_S pstRxAttr;
	VB_CONFIG_S stVbConf;
	int i;

	if (sensor_cfg == CVI_NULL) {
		printf("[ERROR] input point is NULL\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * ipcm init for dual os
	 ************************************************/

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		printf("[ERROR] CVI_SYS_Init failed!\n");
		return s32Ret;
	}
	/************************************************
	 * Get ini, config; Set sensor driver mipi attr,
	 * Get VI config
	 ************************************************/
	s32Ret = CVI_SNS_ParseIni(sensor_cfg);
	if (s32Ret == CVI_FAILURE) {
		printf("[ERROR] parse ini failed\n");
	}
	s32Ret = CVI_SNS_GetConfigInfo(sensor_cfg);
	if (s32Ret == CVI_FAILURE) {
		printf("[ERROR] get sns cfg failed\n");
	}
	s32Ret = CVI_SNS_SetSnsDrvCfg(sensor_cfg);
	if (s32Ret == CVI_FAILURE) {
		printf("[ERROR] set sns_drv failed\n");
	}

	SNS_INI_CFG_S *stSnsIniCfg = &sensor_cfg->sns_ini_cfg;
	SNS_CFG_S *stSnsCfg = &sensor_cfg->sns_cfg;

	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		g_stViConfig.s32WorkingViNum			= 1 + i;
		g_stViConfig.as32WorkingViId[i]			= i;

		g_stViConfig.astViInfo[i].stDevInfo.ViDev					= i;
		g_stViConfig.astViInfo[i].stDevInfo.enWDRMode				= stSnsCfg->enWDRMode[i];

		for (int j = 0; j < WDR_MAX_PIPE_NUM; j++) {
			g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j] = j == 0 ? i : -1;
		}
		g_stViConfig.astViInfo[i].stPipeInfo.enMastPipeMode		= VI_OFFLINE_VPSS_OFFLINE;
		g_stViConfig.astViInfo[i].stPipeInfo.bMultiPipe			= CVI_FALSE;
		g_stViConfig.astViInfo[i].stPipeInfo.bVcNumCfged			= CVI_FALSE;

		g_stViConfig.astViInfo[i].stChnInfo.ViChn					= 0;
		g_stViConfig.astViInfo[i].stChnInfo.enPixFormat			=
					stSnsCfg->bBypassIsp[i] ? PIXEL_FORMAT_YUYV : PIXEL_FORMAT_NV21;
		g_stViConfig.astViInfo[i].stChnInfo.enDynamicRange		= DYNAMIC_RANGE_SDR8;
		g_stViConfig.astViInfo[i].stChnInfo.enVideoFormat			= VIDEO_FORMAT_LINEAR;
		g_stViConfig.astViInfo[i].stChnInfo.enCompressMode		= COMPRESS_MODE_TILE;
	}
	/************************************************
	 * Set VI-VPSS config
	 ************************************************/
	VI_VPSS_MODE_S	stVIVPSSMode;


	/************************************************
	 * Config vpss online mode
	 ************************************************/
	stVIVPSSMode.aenMode[0] = stVIVPSSMode.aenMode[1] =
		g_stViConfig.astViInfo[i].stPipeInfo.enMastPipeMode;
	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		return s32Ret;
	}
	/************************************************
	 * Set VB config
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 0;

	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		int Vb_cnt = 0;
		bool createNewPool = true;
		CVI_U32 u32BlkSize, u32BlkRotSize;

		if (stSnsCfg->enFormatMode[i] == SNS_DATA_TYPE_YUV) {
			if (stSnsCfg->enChnMode[i] == SNS_CHN_MODE_2Multiplex) {
				Vb_cnt = 6;
			} else if (stSnsCfg->enChnMode[i] == SNS_CHN_MODE_3Multiplex) {
				Vb_cnt = 9;
			} else if (stSnsCfg->enChnMode[i] == SNS_CHN_MODE_4Multiplex) {
				Vb_cnt = 12;
			} else {
				Vb_cnt = 3;
			}
		} else {
			Vb_cnt = 3;
		}

		u32BlkSize = COMMON_GetPicBufferSize(stSnsCfg->u32ImageWigth[i], stSnsCfg->u32ImageHeight[i],
					g_stViConfig.astViInfo[i].stChnInfo.enPixFormat,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkRotSize = COMMON_GetPicBufferSize(stSnsCfg->u32ImageHeight[i], stSnsCfg->u32ImageWigth[i],
					g_stViConfig.astViInfo[i].stChnInfo.enPixFormat,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize = u32BlkSize > u32BlkRotSize ? u32BlkSize : u32BlkRotSize;


		for (CVI_U32 j = 0; j < stVbConf.u32MaxPoolCnt; j++) {
			if (stVbConf.astCommPool[j].u32BlkSize == u32BlkSize) {
				stVbConf.astCommPool[j].u32BlkCnt += Vb_cnt;
				createNewPool = false;
				break;
			}
		}

		if (createNewPool) {
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize = u32BlkSize;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt = Vb_cnt;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].enRemapMode = VB_REMAP_MODE_CACHED;
			stVbConf.u32MaxPoolCnt++;
			printf("[INFO]  createNewPool VBpool [%d] %d:%d, BlkCnt= %d, Size = %d\n",
						stVbConf.u32MaxPoolCnt, stSnsCfg->u32ImageWigth[i],
						stSnsCfg->u32ImageHeight[i],
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkCnt,
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkSize);
		} else {
			printf("[INFO] set VBpool [%d] %d:%d, BlkCnt= %d, Size = %d\n",
						stVbConf.u32MaxPoolCnt, stSnsCfg->u32ImageWigth[i],
						stSnsCfg->u32ImageHeight[i],
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkCnt,
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkSize);
		}

	}

	if (stVbConf.u32MaxPoolCnt == 1) {
		stVbConf.astCommPool[0].u32BlkCnt += 2;
	}


	/************************************************
	 * Set SYS config
	 ************************************************/
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = sys_handle_signal;
	sa.sa_flags = SA_SIGINFO|SA_RESETHAND; // Reset signal handler to system default after signal triggered
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		printf("[ERROR] CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		printf("[ERROR] CVI_VB_Init failed!\n");
		return s32Ret;
	}

	/************************************************
	 * Set sns reset, probe; Set MIPI attr
	 ************************************************/
	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		s32Ret = CVI_MIPI_SetSensorReset(stSnsIniCfg->MipiDev[i], stSnsIniCfg->s32RstPort[i],
						stSnsIniCfg->s32RstPin[i], stSnsIniCfg->s32RstPol[i], 1);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] sensor_%d reset failed!\n", i);
			return s32Ret;
		}
	}

	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		s32Ret = CVI_MIPI_SetMipiReset(stSnsIniCfg->MipiDev[i], 1);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] mipi dev_%d reset failed!\n", i);
			return s32Ret;
		}
	}

	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		if (CVI_SNS_GetSnsRxAttr(i, &pstRxAttr) != CVI_SUCCESS) {
			printf("[ERROR] get mipi dev_%d attr failed!\n", i);
			return CVI_FAILURE;
		}
		if (pstRxAttr.input_mode == INPUT_MODE_MIPI) {
			if (stSnsIniCfg->bHsettlen[i]) {
				pstRxAttr.mipi_attr.dphy.enable = 1;
				pstRxAttr.mipi_attr.dphy.hs_settle = stSnsIniCfg->u8Hsettle[i];
			}
		}
		if (pstRxAttr.input_mode == INPUT_MODE_MIPI ||
			pstRxAttr.input_mode == INPUT_MODE_SUBLVDS ||
			pstRxAttr.input_mode == INPUT_MODE_HISPI) {
			pstRxAttr.cif_mode = stSnsIniCfg->enSnsMode;
		}
		s32Ret = CVI_MIPI_SetMipiAttr(i, (CVI_VOID *)&pstRxAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] set mipi dev_%d attr failed!\n", i);
			return s32Ret;
		}
	}

	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		s32Ret = CVI_MIPI_SetSensorClock(stSnsIniCfg->MipiDev[i], 1);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] sensor %d clock enable failed!\n", i);
			return s32Ret;
		}
	}

	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		s32Ret = CVI_MIPI_SetSensorReset(stSnsIniCfg->MipiDev[i], stSnsIniCfg->s32RstPort[i],
							stSnsIniCfg->s32RstPin[i], stSnsIniCfg->s32RstPol[i], 0);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] sensor_%d unreset failed!\n", i);
			return s32Ret;
		}
	}

	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		if (CVI_SNS_SetSnsProbe(i) != CVI_SUCCESS) {
			printf("[ERROR] sensor_%d probe failed!\n", i);
			return CVI_FAILURE;
		}
	}
	/************************************************
	 * Set VI dev config
	 ************************************************/
	VI_DEV_ATTR_S       stViDevAttr;
	VI_DEV_BIND_PIPE_S  stViDevBindAttr;

	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		stViDevAttr.snrFps				= stSnsCfg->f32FrameRate[i];
		stViDevAttr.stSize.u32Width		= stSnsCfg->u32ImageWigth[i];
		stViDevAttr.stSize.u32Height	= stSnsCfg->u32ImageHeight[i];
		stViDevAttr.enIntfMode			= (VI_INTF_MODE_E)stSnsCfg->enInterFaceMode[i];
		stViDevAttr.enInputDataType		= (VI_DATA_TYPE_E)stSnsCfg->enFormatMode[i];
		stViDevAttr.enDataSeq			= (VI_YUV_DATA_SEQ_E)stSnsCfg->enYuvFormat[i];
		stViDevAttr.stWDRAttr.enWDRMode	= g_stViConfig.astViInfo[i].stDevInfo.enWDRMode;
		stViDevAttr.enWorkMode			= (VI_WORK_MODE_E)stSnsCfg->enChnMode[i];
		stViDevBindAttr.PipeId[0]		= stSnsIniCfg->MipiDev[i];
		stViDevBindAttr.u32Num			= 1;
		stViDevBindAttr.MipiDev			= stSnsIniCfg->MipiDev[i];

		s32Ret = CVI_VI_SetDevAttr(g_stViConfig.astViInfo[i].stDevInfo.ViDev, &stViDevAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] CVI_VI_SetDevAttr failed with %#x!\n", s32Ret);
			return s32Ret;
		}
		s32Ret = CVI_VI_SetDevBindAttr(g_stViConfig.astViInfo[i].stDevInfo.ViDev, &stViDevBindAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] CVI_VI_SetDevBindAttr failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		s32Ret = CVI_VI_EnableDev(g_stViConfig.astViInfo[i].stDevInfo.ViDev);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] CVI_VI_EnableDev failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}
	/************************************************
	 * Set VI pipe config
	 ************************************************/
	VI_PIPE_ATTR_S stPipeAttr;

	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		stPipeAttr.u32MaxW						= stSnsCfg->u32ImageWigth[i];
		stPipeAttr.u32MaxH						= stSnsCfg->u32ImageHeight[i];
		stPipeAttr.enPixFmt						= PIXEL_FORMAT_RGB_BAYER_12BPP;
		stPipeAttr.enBitWidth					= DATA_BITWIDTH_12;
		stPipeAttr.stFrameRate.s32SrcFrameRate	= -1;
		stPipeAttr.stFrameRate.s32DstFrameRate	= -1;
		stPipeAttr.bNrEn						= CVI_TRUE;
		stPipeAttr.bYuvBypassPath				= stSnsCfg->bBypassIsp[i];
		stPipeAttr.enCompressMode				=
							g_stViConfig.astViInfo[i].stChnInfo.enCompressMode;

		for (int j = 0; j < VI_MAX_PIPE_NUM; j++) {
			if (g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j] >= 0 &&
				g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
				s32Ret = CVI_VI_CreatePipe(g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j], &stPipeAttr);
				if (s32Ret != CVI_SUCCESS) {
					printf("[ERROR] CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
					return s32Ret;
				}

				s32Ret = CVI_VI_StartPipe(g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j]);
				if (s32Ret != CVI_SUCCESS) {
					printf("[ERROR] CVI_VI_StartPipe failed with %#x!\n", s32Ret);
					return s32Ret;
				}
			}
		}
	}
	//todo: ISP enable
	/************************************************
	 * Set ISP init
	 ************************************************/
	ISP_PUB_ATTR_S stPubAttr;
	ISP_BIND_ATTR_S stBindAttr;
	ALG_LIB_S stAeLib;
	ALG_LIB_S stAwbLib;
	SIZE_S stSize;
	VI_PIPE ViPipe = 0;
	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		ViPipe = i;
		memset(&stBindAttr, 0, sizeof(ISP_BIND_ATTR_S));
		memset(&stPubAttr, 0, sizeof(ISP_PUB_ATTR_S));

		stAeLib.s32Id = ViPipe;
		strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
		s32Ret = CVI_AE_Register(ViPipe, &stAeLib);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "AE Algo register failed!, error: %d\n",	s32Ret);
			return s32Ret;
		}

		stAwbLib.s32Id = ViPipe;
		strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
		s32Ret = CVI_AWB_Register(ViPipe, &stAwbLib);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "AWB Algo register failed!, error: %d\n",	s32Ret);
			return s32Ret;
		}
#if ENABLE_AF_LIB
		ALG_LIB_S stAfLib;
		stAfLib.s32Id = ViPipe;
		strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, sizeof(stAfLib.acLibName));
		s32Ret = CVI_AF_Register(ViPipe, &stAfLib);

		if (s32Ret != CVI_SUCCESS) {
			printf("AF Algo register failed!, error: %d\n", s32Ret);
			return s32Ret;
		}
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

		memcpy(&stPubAttr, &ISP_PUB_ATTR_SAMPLE, sizeof(ISP_PUB_ATTR_S));

		stSize.u32Width = stSnsCfg->u32ImageWigth[ViPipe];
		stSize.u32Height = stSnsCfg->u32ImageHeight[ViPipe];

		stPubAttr.stSnsSize.u32Width = stSize.u32Width;
		stPubAttr.stSnsSize.u32Height = stSize.u32Height;
		stPubAttr.stWndRect.u32Width = stSize.u32Width;
		stPubAttr.stWndRect.u32Height = stSize.u32Height;

		stPubAttr.enWDRMode = stSnsCfg->enWDRMode[ViPipe];
		stPubAttr.enBayer = (ISP_BAYER_FORMAT_E)stSnsCfg->enBayerFormat[ViPipe];
		stPubAttr.f32FrameRate = stSnsCfg->f32FrameRate[ViPipe];
		stPubAttr.u8LaneNum = stSnsCfg->u8LaneNumber[ViPipe];
		stPubAttr.u8EnableMaster = stSnsCfg->u8EnMasterMode[ViPipe];

		s32Ret = CVI_ISP_SetPubAttr(ViPipe, &stPubAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SetPubAttr failed with %#x!\n", s32Ret);
			return s32Ret;
		}
		s32Ret = CVI_ISP_Init(ViPipe);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "ISP Init failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	CVI_U8 *arg = malloc(sizeof(*arg));
	struct sched_param param;
	pthread_attr_t attr;
	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		ViPipe = i;

		if (arg == NULL) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "malloc failed\n");
			return CVI_FAILURE;
		}

		*arg = ViPipe;
		param.sched_priority = 80;

		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_attr_setschedparam(&attr, &param);
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		s32Ret = pthread_create(&g_IspPid[ViPipe], &attr, ISP_Thread, arg);
		if (s32Ret != 0) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "create isp running thread failed!, error: %d, %s\r\n",
						s32Ret, strerror(s32Ret));
			return CVI_FAILURE;
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
	}

	/************************************************
	 * Set sensor init
	 ************************************************/
	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		if (CVI_SNS_SetSnsInit(i) != CVI_SUCCESS) {
			printf("[ERROR] sensor_%d init failed!\n", i);
			return CVI_FAILURE;
		}
	}
	/************************************************
	 * Set VI chn config
	 ************************************************/
	VI_CHN_ATTR_S stChnAttr;

	for (i = 0; i < stSnsIniCfg->devNum; i++) {
		stChnAttr.stSize.u32Width = stSnsCfg->u32ImageWigth[i];
		stChnAttr.stSize.u32Height = stSnsCfg->u32ImageHeight[i];
		stChnAttr.enDynamicRange = g_stViConfig.astViInfo[i].stChnInfo.enDynamicRange;
		stChnAttr.enVideoFormat  = g_stViConfig.astViInfo[i].stChnInfo.enVideoFormat;
		stChnAttr.enCompressMode = g_stViConfig.astViInfo[i].stChnInfo.enCompressMode;
		stChnAttr.enPixelFormat = g_stViConfig.astViInfo[i].stChnInfo.enPixFormat;
		stChnAttr.u32Depth = 1;
		stChnAttr.u32BindVbPool = -1;

		/* fill the sensor orientation */
		stChnAttr.bMirror = false;
		stChnAttr.bFlip = false;

		s32Ret = CVI_VI_SetChnAttr(g_stViConfig.astViInfo[i].stPipeInfo.aPipe[0],
						g_stViConfig.astViInfo[i].stChnInfo.ViChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] CVI_VI_SetChnAttr failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		if (CVI_SNS_SetVIFlipMirrorCB(g_stViConfig.astViInfo[i].stPipeInfo.aPipe[0],
									g_stViConfig.astViInfo[i].stDevInfo.ViDev) != CVI_SUCCESS) {
			printf("[ERROR] CVI_SNS_SetVIFlipMirrorCB failed!\n");
		}

		s32Ret = CVI_VI_EnableChn(g_stViConfig.astViInfo[i].stPipeInfo.aPipe[0],
									g_stViConfig.astViInfo[i].stChnInfo.ViChn);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] CVI_VI_EnableChn failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 sensor_dump_raw(void)
{
	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DUMP_ATTR_S attr;
	struct timeval tv1;
	int frm_num = 1, j = 0;
	CVI_U32 dev = 0, loop = 0;
	struct timespec start, end;
	CVI_S32 s32Ret = CVI_SUCCESS;

	memset(stVideoFrame, 0, sizeof(stVideoFrame));

	stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

	printf("[INFO] To get raw dump from dev(0~1): ");
	scanf("%d", &dev);

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;

	CVI_VI_SetPipeDumpAttr(dev, &attr);

	attr.bEnable = 0;
	attr.enDumpType = VI_DUMP_TYPE_IR;

	CVI_VI_GetPipeDumpAttr(dev, &attr);

	printf("[INFO] Enable(%d), DumpType(%d):\n", attr.bEnable, attr.enDumpType);
	printf("[INFO] how many loops to do (1~60)");
	scanf("%d", &loop);

	if (loop > 60)
		return s32Ret;

	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		frm_num = 1;

		CVI_VI_GetPipeFrame(dev, stVideoFrame, 1000);

		if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0)
			frm_num = 2;

		gettimeofday(&tv1, NULL);

		for (j = 0; j < frm_num; j++) {
			size_t image_size = stVideoFrame[j].stVFrame.u32Length[0];
			unsigned char *ptr = calloc(1, image_size);
			FILE *output;
			char img_name[128] = {0,}, order_id[8] = {0,};

			if (attr.enDumpType == VI_DUMP_TYPE_RAW) {
				stVideoFrame[j].stVFrame.pu8VirAddr[0]
					= CVI_SYS_Mmap(stVideoFrame[j].stVFrame.u64PhyAddr[0]
					  , stVideoFrame[j].stVFrame.u32Length[0]);
				printf("[INFO] paddr(%#"PRIx64") vaddr(%p)\n",
							stVideoFrame[j].stVFrame.u64PhyAddr[0],
							stVideoFrame[j].stVFrame.pu8VirAddr[0]);

				memcpy(ptr, (const void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
					stVideoFrame[j].stVFrame.u32Length[0]);
				CVI_SYS_Munmap((void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
						stVideoFrame[j].stVFrame.u32Length[0]);

				switch (stVideoFrame[j].stVFrame.enBayerFormat) {
				default:
				case BAYER_FORMAT_BG:
					snprintf(order_id, sizeof(order_id), "BG");
					break;
				case BAYER_FORMAT_GB:
					snprintf(order_id, sizeof(order_id), "GB");
					break;
				case BAYER_FORMAT_GR:
					snprintf(order_id, sizeof(order_id), "GR");
					break;
				case BAYER_FORMAT_RG:
					snprintf(order_id, sizeof(order_id), "RG");
					break;
				}

				snprintf(img_name, sizeof(img_name),
						"./vi_%d_%s_%s_w_%d_h_%d_x_%d_y_%d_tv_%ld_%ld.raw",
						dev, (j == 0) ? "LE" : "SE", order_id,
						stVideoFrame[j].stVFrame.u32Width,
						stVideoFrame[j].stVFrame.u32Height,
						stVideoFrame[j].stVFrame.s16OffsetLeft,
						stVideoFrame[j].stVFrame.s16OffsetTop,
						(long int)tv1.tv_sec, (long int)tv1.tv_usec);

				printf("[INFO] dump image %s\n", img_name);

				output = fopen(img_name, "wb");

				fwrite(ptr, image_size, 1, output);
				fclose(output);
				free(ptr);
			}
		}

		CVI_VI_ReleasePipeFrame(dev, stVideoFrame);

		clock_gettime(CLOCK_MONOTONIC, &end);
		printf("[INFO] ms consumed: %f\n",
					(CVI_FLOAT)diff_in_us(start, end) / 1000);

		loop--;
	}

	printf("[INFO] Dump VI raw TEST-PASS\n");

	return s32Ret;
}

static CVI_S32 _vi_get_chn_frame(CVI_U8 chn)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VI_CROP_INFO_S crop_info = {0};
	struct timeval tv1;

	if (CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 3000) == 0) {
		FILE *output;
		size_t image_size = stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1]
				  + stVideoFrame.stVFrame.u32Length[2];
		CVI_VOID *vir_addr;
		CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;
		CVI_CHAR img_name[128] = {0, };

		printf("width: %d, height: %d, total_buf_length: %zu\n",
			   stVideoFrame.stVFrame.u32Width,
			   stVideoFrame.stVFrame.u32Height, image_size);

		snprintf(img_name, sizeof(img_name), "sample_%d.yuv", chn);

		output = fopen(img_name, "wb");
		gettimeofday(&tv1, NULL);
		if (output == NULL) {
			memset(img_name, 0x0, sizeof(img_name));
			snprintf(img_name, sizeof(img_name), "/mnt/data/sample_%d_tv_%ld_%ld.yuv",
					chn, (long int)tv1.tv_sec, (long int)tv1.tv_usec);
			output = fopen(img_name, "wb");
			if (output == NULL) {
				CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame);
				printf("fopen fail\n");
				return CVI_FAILURE;
			}
		}

		u32LumaSize =  stVideoFrame.stVFrame.u32Stride[0] * stVideoFrame.stVFrame.u32Height;
		u32ChromaSize =  stVideoFrame.stVFrame.u32Stride[1] * stVideoFrame.stVFrame.u32Height / 2;
		CVI_VI_GetChnCrop(0, chn, &crop_info);
		if (crop_info.bEnable) {
			u32LumaSize = ALIGN((crop_info.stCropRect.u32Width * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2);
			u32ChromaSize = (ALIGN(((crop_info.stCropRect.u32Width >> 1) * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2)) >> 1;
		}
		vir_addr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[0], vir_addr, image_size);
		plane_offset = 0;
		for (int i = 0; i < 3; i++) {
			if (stVideoFrame.stVFrame.u32Length[i] != 0) {
				stVideoFrame.stVFrame.pu8VirAddr[i] = vir_addr + plane_offset;
				plane_offset += stVideoFrame.stVFrame.u32Length[i];
				printf("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) length(%d)\n",
					   i, stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Stride[i],
					   stVideoFrame.stVFrame.u32Length[i]);
				fwrite((void *)stVideoFrame.stVFrame.pu8VirAddr[i]
					, (i == 0) ? u32LumaSize : u32ChromaSize, 1, output);
			}
		}
		CVI_SYS_Munmap(vir_addr, image_size);

		if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0)
			printf("CVI_VI_ReleaseChnFrame NG\n");

		fclose(output);
		return CVI_SUCCESS;
	}
	printf("CVI_VI_GetChnFrame NG\n");
	return CVI_FAILURE;
}

CVI_S32 sensor_dump_yuv(void)
{
	CVI_S32 loop = 0;
	CVI_U32 ok = 0, ng = 0;
	CVI_U8  chn = 0;
	int tmp;
	struct timespec start, end;

	printf("[INFO] Get frm from which chn(0~1): ");
	scanf("%d", &tmp);
	chn = tmp;
	printf("[INFO] how many loops to do(11111 is infinite: ");
	scanf("%d", &loop);
	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		if (_vi_get_chn_frame(chn) == CVI_SUCCESS) {
			++ok;
			clock_gettime(CLOCK_MONOTONIC, &end);
			printf("[INFO] ms consumed: %f\n",
						(CVI_FLOAT)diff_in_us(start, end)/1000);
		} else
			++ng;
		//sleep(1);
		if (loop != 11111)
			loop--;
	}
	printf("[INFO] VI GetChnFrame OK(%d) NG(%d)\n", ok, ng);

	printf("[INFO] Dump VI yuv TEST-PASS\n");

	return CVI_SUCCESS;
}

CVI_S32 sensor_flip_mirror(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	//todo:add flip/mirror func
	return s32Ret;
}

CVI_S32 sensor_linear_wdr_switch(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	//todo:add linear/wdr switch func
	return s32Ret;
}

CVI_S32 sensor_dump(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	//todo:add dump sensor func
	return s32Ret;
}

CVI_S32 sensor_proc(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_S32 op;

	printf("---debug_info------------------------------------------------\n");
	printf("1: /proc/soph/vi_dbg\n");
	printf("2: /proc/soph/vi\n");
	printf("3: /proc/mipi-rx\n");
	scanf("%d", &op);

	switch (op) {
	case 1:
		system("cat /proc/soph/vi_dbg");
		break;
	case 2:
		system("cat /proc/soph/vi");
		break;
	case 3:
		system("cat /proc/mipi-rx");
		break;
	default:
		break;
	}
	return s32Ret;
}

CVI_S32 sys_vi_deinit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ViNum;
	CVI_U32 u32SnsId;
	CVI_S32 i;
	TEST_VI_INFO_S stViInfo;
	VI_CHN              ViChn;
	VI_PIPE             ViPipe = 0;
	VI_VPSS_MODE_E      enMastPipeMode;
	VI_DEV ViDev;
	ALG_LIB_S stAeLib;
	ALG_LIB_S stAwbLib;

	for (i = 0; i < g_stViConfig.s32WorkingViNum - 1; i++) {
		s32ViNum  = g_stViConfig.as32WorkingViId[i];
		stViInfo = g_stViConfig.astViInfo[s32ViNum];

	/************************************************
	 *  ISP  stop
	 ************************************************/
		if (g_IspPid[i]) {
			s32Ret = CVI_ISP_Exit(i);
			if (s32Ret != CVI_SUCCESS) {
				printf("CVI_ISP_Exit fail with %#x!\n", s32Ret);
				return s32Ret;
			}
			pthread_join(g_IspPid[i], NULL);
			g_IspPid[i] = 0;
			u32SnsId = g_au32IspSnsId[i];

			if (u32SnsId > VI_MAX_PIPE_NUM) {
				printf("%s: invalid sensor id: %d\n", __func__, u32SnsId);
				return CVI_FAILURE;
			}

			s32Ret = CVI_SNS_UnRegCallback(u32SnsId, i);
			if (s32Ret!= CVI_SUCCESS) {
				printf("CVI_SNS_UnRegCallback error id: %d s32Ret %d\n", i, s32Ret);
			}

			stAeLib.s32Id = i;
			strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
			s32Ret = CVI_AE_UnRegister(i, &stAeLib);
			if (s32Ret) {
				printf("AE Algo unRegister failed!, error: %d\n",	s32Ret);
				return s32Ret;
			}

			stAwbLib.s32Id = i;
			strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
			s32Ret = CVI_AWB_UnRegister(i, &stAwbLib);
			if (s32Ret) {
				printf("AWB Algo unRegister failed!, error: %d\n",	s32Ret);
				return s32Ret;
			}
#if ENABLE_AF_LIB
			ALG_LIB_S stAfLib;
			stAfLib.s32Id = i;
			strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, sizeof(stAfLib.acLibName));
			s32Ret = CVI_AF_UnRegister(i, &stAfLib);
			if (s32Ret) {
				printf("AF Algo unRegister failed!, error: %d\n",	s32Ret);
				return s32Ret;
			}
#endif
		}
	/************************************************
	 *  VI chn stop
	 ************************************************/
		ViChn  = stViInfo.stChnInfo.ViChn;
		if (ViChn < VI_MAX_CHN_NUM) {
			enMastPipeMode = stViInfo.stPipeInfo.enMastPipeMode;

			if (enMastPipeMode == VI_OFFLINE_VPSS_OFFLINE
				|| enMastPipeMode == VI_ONLINE_VPSS_OFFLINE) {
				s32Ret = CVI_VI_DisableChn(ViPipe, ViChn);
				if (s32Ret != CVI_SUCCESS) {
					printf("CVI_VI_DisableChn failed with %#x!\n",
									s32Ret);
					return s32Ret;
				}
			}
		}
	/************************************************
	 *  VI pipe stop
	 ************************************************/
		for (i = 0; i < WDR_MAX_PIPE_NUM; i++) {
			if (stViInfo.stPipeInfo.aPipe[i] >= 0  && stViInfo.stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM) {
				ViPipe = stViInfo.stPipeInfo.aPipe[i];
				s32Ret = CVI_VI_StopPipe(ViPipe);
				if (s32Ret != CVI_SUCCESS) {
					printf("CVI_VI_StopPipe failed with %#x!\n", s32Ret);
					return s32Ret;
				}

				s32Ret = CVI_VI_DestroyPipe(ViPipe);
				if (s32Ret != CVI_SUCCESS) {
					printf("CVI_VI_DestroyPipe failed with %#x!\n", s32Ret);
					return s32Ret;
				}

			}
		}

	/************************************************
	 *  VI dev stop
	 ************************************************/
		ViDev   = stViInfo.stDevInfo.ViDev;
		s32Ret  = CVI_VI_DisableDev(ViDev);

		CVI_VI_UnRegChnFlipMirrorCallBack(0, ViDev);
		CVI_VI_UnRegPmCallBack(ViDev);

		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VI_DisableDev failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	/************************************************
	 *  Sys exit
	 ************************************************/
	CVI_VB_Exit();
	CVI_SYS_Exit();

	return s32Ret;
}

int main(int argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 op;

	UNUSED(argc);
	UNUSED(argv);

	s32Ret = sys_vi_init(&g_stSensorCfg);
	if (s32Ret != CVI_SUCCESS) {
		_PLAT_ERR_Exit();
		return s32Ret;
	}

#ifdef ENABLE_ISP_TOOL_DAEMON
	isp_daemon2_init(JSONRPC_PORT);
#endif

	usleep(500 * 1000);

	system("stty erase ^H");

	do {
		printf("---Basic------------------------------------------------\n");
		printf("1: dump vi raw frame\n");
		printf("2: dump vi yuv frame\n");
		printf("3: set chn flip/mirror\n");
		printf("4: linear wdr switch\n");
		printf("5: AE debug\n");
		printf("6: sensor dump\n");
#ifndef CONFIG_DUAL_OS
		printf("7: sensor proc\n");
#endif
		printf("255: exit\n");
		scanf("%d", &op);

		switch (op) {
		case 1:
			s32Ret = sensor_dump_raw();
			break;
		case 2:
			s32Ret = sensor_dump_yuv();
			break;
		case 3:
			s32Ret = sensor_flip_mirror();
			break;
		case 4:
			s32Ret = sensor_linear_wdr_switch();
			break;
		case 5:
			s32Ret = sensor_ae_test();
			break;
		case 6:
			s32Ret = sensor_dump();
			break;
#ifndef CONFIG_DUAL_OS
			case 7:
				s32Ret = sensor_proc();
			break;
#endif
		default:
			break;
		}
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] op(%d) failed with %#x!\n", op, s32Ret);
			break;
		}
	} while (op != 255);

#ifdef ENABLE_ISP_TOOL_DAEMON
	isp_daemon2_uninit();
#endif

	sys_vi_deinit();

	return s32Ret;
}

