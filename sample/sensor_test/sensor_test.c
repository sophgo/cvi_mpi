
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <inttypes.h>

#include <fcntl.h>		/* low-level i/o */
#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_isp.h"
#include "cvi_comm_sns.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"
#include "cvi_sns_ctrl.h"
#include "cvi_msg_client.h"
#include "sample_comm.h"
#include "ae_test.h"
#include "replay.h"

static SAMPLE_VI_CONFIG_S g_stViConfig;
static SAMPLE_INI_CFG_S g_stIniCfg;

static void sys_handle_signal(int nSignal, siginfo_t *si, void *arg)
{
	UNUSED(nSignal);
	UNUSED(si);
	UNUSED(arg);

	if (g_stViConfig.s32WorkingViNum != 0) {
		SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);
		SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);
	}
	SAMPLE_COMM_SYS_Exit();
	exit(1);
}

static int sys_vi_init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MMF_VERSION_S stVersion;
	SAMPLE_INI_CFG_S stIniCfg;
	SAMPLE_VI_CONFIG_S stViConfig;
	LOG_LEVEL_CONF_S log_conf;

	memset(&stVersion, 0, sizeof(MMF_VERSION_S));
	memset(&stIniCfg, 0, sizeof(SAMPLE_INI_CFG_S));
	memset(&stViConfig, 0, sizeof(SAMPLE_VI_CONFIG_S));
	memset(&log_conf, 0, sizeof(LOG_LEVEL_CONF_S));

	s32Ret = CVI_MSG_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_MSG_Init fail\n");
		return s32Ret;
	}

	CVI_SYS_GetVersion(&stVersion);
	SAMPLE_PRT("MMF Version:%s\n", stVersion.version);

	log_conf.enModId = CVI_ID_LOG;
	log_conf.s32Level = CVI_DBG_INFO;
	CVI_LOG_SetLevelConf(&log_conf);

	// Get config from ini if found.
	s32Ret = SAMPLE_COMM_VI_ParseIni(&stIniCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Parse fail\n");
	} else {
		SAMPLE_PRT("Parse complete\n");
	}


	//Set sensor number
	CVI_VI_SetDevNum(stIniCfg.devNum);

	/************************************************
	 * step1:  Config VI
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	for (CVI_S32 i = 0; i < stViConfig.s32WorkingViNum; i++) {
		stViConfig.astViInfo[i].stChnInfo.enPixFormat =
			SAMPLE_COMM_VI_GetYuvBypassSts(stIniCfg.enSnsType[i])
			? PIXEL_FORMAT_YUYV : PIXEL_FORMAT_NV21;
	}

	memcpy(&g_stViConfig, &stViConfig, sizeof(SAMPLE_VI_CONFIG_S));
	memcpy(&g_stIniCfg, &stIniCfg, sizeof(SAMPLE_INI_CFG_S));

	/************************************************
	 * step2:  Get input size
	 ************************************************/
	CVI_U32 u32BlkSize, u32BlkRotSize;
	PIC_SIZE_E enPicSize;
	VB_CONFIG_S stVbConf;
	SIZE_S stSize;
	CVI_BOOL b64Ddr = false;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 0;

	for (CVI_S32 i = 0; i < stViConfig.s32WorkingViNum; i++) {
		bool createNewPool = true;

		s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[i], &enPicSize);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
			return s32Ret;
		}

		s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
			return s32Ret;
		}

		u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
					stViConfig.astViInfo[i].stChnInfo.enPixFormat,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkRotSize = COMMON_GetPicBufferSize(stSize.u32Height, stSize.u32Width,
					stViConfig.astViInfo[i].stChnInfo.enPixFormat,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize = u32BlkSize > u32BlkRotSize ? u32BlkSize : u32BlkRotSize;

		for (CVI_U32 j = 0; j < stVbConf.u32MaxPoolCnt; j++) {
			if (stVbConf.astCommPool[j].u32BlkSize == u32BlkSize) {
				stVbConf.astCommPool[j].u32BlkCnt += 2;
				createNewPool = false;
				break;
			}
		}

		s32Ret = SAMPLE_COMM_CheckDDR64MBSize(&b64Ddr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_CheckDDR64MBSize failed with %#x\n", s32Ret);
			return s32Ret;
		}

		if (createNewPool) {
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize = u32BlkSize;
			if (!b64Ddr)
				stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt = 2;
			else
				stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt = 4;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].enRemapMode = VB_REMAP_MODE_CACHED;
			stVbConf.u32MaxPoolCnt++;
		}
	}

	if (stVbConf.u32MaxPoolCnt == 1) {
		stVbConf.astCommPool[0].u32BlkCnt += 2;
	}

	/************************************************
	 * step3:  Init modules
	 ************************************************/
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = sys_handle_signal;
	sa.sa_flags = SA_SIGINFO|SA_RESETHAND; // Reset signal handler to system default after signal triggered
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_PLAT_VI_INIT(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static void sys_vi_deinit(void)
{
	SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);

	SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);

	SAMPLE_COMM_SYS_Exit();

	CVI_MSG_Deinit();
}

static CVI_S32 _vi_get_chn_frame(CVI_U8 chn)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VI_CROP_INFO_S crop_info = {0};

	if (CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 3000) == 0) {
		FILE *output;
		size_t image_size = stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1]
				  + stVideoFrame.stVFrame.u32Length[2];
		CVI_VOID *vir_addr;
		CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;
		CVI_CHAR img_name[128] = {0, };

		CVI_TRACE_LOG(CVI_DBG_WARN, "width: %d, height: %d, total_buf_length: %zu\n",
			   stVideoFrame.stVFrame.u32Width,
			   stVideoFrame.stVFrame.u32Height, image_size);

		snprintf(img_name, sizeof(img_name), "sample_%d.yuv", chn);

		output = fopen(img_name, "wb");
		if (output == NULL) {
			memset(img_name, 0x0, sizeof(img_name));
			snprintf(img_name, sizeof(img_name), "/mnt/data/sample_%d.yuv", chn);
			output = fopen(img_name, "wb");
			if (output == NULL) {
				CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame);
				CVI_TRACE_LOG(CVI_DBG_ERR, "fopen fail\n");
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
				CVI_TRACE_LOG(CVI_DBG_WARN,
					   "plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) length(%d)\n",
					   i, stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Stride[i],
					   stVideoFrame.stVFrame.u32Length[i]);
				fwrite((void *)stVideoFrame.stVFrame.pu8VirAddr[i]
					, (i == 0) ? u32LumaSize : u32ChromaSize, 1, output);
			}
		}
		CVI_SYS_Munmap(vir_addr, image_size);

		if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_ReleaseChnFrame NG\n");
		}

		fclose(output);
		return CVI_SUCCESS;
	}
	CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetChnFrame NG\n");
	return CVI_FAILURE;
}

#if (CONFIG_CVI_LOG_TRACE_SUPPORT == 1)
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
#endif

static void ViConfigReInit(SAMPLE_VI_CONFIG_S *p_stViConfig, SAMPLE_INI_CFG_S *p_stIniCfg)
{
	int s32WorkSnsId = 0;

	for (s32WorkSnsId = 0; s32WorkSnsId < p_stIniCfg->devNum; s32WorkSnsId++) {
		p_stViConfig->s32WorkingViNum					= 1 + s32WorkSnsId;
		p_stViConfig->as32WorkingViId[s32WorkSnsId]			= s32WorkSnsId;
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.enSnsType	=
			p_stIniCfg->enSnsType[s32WorkSnsId];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.MipiDev		=
			p_stIniCfg->MipiDev[s32WorkSnsId];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s32BusId	=
			p_stIniCfg->s32BusId[s32WorkSnsId];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[0]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][0];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[1]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][1];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[2]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][2];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[3]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][3];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[4]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][4];

		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[0] =
			p_stIniCfg->as8PNSwap[s32WorkSnsId][0];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[1] =
			p_stIniCfg->as8PNSwap[s32WorkSnsId][1];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[2] =
			p_stIniCfg->as8PNSwap[s32WorkSnsId][2];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[3] =
			p_stIniCfg->as8PNSwap[s32WorkSnsId][3];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[4] =
			p_stIniCfg->as8PNSwap[s32WorkSnsId][4];
		p_stViConfig->astViInfo[s32WorkSnsId].stDevInfo.enWDRMode =
			p_stIniCfg->enWDRMode[s32WorkSnsId];
	}
}

CVI_S32 sensor_dump_yuv(void)
{
	CVI_S32 loop = 0;
	CVI_U32 ok = 0, ng = 0;
	CVI_U8  chn = 0;
	int tmp;
	struct timespec start, end;

	CVI_TRACE_LOG(CVI_DBG_WARN, "Get frm from which chn(0~2): ");
	scanf("%d", &tmp);
	chn = tmp;
	CVI_TRACE_LOG(CVI_DBG_WARN, "how many loops to do(11111 is infinite: ");
	scanf("%d", &loop);
	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		if (_vi_get_chn_frame(chn) == CVI_SUCCESS) {
			++ok;
			clock_gettime(CLOCK_MONOTONIC, &end);
			CVI_TRACE_LOG(CVI_DBG_WARN, "ms consumed: %f\n",
						(CVI_FLOAT)diff_in_us(start, end)/1000);
		} else
			++ng;
		//sleep(1);
		if (loop != 11111)
			loop--;
	}
	CVI_TRACE_LOG(CVI_DBG_WARN, "VI GetChnFrame OK(%d) NG(%d)\n", ok, ng);

	CVI_TRACE_LOG(CVI_DBG_WARN, "Dump VI yuv TEST-PASS\n");

	return CVI_SUCCESS;
}

static CVI_S32 sensor_flip_mirror(void)
{
	int flip;
	int mirror;
	int chnID;
	int pipeID;

	CVI_TRACE_LOG(CVI_DBG_WARN, "chn(0~2): ");
	scanf("%d", &chnID);
	CVI_TRACE_LOG(CVI_DBG_WARN, "Flip enable/disable(1/0): ");
	scanf("%d", &flip);
	CVI_TRACE_LOG(CVI_DBG_WARN, "Mirror enable/disable(1/0): ");
	scanf("%d", &mirror);
	pipeID = chnID;
	CVI_VI_SetChnFlipMirror(pipeID, chnID, flip, mirror);

	return CVI_SUCCESS;
}

static CVI_S32 sensor_dump_raw(void)
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

	CVI_TRACE_LOG(CVI_DBG_WARN, "To get raw dump from dev(0~2): ");
	scanf("%d", &dev);

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;

	CVI_VI_SetPipeDumpAttr(dev, &attr);

	attr.bEnable = 0;
	attr.enDumpType = VI_DUMP_TYPE_IR;

	CVI_VI_GetPipeDumpAttr(dev, &attr);

	CVI_TRACE_LOG(CVI_DBG_WARN, "Enable(%d), DumpType(%d):\n", attr.bEnable, attr.enDumpType);
	CVI_TRACE_LOG(CVI_DBG_WARN, "how many loops to do (1~60)");
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
				CVI_TRACE_LOG(CVI_DBG_WARN, "paddr(%#"PRIx64") vaddr(%p)\n",
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
						tv1.tv_sec, tv1.tv_usec);

				CVI_TRACE_LOG(CVI_DBG_WARN, "dump image %s\n", img_name);

				output = fopen(img_name, "wb");

				fwrite(ptr, image_size, 1, output);
				fclose(output);
				free(ptr);
			}
		}

		CVI_VI_ReleasePipeFrame(dev, stVideoFrame);

		clock_gettime(CLOCK_MONOTONIC, &end);
		CVI_TRACE_LOG(CVI_DBG_WARN, "ms consumed: %f\n",
					(CVI_FLOAT)diff_in_us(start, end) / 1000);

		loop--;
	}

	CVI_TRACE_LOG(CVI_DBG_WARN, "Dump VI raw TEST-PASS\n");

	return s32Ret;
}

static CVI_S32 sensor_linear_wdr_switch(void)
{
	int tmp;
	CVI_U8 wdrMode = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);
	// Stop VI.
	SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);
	// Close ISP device.
	s32Ret = SAMPLE_COMM_VI_CLOSE();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi close failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	// select which mode want to switch.
	printf("Please select sensor input mode (0:linear/1:wdr) :");
	scanf("%d", &tmp);
	wdrMode = tmp;
	if (wdrMode == 0) {
		// Reset main sensor initial config to linear setting.
		g_stIniCfg.enSnsType[0] = SONY_IMX307_2L_MIPI_2M_30FPS_12BIT;
		g_stIniCfg.enWDRMode[0] = WDR_MODE_NONE;
		// Reset slave sensor initial config to linear setting.
		g_stIniCfg.enSnsType[1] = SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT;
		g_stIniCfg.enWDRMode[1] = WDR_MODE_NONE;
	} else {
		// Reset main sensor initial config to wdr setting.
		g_stIniCfg.enSnsType[0] = SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1;
		g_stIniCfg.enWDRMode[0] = WDR_MODE_2To1_LINE;
		// Reset slave sensor initial config to wdr setting.
		g_stIniCfg.enSnsType[1] = SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT_WDR2TO1;
		g_stIniCfg.enWDRMode[1] = WDR_MODE_2To1_LINE;
	}
	// Reconfig VI setting for different mode Re-initial correctly.
	ViConfigReInit(&g_stViConfig, &g_stIniCfg);
	// open Isp device.
	s32Ret = SAMPLE_COMM_VI_OPEN();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi open failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	// Initial VI & ISP.
	s32Ret = SAMPLE_PLAT_VI_INIT(&g_stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

#ifdef ENABLE_LOAD_ISPD_SO

#include <dlfcn.h>

void load_ispd(void)
{
#define ISPD_LIBNAME "libcvi_ispd2.so"
#define ISPD_CONNECT_PORT 5566

	char *error = NULL;
	void *handle = NULL;

	dlerror();
	handle = dlopen(ISPD_LIBNAME, RTLD_NOW);

	if (handle) {
		void (*daemon_init)(unsigned int port);

		printf("Load dynamic library %s success\n", ISPD_LIBNAME);

		dlerror();
		daemon_init = dlsym(handle, "isp_daemon2_init");
		error = dlerror();

		if (error == NULL) {
			(*daemon_init)(ISPD_CONNECT_PORT);
		} else {
			printf("Run daemon initial fail, %s\n", error);
			dlclose(handle);
		}
	} else {
		error = dlerror();
		printf("dlopen: %s, error: %s\n", ISPD_LIBNAME, error);
	}
}
#endif

int main(int argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 op;

	UNUSED(argc);
	UNUSED(argv);

	SAMPLE_PRT("select is replay yes [0] no [1]:\n");
	scanf("%d", &op);
	if (!op) {
		s32Ret = replay_vi_init();
		if (s32Ret != CVI_SUCCESS)
			return s32Ret;
		goto REPLAY_PATH;
	}

	s32Ret = sys_vi_init();
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

#ifdef ENABLE_LOAD_ISPD_SO
	load_ispd();
#endif

	usleep(500 * 1000);

	system("stty erase ^H");

	do {
		SAMPLE_PRT("---Basic------------------------------------------------\n");
		SAMPLE_PRT("1: dump vi raw frame\n");
		SAMPLE_PRT("2: dump vi yuv frame\n");
		SAMPLE_PRT("3: set chn flip/mirror\n");
		SAMPLE_PRT("4: linear wdr switch\n");
		SAMPLE_PRT("5: AE debug\n");
		SAMPLE_PRT("255: exit\n");
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
		default:
			break;
		}
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "op(%d) failed with %#x!\n", op, s32Ret);
			break;
		}
	} while (op != 255);

REPLAY_PATH:
	sys_vi_deinit();

	return s32Ret;
}

