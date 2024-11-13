/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample_common.h
 * Description:
 */

#ifndef __SAMPLE_COMM_H__
#define __SAMPLE_COMM_H__

#include <pthread.h>

#include "cvi_sys.h"
#include <cvi_common.h>
#include "cvi_buffer.h"
#include "cvi_comm_vb.h"
#include "cvi_comm_isp.h"
#include "cvi_comm_3a.h"
#include "cvi_comm_sns.h"
#include <cvi_comm_vi.h>
#include <cvi_comm_vpss.h>
#include <cvi_comm_vo.h>
#include <cvi_comm_venc.h>
#include <cvi_comm_vdec.h>
#include <cvi_comm_region.h>
#include "cvi_comm_adec.h"
#include "cvi_comm_aenc.h"
#include "cvi_comm_aio.h"
#include "cvi_audio.h"
#include "cvi_defines.h"
//#include "cvi_mipi.h"

#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_vpss.h"
#include "cvi_vo.h"
#include "cvi_isp.h"
#include "cvi_venc.h"
#include "cvi_vdec.h"
#include "cvi_gdc.h"
#include "cvi_region.h"
#include "cvi_bin.h"
#include "cvi_sensor.h"
#include "sensor_cfg.h"
#include "md5sum.h"
#include "cvi_datafifo.h"
#include "cvi_ipcmsg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define FILE_NAME_LEN 128
#define MAX_NUM_INSTANCE 4
#define NUM_OF_USER_DATA_BUF 4
typedef unsigned int combo_dev_t;
extern int cur_sns_num;

#define CHECK_CHN_RET(express, Chn, name)                                                                              \
	do {                                                                                                           \
		CVI_S32 Ret;                                                                                           \
		Ret = express;                                                                                         \
		if (Ret != CVI_SUCCESS) {                                                                              \
			printf("\033[0;31m%s chn %d failed at %s: LINE: %d with %#x!\033[0;39m\n", name, Chn,          \
			       __func__, __LINE__, Ret);                                                               \
			fflush(stdout);                                                                                \
			return Ret;                                                                                    \
		}                                                                                                      \
	} while (0)

#define CHECK_RET(express, name)                                                                                       \
	do {                                                                                                           \
		CVI_S32 Ret;                                                                                           \
		Ret = express;                                                                                         \
		if (Ret != CVI_SUCCESS) {                                                                              \
			printf("\033[0;31m%s failed at %s: LINE: %d with %#x!\033[0;39m\n", name, __func__,            \
			       __LINE__, Ret);                                                                         \
			return Ret;                                                                                    \
		}                                                                                                      \
	} while (0)

#define SAMPLE_PIXEL_FORMAT VI_PIXEL_FORMAT

#define COLOR_RGB_RED RGB_8BIT(0xFF, 0, 0)			/* 8-bit color depth of red */
#define COLOR_RGB_GREEN RGB_8BIT(0, 0xFF, 0)		/* 8-bit color depth of green */
#define COLOR_RGB_BLUE RGB_8BIT(0, 0, 0xFF)			/* 8-bit color depth of blue */
#define COLOR_RGB_BLACK RGB_8BIT(0, 0, 0)			/* 8-bit color depth of black */
#define COLOR_RGB_YELLOW RGB_8BIT(0xFF, 0xFF, 0)	/* 8-bit color depth of yellow */
#define COLOR_RGB_CYN RGB_8BIT(0, 0xFF, 0xFF)		/* 8-bit color depth of cyan */
#define COLOR_RGB_WHITE RGB_8BIT(0xFF, 0xFF, 0xFF)	/* 8-bit color depth of white */

#define COLOR_10_RGB_RED RGB(0x3FF, 0, 0)			/* 10-bit color depth of red */
#define COLOR_10_RGB_GREEN RGB(0, 0x3FF, 0)			/* 10-bit color depth of green */
#define COLOR_10_RGB_BLUE RGB(0, 0, 0x3FF)			/* 10-bit color depth of blue */
#define COLOR_10_RGB_BLACK RGB(0, 0, 0)				/* 10-bit color depth of black */
#define COLOR_10_RGB_YELLOW RGB(0x3FF, 0x3FF, 0)	/* 10-bit color depth of yellow */
#define COLOR_10_RGB_CYN RGB(0, 0x3FF, 0x3FF)		/* 10-bit color depth of cyan */
#define COLOR_10_RGB_WHITE RGB(0x3FF, 0x3FF, 0x3FF)	/* 10-bit color depth of white */

#define SAMPLE_AUDIO_EXTERN_AI_DEV 0
#define SAMPLE_AUDIO_EXTERN_AO_DEV 0
#define SAMPLE_AUDIO_INNER_AI_DEV 0
#define SAMPLE_AUDIO_INNER_AO_DEV 0
#define SAMPLE_AUDIO_INNER_HDMI_AO_DEV 1
#define SAMPLE_AUDIO_PTNUMPERFRM 480

#define WDR_MAX_PIPE_NUM 6 //need checking by jammy
#define ISP_MAX_DEV_NUM     5

#define __FILENAM__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define MAX_STRING_LEN 255

#define PAUSE()                                                                                                        \
	do {                                                                                                           \
		printf("---------------press Enter key to exit!---------------\n");                                    \
		getchar();                                                                                             \
	} while (0)

#define SAMPLE_PRT(fmt...)                                                                                             \
	do {                                                                                                           \
		printf("[%s]-%d: ", __func__, __LINE__);                                                               \
		printf(fmt);                                                                                           \
	} while (0)

#define CHECK_NULL_PTR(ptr)                                                                                            \
	do {                                                                                                           \
		if (ptr == NULL) {                                                                                     \
			printf("func:%s,line:%d, NULL pointer\n", __func__, __LINE__);                                 \
			return CVI_FAILURE;                                                                            \
		}                                                                                                      \
	} while (0)

#if 0
#define API_COST_TIME_LOG(func, fmt, ...) \
	({ \
		int ret;\
		struct timespec start; \
		struct timespec end;\
		long long elapsed_us;\
		clock_gettime(CLOCK_MONOTONIC, &start); \
		ret = func;\
		clock_gettime(CLOCK_MONOTONIC, &end);\
		elapsed_us = (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_nsec - start.tv_nsec) / 1000LL;\
		printf("func %s time diff:%lld\n", fmt, elapsed_us);\
		ret;\
	} )
#else
#define API_COST_TIME_LOG(func, fmt, ...) \
	({ \
		int ret;\
		ret = func;\
		ret;\
	} )
#endif
#define ALIGN_BASE(val, base)	(((val) + ((base)-1)) & ~((base)-1))

/*******************************************************
 *  enum define
 *******************************************************/
enum CVI_GPIO_NUM_E {
#if defined(__CV181X__) || defined(__CV180X__)
	CVI_GPIOE_00 = 352,
	CVI_GPIOE_01,   CVI_GPIOE_02,   CVI_GPIOE_03,   CVI_GPIOE_04,   CVI_GPIOE_05,
	CVI_GPIOE_06,   CVI_GPIOE_07,   CVI_GPIOE_08,   CVI_GPIOE_09,   CVI_GPIOE_10,
	CVI_GPIOE_11,   CVI_GPIOE_12,   CVI_GPIOE_13,   CVI_GPIOE_14,   CVI_GPIOE_15,
	CVI_GPIOE_16,   CVI_GPIOE_17,   CVI_GPIOE_18,   CVI_GPIOE_19,   CVI_GPIOE_20,
	CVI_GPIOE_21,   CVI_GPIOE_22,   CVI_GPIOE_23,   CVI_GPIOE_24,   CVI_GPIOE_25,
	CVI_GPIOE_26,   CVI_GPIOE_27,   CVI_GPIOE_28,   CVI_GPIOE_29,   CVI_GPIOE_30,
	CVI_GPIOE_31,
	CVI_GPIOD_00 = 384,
	CVI_GPIOD_01,   CVI_GPIOD_02,   CVI_GPIOD_03,   CVI_GPIOD_04,   CVI_GPIOD_05,
	CVI_GPIOD_06,   CVI_GPIOD_07,   CVI_GPIOD_08,   CVI_GPIOD_09,   CVI_GPIOD_10,
	CVI_GPIOD_11,   CVI_GPIOD_12,   CVI_GPIOD_13,   CVI_GPIOD_14,   CVI_GPIOD_15,
	CVI_GPIOD_16,   CVI_GPIOD_17,   CVI_GPIOD_18,   CVI_GPIOD_19,   CVI_GPIOD_20,
	CVI_GPIOD_21,   CVI_GPIOD_22,   CVI_GPIOD_23,   CVI_GPIOD_24,   CVI_GPIOD_25,
	CVI_GPIOD_26,   CVI_GPIOD_27,   CVI_GPIOD_28,   CVI_GPIOD_29,   CVI_GPIOD_30,
	CVI_GPIOD_31,
#else
	CVI_GPIOD_00 = 404,
	CVI_GPIOD_01,   CVI_GPIOD_02,   CVI_GPIOD_03,   CVI_GPIOD_04,   CVI_GPIOD_05,
	CVI_GPIOD_06,   CVI_GPIOD_07,   CVI_GPIOD_08,   CVI_GPIOD_09,   CVI_GPIOD_10,
	CVI_GPIOD_11,
#endif
	CVI_GPIOC_00 = 416,
	CVI_GPIOC_01,   CVI_GPIOC_02,   CVI_GPIOC_03,   CVI_GPIOC_04,   CVI_GPIOC_05,
	CVI_GPIOC_06,   CVI_GPIOC_07,   CVI_GPIOC_08,   CVI_GPIOC_09,   CVI_GPIOC_10,
	CVI_GPIOC_11,   CVI_GPIOC_12,   CVI_GPIOC_13,   CVI_GPIOC_14,   CVI_GPIOC_15,
	CVI_GPIOC_16,   CVI_GPIOC_17,   CVI_GPIOC_18,   CVI_GPIOC_19,   CVI_GPIOC_20,
	CVI_GPIOC_21,   CVI_GPIOC_22,   CVI_GPIOC_23,   CVI_GPIOC_24,   CVI_GPIOC_25,
	CVI_GPIOC_26,   CVI_GPIOC_27,   CVI_GPIOC_28,   CVI_GPIOC_29,   CVI_GPIOC_30,
	CVI_GPIOC_31,
	CVI_GPIOB_00 = 448,
	CVI_GPIOB_01,   CVI_GPIOB_02,   CVI_GPIOB_03,   CVI_GPIOB_04,   CVI_GPIOB_05,
	CVI_GPIOB_06,   CVI_GPIOB_07,   CVI_GPIOB_08,   CVI_GPIOB_09,   CVI_GPIOB_10,
	CVI_GPIOB_11,   CVI_GPIOB_12,   CVI_GPIOB_13,   CVI_GPIOB_14,   CVI_GPIOB_15,
	CVI_GPIOB_16,   CVI_GPIOB_17,   CVI_GPIOB_18,   CVI_GPIOB_19,   CVI_GPIOB_20,
	CVI_GPIOB_21,   CVI_GPIOB_22,   CVI_GPIOB_23,   CVI_GPIOB_24,   CVI_GPIOB_25,
	CVI_GPIOB_26,   CVI_GPIOB_27,   CVI_GPIOB_28,   CVI_GPIOB_29,   CVI_GPIOB_30,
	CVI_GPIOB_31,
	CVI_GPIOA_00 = 480,
	CVI_GPIOA_01,   CVI_GPIOA_02,   CVI_GPIOA_03,   CVI_GPIOA_04,   CVI_GPIOA_05,
	CVI_GPIOA_06,   CVI_GPIOA_07,   CVI_GPIOA_08,   CVI_GPIOA_09,   CVI_GPIOA_10,
	CVI_GPIOA_11,   CVI_GPIOA_12,   CVI_GPIOA_13,   CVI_GPIOA_14,   CVI_GPIOA_15,
	CVI_GPIOA_16,   CVI_GPIOA_17,   CVI_GPIOA_18,   CVI_GPIOA_19,   CVI_GPIOA_20,
	CVI_GPIOA_21,   CVI_GPIOA_22,   CVI_GPIOA_23,   CVI_GPIOA_24,   CVI_GPIOA_25,
	CVI_GPIOA_26,   CVI_GPIOA_27,   CVI_GPIOA_28,   CVI_GPIOA_29,   CVI_GPIOA_30,
	CVI_GPIOA_31,
};

/*
 * Enumeration of picture sizes supported in the video input system.
 * Each value corresponds to a specific resolution or aspect ratio.
 */
typedef enum _PIC_SIZE_E {
	PIC_CIF,                     /* CIF (Common Intermediate Format) */
	PIC_D1_PAL,                  /* D1 resolution for PAL (720 * 576) */
	PIC_D1_NTSC,                 /* D1 resolution for NTSC (720 * 480) */
	PIC_720P,                    /* 720P resolution (1280 * 720) */
	PIC_1600x1200,               /* Resolution (1600 * 1200) */
	PIC_1080P,                   /* 1080P resolution (1920 * 1080) */
	PIC_1088,                    /* Resolution (1920 * 1088) */
	PIC_1440P,                   /* 1440P resolution (2560 * 1440) */
	PIC_1536P,                   /* Resolution (2048 * 1536) */
	PIC_2304x1296,               /* Resolution (2304 * 1296) */
	PIC_2048x1536,               /* Resolution (2048 * 1536) */
	PIC_2560x1600,               /* Resolution (2560 * 1600) */
	PIC_2560x1944,               /* Resolution (2560 * 1944) */
	PIC_2592x1520,               /* Resolution (2592 * 1520) */
	PIC_2592x1536,               /* Resolution (2592 * 1536) */
	PIC_2592x1944,               /* Resolution (2592 * 1944) */
	PIC_2688x1520,               /* Resolution (2688 * 1520) */
	PIC_2716x1524,               /* Resolution (2716 * 1524) */
	PIC_2880x1620,               /* Resolution (2880 * 1620) */
	PIC_3844x1124,               /* Resolution (3844 * 1124) */
	PIC_3840x2160,               /* 4K resolution (3840 * 2160) */
	PIC_4096x2160,               /* 4K resolution (4096 * 2160) */
	PIC_3000x3000,               /* Resolution (3000 * 3000) */
	PIC_4000x3000,               /* Resolution (4000 * 3000) */
	PIC_3840x8640,               /* 8K resolution (3840 * 8640) */
	PIC_7688x1124,               /* Resolution (7688 * 1124) */
	PIC_640x480,                 /* VGA resolution (640 * 480) */
	PIC_479P,                    /* Resolution (632 * 479) */
	PIC_400x400,                 /* Resolution (400 * 400) */
	PIC_288P,                    /* Resolution (384 * 288) */
	PIC_1280x1024,               /* Resolution (1280 * 1024) */
	PIC_CUSTOMIZE,               /* Custom resolution */
	PIC_BUTT                      /* Placeholder for the end of enumeration */
} PIC_SIZE_E;

#if 0
typedef enum _SNS_TYPE_E {
	/* ------ LINEAR BEGIN ------*/
	BRIGATES_BG0808_MIPI_2M_30FPS_10BIT,
	GCORE_GC02M1_MIPI_2M_30FPS_10BIT,
	GCORE_GC1054_MIPI_1M_30FPS_10BIT,
	GCORE_GC2053_MIPI_2M_30FPS_10BIT,
	GCORE_GC2053_SLAVE_MIPI_2M_30FPS_10BIT,
	GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT,
	GCORE_GC2093_MIPI_2M_30FPS_10BIT,
	GCORE_GC2093_SLAVE_MIPI_2M_30FPS_10BIT,
	GCORE_GC2145_MIPI_2M_12FPS_8BIT,
	GCORE_GC4023_MIPI_4M_30FPS_10BIT,
	GCORE_GC4653_MIPI_4M_30FPS_10BIT,
	GCORE_GC4653_SLAVE_MIPI_4M_30FPS_10BIT,
	NEXTCHIP_N5_1M_2CH_25FPS_8BIT,
	NEXTCHIP_N5_2M_25FPS_8BIT,
	NEXTCHIP_N6_2M_4CH_25FPS_8BIT,
	OV_OS02D10_MIPI_2M_30FPS_10BIT,
	OV_OS02D10_SLAVE_MIPI_2M_30FPS_10BIT,
	OV_OS02K10_SLAVE_MIPI_2M_30FPS_12BIT,
	OV_OS04A10_MIPI_4M_1440P_30FPS_12BIT,
	OV_OS04C10_MIPI_4M_30FPS_12BIT,
	OV_OS04C10_MIPI_4M_1440P_30FPS_12BIT,
	OV_OS04C10_SLAVE_MIPI_4M_30FPS_12BIT,
	OV_OS08A20_MIPI_4M_30FPS_10BIT,
	OV_OS08A20_SLAVE_MIPI_4M_30FPS_10BIT,
	OV_OS08A20_MIPI_5M_30FPS_10BIT,
	OV_OS08A20_SLAVE_MIPI_5M_30FPS_10BIT,
	OV_OS08A20_MIPI_8M_30FPS_10BIT,
	OV_OS08A20_SLAVE_MIPI_8M_30FPS_10BIT,
	OV_OV4689_MIPI_4M_30FPS_10BIT,
	OV_OV6211_MIPI_400P_120FPS_10BIT,
	OV_OV7251_MIPI_480P_120FPS_10BIT,
	PICO384_THERMAL_384X288,
	PICO640_THERMAL_479P,
	PIXELPLUS_PR2020_1M_25FPS_8BIT,
	PIXELPLUS_PR2020_1M_30FPS_8BIT,
	PIXELPLUS_PR2020_2M_25FPS_8BIT,
	PIXELPLUS_PR2020_2M_30FPS_8BIT,
	PIXELPLUS_PR2100_2M_25FPS_8BIT,
	PIXELPLUS_PR2100_2M_2CH_25FPS_8BIT,
	PIXELPLUS_PR2100_2M_4CH_25FPS_8BIT,
	SMS_SC035GS_MIPI_480P_120FPS_12BIT,
	SMS_SC035GS_1L_MIPI_480P_120FPS_10BIT,
	SMS_SC035HGS_MIPI_480P_120FPS_12BIT,
	SMS_SC200AI_MIPI_2M_30FPS_10BIT,
	SMS_SC301IOT_MIPI_3M_30FPS_10BIT,
	SMS_SC401AI_MIPI_3M_30FPS_10BIT,
	SMS_SC401AI_MIPI_4M_30FPS_10BIT,
	SMS_SC500AI_MIPI_4M_30FPS_10BIT,
	SMS_SC500AI_MIPI_5M_30FPS_10BIT,
	SMS_SC501AI_2L_MIPI_5M_30FPS_10BIT,
	SMS_SC531AI_2L_MIPI_5M_30FPS_10BIT,
	SMS_SC850SL_MIPI_8M_30FPS_12BIT,
	SMS_SC3332_MIPI_3M_30FPS_10BIT,
	SMS_SC3335_MIPI_3M_30FPS_10BIT,
	SMS_SC3335_SLAVE_MIPI_3M_30FPS_10BIT,
	SMS_SC3336_MIPI_3M_30FPS_10BIT,
	SMS_SC2331_1L_SLAVE_MIPI_2M_30FPS_10BIT,
	SMS_SC2331_1L_SLAVE1_MIPI_2M_30FPS_10BIT,
	SMS_SC2335_MIPI_2M_30FPS_10BIT,
	SMS_SC2336_SLAVE_MIPI_2M_30FPS_10BIT,
	SMS_SC2336_SLAVE1_MIPI_2M_30FPS_10BIT,
	SMS_SC4210_MIPI_4M_30FPS_12BIT,
	SMS_SC4336_MIPI_4M_30FPS_10BIT,
	SMS_SC8238_MIPI_8M_30FPS_10BIT,
	SOI_F23_MIPI_2M_30FPS_10BIT,
	SOI_F35_MIPI_2M_30FPS_10BIT,
	SOI_F35_SLAVE_MIPI_2M_30FPS_10BIT,
	SOI_F37P_MIPI_2M_30FPS_10BIT,
	SOI_H65_MIPI_1M_30FPS_10BIT,
	SOI_K06_MIPI_4M_25FPS_10BIT,
	SOI_Q03_MIPI_3M_30FPS_10BIT,
	SONY_IMX290_MIPI_1M_30FPS_12BIT,
	SONY_IMX290_MIPI_2M_60FPS_12BIT,
	SONY_IMX307_MIPI_2M_30FPS_12BIT,
	SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT,
	SONY_IMX307_2L_MIPI_2M_30FPS_12BIT,
	SONY_IMX307_SUBLVDS_2M_30FPS_12BIT,
	SONY_IMX307_MIPI_2M_60FPS_12BIT,
	SONY_IMX307_SUBLVDS_2M_60FPS_12BIT,
	SONY_IMX327_MIPI_2M_30FPS_12BIT,
	SONY_IMX327_SLAVE_MIPI_2M_30FPS_12BIT,
	SONY_IMX327_2L_MIPI_2M_30FPS_12BIT,
	SONY_IMX327_SUBLVDS_2M_30FPS_12BIT,
	SONY_IMX327_MIPI_2M_60FPS_12BIT,
	SONY_IMX334_MIPI_8M_30FPS_12BIT,
	SONY_IMX335_MIPI_4M_30FPS_12BIT,
	SONY_IMX335_MIPI_4M_1600P_30FPS_12BIT,
	SONY_IMX335_2L_MIPI_4M_30FPS_10BIT,
	SONY_IMX335_MIPI_5M_30FPS_12BIT,
	SONY_IMX335_MIPI_2M_60FPS_10BIT,
	SONY_IMX335_MIPI_4M_60FPS_10BIT,
	SONY_IMX335_MIPI_5M_60FPS_10BIT,
	SONY_IMX347_MIPI_4M_60FPS_12BIT,
	SONY_IMX385_MIPI_2M_30FPS_12BIT,
	TECHPOINT_TP2850_MIPI_2M_30FPS_8BIT,
	TECHPOINT_TP2850_MIPI_4M_30FPS_8BIT,
	VIVO_MCS369_2M_30FPS_12BIT,
	VIVO_MCS369Q_4M_30FPS_12BIT,
	VIVO_MM308M2_2M_25FPS_8BIT,
	SMS_SC1336_2L_MIPI_1M_30FPS_10BIT,
	SMS_SC1336_2L_MIPI_1M_60FPS_10BIT,
	SMS_SC1336_2L_SLAVE_MIPI_1M_30FPS_10BIT,
	/* ------ LINEAR END ------*/
	SNS_TYPE_LINEAR_BUTT,

	/* ------ WDR 2TO1 BEGIN ------*/
	BRIGATES_BG0808_MIPI_2M_30FPS_10BIT_WDR2TO1 = SNS_TYPE_LINEAR_BUTT,
	GCORE_GC2093_MIPI_2M_30FPS_10BIT_WDR2TO1,
	GCORE_GC2093_SLAVE_MIPI_2M_30FPS_10BIT_WDR2TO1,
	OV_OS04A10_MIPI_4M_1440P_30FPS_10BIT_WDR2TO1,
	OV_OS04C10_MIPI_4M_30FPS_10BIT_WDR2TO1,
	OV_OS04C10_MIPI_4M_1440P_30FPS_10BIT_WDR2TO1,
	OV_OS04C10_SLAVE_MIPI_4M_30FPS_10BIT_WDR2TO1,
	OV_OS08A20_MIPI_4M_30FPS_10BIT_WDR2TO1,
	OV_OS08A20_SLAVE_MIPI_4M_30FPS_10BIT_WDR2TO1,
	OV_OS08A20_MIPI_5M_30FPS_10BIT_WDR2TO1,
	OV_OS08A20_SLAVE_MIPI_5M_30FPS_10BIT_WDR2TO1,
	OV_OS08A20_MIPI_8M_30FPS_10BIT_WDR2TO1,
	OV_OS08A20_SLAVE_MIPI_8M_30FPS_10BIT_WDR2TO1,
	SMS_SC200AI_MIPI_2M_30FPS_10BIT_WDR2TO1,
	SMS_SC500AI_MIPI_4M_30FPS_10BIT_WDR2TO1,
	SMS_SC500AI_MIPI_5M_30FPS_10BIT_WDR2TO1,
	SMS_SC850SL_MIPI_8M_30FPS_10BIT_WDR2TO1,
	SMS_SC4210_MIPI_4M_30FPS_10BIT_WDR2TO1,
	SMS_SC8238_MIPI_8M_15FPS_10BIT_WDR2TO1,
	SOI_F35_MIPI_2M_30FPS_10BIT_WDR2TO1,
	SOI_F35_SLAVE_MIPI_2M_30FPS_10BIT_WDR2TO1,
	SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1,
	SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1,
	SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT_WDR2TO1,
	SONY_IMX307_SUBLVDS_2M_30FPS_12BIT_WDR2TO1,
	SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1,
	SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1,
	SONY_IMX327_SLAVE_MIPI_2M_30FPS_12BIT_WDR2TO1,
	SONY_IMX327_SUBLVDS_2M_30FPS_12BIT_WDR2TO1,
	SONY_IMX334_MIPI_8M_30FPS_12BIT_WDR2TO1,
	SONY_IMX335_MIPI_2M_30FPS_10BIT_WDR2TO1,
	SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1,
	SONY_IMX335_MIPI_4M_1600P_30FPS_10BIT_WDR2TO1,
	SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1,
	SONY_IMX347_MIPI_4M_30FPS_12BIT_WDR2TO1,
	SONY_IMX385_MIPI_2M_30FPS_12BIT_WDR2TO1,
	SMS_SC1336_2L_MIPI_1M_30FPS_10BIT_WDR2TO1,
	SMS_SC1336_2L_MIPI_1M_60FPS_10BIT_WDR2TO1,
	/* ------ WDR 2TO1 END ------*/
	SNS_TYPE_WDR_BUTT,
} SNS_TYPE_E;
#endif
/*
 * Structure representing the clock attributes for a sample sensor.
 * Contains the clock frequency and enable status.
 */
typedef struct _SAMPLE_SENSOR_MCLK_ATTR_S {
	CVI_U8 u8Mclk;        /* Clock frequency in MHz */
	CVI_BOOL bMclkEn;     /* Enable status of the clock */
} SAMPLE_SENSOR_MCLK_ATTR_S;

/*
 * Structure representing information about a sample sensor.
 * Includes sensor type, ID, I2C address, and various configuration parameters.
 */
typedef struct _SAMPLE_SENSOR_INFO_S {
	SNS_TYPE_E enSnsType;              /* Type of the sensor */
	CVI_S32 s32SnsId;                  /* Sensor identifier */
	CVI_S32 s32BusId;                  /* Bus identifier for the sensor */
	CVI_S32 s32SnsI2cAddr;             /* I2C address of the sensor */
	combo_dev_t MipiDev;               /* MIPI device configuration */
	CVI_S16 as16LaneId[5];             /* Array of lane IDs (up to 5 lanes) */
	CVI_S8  as8PNSwap[5];              /* Array for pin swap configuration (up to 5 pins) */
	CVI_U8  u8HwSync;                  /* Hardware synchronization signal */
	SAMPLE_SENSOR_MCLK_ATTR_S stMclkAttr; /* Clock attributes structure */
	CVI_U8 u8Orien;                    /* Orientation: 0 = normal, 1 = mirror, 2 = flip, 3 = mirror and flip */
	CVI_U8 u8MuxDev;                   /* Multiplexer device identifier */
	CVI_S16 s16SwitchPort;             /* Switch port configuration */
	CVI_S16 s16SwitchGpio;             /* GPIO for switch control */
	CVI_S16 s16SwitchPol;              /* Polarity for switch control */
	CVI_S32 s32RstPort;                /* Reset port configuration */
	CVI_S32 s32RstPin;                 /* Reset pin configuration */
	CVI_S32 s32RstPol;                 /* Polarity for reset signal */
	CVI_FLOAT f32FpsRate;              /* Frame rate in frames per second */
} SAMPLE_SENSOR_INFO_S;


#if 0
typedef enum _SAMPLE_VO_MODE_E {
	VO_MODE_1MUX,
	VO_MODE_2MUX,
	VO_MODE_4MUX,
	VO_MODE_8MUX,
	VO_MODE_9MUX,
	VO_MODE_16MUX,
	VO_MODE_25MUX,
	VO_MODE_36MUX,
	VO_MODE_49MUX,
	VO_MODE_64MUX,
	VO_MODE_2X4,
	VO_MODE_BUTT
} SAMPLE_VO_MODE_E;
#endif
typedef enum _SAMPLE_RC_E {
	SAMPLE_RC_CBR = 0,
	SAMPLE_RC_VBR,
	SAMPLE_RC_AVBR,
	SAMPLE_RC_QVBR,
	SAMPLE_RC_FIXQP,
	SAMPLE_RC_QPMAP,
	SAMPLE_RC_UBR,
	SAMPLE_RC_MAX
} SAMPLE_RC_E;

/*******************************************************
 *   structure define
 *******************************************************/
/*
 * Structure representing snapshot information for a sample sensor.
 * Contains flags and configurations for video and snapshot processing.
 */
typedef struct _SAMPLE_SNAP_INFO_S {
	bool bSnap;                        /* Flag indicating if snapshot mode is enabled */
	bool bDoublePipe;                  /* Flag indicating if double pipe mode is used */
	VI_PIPE VideoPipe;                 /* Video processing pipeline identifier */
	VI_PIPE SnapPipe;                  /* Snapshot processing pipeline identifier */
	VI_VPSS_MODE_E enVideoPipeMode;    /* Video pipe mode configuration */
	VI_VPSS_MODE_E enSnapPipeMode;     /* Snapshot pipe mode configuration */
} SAMPLE_SNAP_INFO_S;

/*
 * Structure representing device information for a sample video input device.
 * Includes device identifier and WDR (Wide Dynamic Range) mode configuration.
 */
typedef struct _SAMPLE_DEV_INFO_S {
	VI_DEV ViDev;                      /* Video input device identifier */
	WDR_MODE_E enWDRMode;             /* Wide Dynamic Range mode configuration */
} SAMPLE_DEV_INFO_S;


/*
 * Structure representing information about video input pipes.
 * Contains pipe configurations, modes, and flags for multi-pipe and ISP bypass.
 */
typedef struct _SAMPLE_PIPE_INFO_S {
	VI_PIPE aPipe[WDR_MAX_PIPE_NUM];     /* Array of video input pipes (up to WDR_MAX_PIPE_NUM) */
	VI_VPSS_MODE_E enMastPipeMode;        /* Master pipe mode configuration */
	bool bMultiPipe;                       /* Flag indicating if multi-pipe mode is enabled */
	bool bVcNumCfged;                     /* Flag indicating if VC (Virtual Channel) numbers are configured */
	bool bIspBypass;                      /* Flag indicating if ISP (Image Signal Processing) is bypassed */
	PIXEL_FORMAT_E enPixFmt;              /* Pixel format for the video input */
	CVI_U32 u32VCNum[WDR_MAX_PIPE_NUM];   /* Array of VC numbers for each pipe */
} SAMPLE_PIPE_INFO_S;

/*
 * Structure representing channel information for a video input.
 * Includes format, dynamic range, video format, and compression mode.
 */
typedef struct _SAMPLE_CHN_INFO_S {
	VI_CHN ViChn;                         /* Video input channel identifier */
	PIXEL_FORMAT_E enPixFormat;           /* Pixel format for the channel */
	DYNAMIC_RANGE_E enDynamicRange;       /* Dynamic range configuration */
	VIDEO_FORMAT_E enVideoFormat;         /* Video format configuration */
	COMPRESS_MODE_E enCompressMode;       /* Compression mode configuration */
} SAMPLE_CHN_INFO_S;

/*
 * Structure representing comprehensive video input information.
 * Combines sensor, device, pipe, channel, and snapshot information.
 */
typedef struct _SAMPLE_VI_INFO_S {
	SAMPLE_SENSOR_INFO_S stSnsInfo;      /* Sensor information structure */
	SAMPLE_DEV_INFO_S stDevInfo;          /* Device information structure */
	SAMPLE_PIPE_INFO_S stPipeInfo;        /* Pipe information structure */
	SAMPLE_CHN_INFO_S stChnInfo;          /* Channel information structure */
	SAMPLE_SNAP_INFO_S stSnapInfo;        /* Snapshot information structure */
} SAMPLE_VI_INFO_S;

/*
 * Structure representing the configuration for video input systems.
 * Contains an array of video input information and working device IDs.
 */
typedef struct _SAMPLE_VI_CONFIG_S {
	SAMPLE_VI_INFO_S astViInfo[VI_MAX_DEV_NUM]; /* Array of video input information (up to VI_MAX_DEV_NUM) */
	CVI_S32 as32WorkingViId[VI_MAX_DEV_NUM];    /* Array of working video input device IDs */
	CVI_S32 s32WorkingViNum;                      /* Number of working video input devices */
	CVI_BOOL bViRotation;                         /* Flag indicating if video input rotation is enabled */
} SAMPLE_VI_CONFIG_S;

/*
 * Structure representing configuration for video input frames.
 * Includes dimensions, pixel format, video format, and dynamic range.
 */
typedef struct _SAMPLE_VI_FRAME_CONFIG_S {
	CVI_U32 u32Width;                          /* Width of the video frame */
	CVI_U32 u32Height;                         /* Height of the video frame */
	CVI_U32 u32ByteAlign;                      /* Byte alignment for the frame data */
	PIXEL_FORMAT_E enPixelFormat;              /* Pixel format for the frame */
	VIDEO_FORMAT_E enVideoFormat;              /* Video format for the frame */
	COMPRESS_MODE_E enCompressMode;            /* Compression mode for the frame */
	DYNAMIC_RANGE_E enDynamicRange;            /* Dynamic range for the frame */
} SAMPLE_VI_FRAME_CONFIG_S;

/*
 * Structure representing information about video input frames.
 * Contains buffer block, size, and associated video frame information.
 */
typedef struct _SAMPLE_VI_FRAME_INFO_S {
	VB_BLK VbBlk;                              /* Video buffer block */
	CVI_U32 u32Size;                           /* Size of the video frame data */
	VIDEO_FRAME_INFO_S stVideoFrameInfo;      /* Structure containing video frame information */
} SAMPLE_VI_FRAME_INFO_S;

/*
 * Structure representing information for FPN (Fixed Pattern Noise) calibration.
 * Includes threshold, frame count, pixel format, and compression mode.
 */
typedef struct _SAMPLE_VI_FPN_CALIBRATE_INFO_S {
	CVI_U32 u32Threshold;                     /* Threshold for FPN calibration */
	CVI_U32 u32FrameNum;                      /* Number of frames for calibration */
	//ISP_FPN_TYPE_E enFpnType;                /* Type of FPN (commented out) */
	PIXEL_FORMAT_E enPixelFormat;             /* Pixel format for FPN calibration */
	COMPRESS_MODE_E enCompressMode;           /* Compression mode for FPN calibration */
} SAMPLE_VI_FPN_CALIBRATE_INFO_S;

/*
 * Structure representing information for FPN correction.
 * Includes operation type, strength, pixel format, and associated frame information.
 */
typedef struct _SAMPLE_VI_FPN_CORRECTION_INFO_S {
	ISP_OP_TYPE_E enOpType;                   /* Operation type for FPN correction */
	//ISP_FPN_TYPE_E enFpnType;                /* Type of FPN (commented out) */
	CVI_U32 u32Strength;                       /* Strength of the FPN correction */
	PIXEL_FORMAT_E enPixelFormat;             /* Pixel format for FPN correction */
	COMPRESS_MODE_E enCompressMode;           /* Compression mode for FPN correction */
	SAMPLE_VI_FRAME_INFO_S stViFrameInfo;    /* Frame information for FPN correction */
} SAMPLE_VI_FPN_CORRECTION_INFO_S;


typedef struct _SAMPLE_COMM_VO_LAYER_CONFIG_S {
	/* for layer */
	VO_LAYER VoLayer;
	VO_INTF_SYNC_E enIntfSync;
	RECT_S stDispRect;
	SIZE_S stImageSize;
	PIXEL_FORMAT_E enPixFormat;

	CVI_U32 u32DisBufLen;

	/* for chn */
	VO_MODE_E enVoMode;
} SAMPLE_COMM_VO_LAYER_CONFIG_S;

typedef struct _SAMPLE_VO_CONFIG_S {
	/* for device */
	VO_DEV VoDev;
	VO_PUB_ATTR_S stVoPubAttr;
	PIC_SIZE_E enPicSize;

	/* for layer */
	PIXEL_FORMAT_E enPixFormat;
	RECT_S stDispRect;
	SIZE_S stImageSize;

	CVI_U32 u32DisBufLen;

	/* for channel */
	VO_MODE_E enVoMode;
} SAMPLE_VO_CONFIG_S;

typedef enum _THREAD_CONTRL_E {
	THREAD_CTRL_START,
	THREAD_CTRL_PAUSE,
	THREAD_CTRL_STOP,
} THREAD_CONTRL_E;

typedef struct _VDEC_THREAD_PARAM_S {
	CVI_S32 s32ChnId;
	PAYLOAD_TYPE_E enType;
	CVI_CHAR cFilePath[128];
	CVI_CHAR cFileName[128];
	CVI_S32 s32StreamMode;
	CVI_S32 s32MilliSec;
	CVI_S32 s32MinBufSize;
	CVI_S32 s32IntervalTime;
	THREAD_CONTRL_E eThreadCtrl;
	CVI_U64 u64PtsInit;
	CVI_U64 u64PtsIncrease;
	CVI_BOOL bCircleSend;
	CVI_BOOL bFileEnd;
	CVI_BOOL bDumpYUV;
	MD5_CTX tMD5Ctx;
} VDEC_THREAD_PARAM_S;

typedef struct _SAMPLE_VDEC_BUF {
	CVI_U32  u32PicBufSize;
	CVI_U32  u32TmvBufSize;
	CVI_BOOL bPicBufAlloc;
	CVI_BOOL bTmvBufAlloc;
} SAMPLE_VDEC_BUF;

typedef struct _SAMPLE_VDEC_VIDEO_ATTR {
	VIDEO_DEC_MODE_E enDecMode;
	CVI_U32              u32RefFrameNum;
	DATA_BITWIDTH_E  enBitWidth;
} SAMPLE_VDEC_VIDEO_ATTR;

typedef struct _SAMPLE_VDEC_PICTURE_ATTR {
	CVI_U32         u32Alpha;
} SAMPLE_VDEC_PICTURE_ATTR;

typedef struct _SAMPLE_VDEC_ATTR {
	PAYLOAD_TYPE_E enType;
	PIXEL_FORMAT_E enPixelFormat;
	VIDEO_MODE_E   enMode;
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	CVI_U32 u32FrameBufCnt;
	CVI_U32 u32DisplayFrameNum;
	union {
		SAMPLE_VDEC_VIDEO_ATTR stSampleVdecVideo;      /* structure with video ( h265/h264) */
		SAMPLE_VDEC_PICTURE_ATTR stSampleVdecPicture; /* structure with picture (jpeg/mjpeg )*/
	};
} SAMPLE_VDEC_ATTR;

typedef struct _vdecChnCtx_ {
	VDEC_THREAD_PARAM_S stVdecThreadParamSend;
	VDEC_THREAD_PARAM_S stVdecThreadParamGet;
	SAMPLE_VDEC_ATTR stSampleVdecAttr;
	pthread_t vdecThreadSend;
	pthread_t vdecThreadGet;
	VDEC_CHN VdecChn;
	CVI_S32 bCreateChn;
} vdecChnCtx;

extern SNS_TYPE_E g_enSnsType[VI_MAX_DEV_NUM];

typedef struct SAMPLE_VENC_GETSTREAM_PARA_S {
	CVI_BOOL bThreadStart;
	VENC_CHN VeChn[VENC_MAX_CHN_NUM];
	CVI_S32  s32Cnt;
} SAMPLE_VENC_GETSTREAM_PARA_S;

typedef struct _commonInputCfg_ {
	CVI_U32 testMode;
	CVI_S32 numChn;
	CVI_S32 ifInitVb;
	CVI_U32 bindmode;
	CVI_U32 u32ViWidth;		// frame width of VI input or VPSS input
	CVI_U32 u32ViHeight;	// frame height of VI input or VPSS input
	CVI_U32 u32VpssWidth;	// frame width of VPSS output
	CVI_U32 u32VpssHeight;	// frame height of VPSS output
	CVI_CHAR yuvFolder[MAX_STRING_LEN];
	CVI_S32 vbMode;
	CVI_S32 bSingleEsBuf_jpege;
	CVI_S32 bSingleEsBuf_h264e;
	CVI_S32 bSingleEsBuf_h265e;
	CVI_S32 singleEsBufSize_jpege;
	CVI_S32 singleEsBufSize_h264e;
	CVI_S32 singleEsBufSize_h265e;
	CVI_S32 h265RefreshType;
	CVI_S32 jpegMarkerOrder;
	CVI_BOOL bThreadDisable;
} commonInputCfg;

typedef struct _chnInputCfg_ {
	char codec[64];
	unsigned int width;
	unsigned int height;
	char input_path[MAX_STRING_LEN];
	char vpssSrcPath[MAX_STRING_LEN];
	char output_path[MAX_STRING_LEN];
	char outputFileName[MAX_STRING_LEN];
	char roiCfgFile[MAX_STRING_LEN];
	char qpMapCfgFile[MAX_STRING_LEN];
	char user_data[NUM_OF_USER_DATA_BUF][MAX_STRING_LEN];
	CVI_S32 num_frames;
	CVI_S32 bsMode;
	CVI_U32 u32Profile;
	CVI_S32 rcMode;
	CVI_S32 iqp;
	CVI_S32 pqp;
	CVI_S32 gop;
	CVI_U32 gopMode;
	CVI_S32 bitrate;
	CVI_S32 minIprop;
	CVI_S32 maxIprop;
	CVI_U32 u32RowQpDelta;
	CVI_S32 firstFrmstartQp;
	CVI_S32 minIqp;
	CVI_S32 maxIqp;
	CVI_S32 minQp;
	CVI_S32 maxQp;
	CVI_S32 framerate;
	CVI_S32 quality;
	CVI_S32 maxbitrate;
	CVI_S32 s32ChangePos;
	CVI_S32 s32MinStillPercent;
	CVI_U32 u32MaxStillQP;
	CVI_U32 u32MotionSensitivity;
	CVI_S32	s32AvbrFrmLostOpen;
	CVI_S32 s32AvbrFrmGap;
	CVI_S32 s32AvbrPureStillThr;
	CVI_S32 statTime;
	CVI_S32 bind_mode;
	CVI_S32 pixel_format;
	CVI_S32 posX;
	CVI_S32 posY;
	CVI_S32 inWidth;
	CVI_S32 inHeight;
	CVI_S32 srcFramerate;
	CVI_U32 bitstreamBufSize;
	CVI_S32 single_LumaBuf;
	CVI_S32 single_core;
	CVI_S32 vpssGrp;
	CVI_S32 vpssChn;
	CVI_S32 forceIdr;
	CVI_S32 chgNum;
	CVI_S32 chgBitrate;
	CVI_S32 chgFramerate;
	CVI_S32 tempLayer;
	CVI_S32 testRoi;
	CVI_S32 bgInterval;
	CVI_S32 frameLost;
	CVI_U32 frameLostGap;
	CVI_U32 frameLostBspThr;
	CVI_S32 MCUPerECS;
	CVI_S32 bCreateChn;
	CVI_S32 getstream_timeout;
	CVI_S32 sendframe_timeout;
	CVI_S32 s32IPQpDelta;
	CVI_S32 s32BgQpDelta;
	CVI_S32 s32ViQpDelta;
	CVI_S32 bVariFpsEn;
	CVI_S32 initialDelay;
	CVI_U32 u32IntraCost;
	CVI_U32 u32ThrdLv;
	CVI_BOOL bBgEnhanceEn;
	CVI_S32 s32BgDeltaQp;
	CVI_U32 h264EntropyMode;
	CVI_S32 h264ChromaQpOffset;
	CVI_S32 h265CbQpOffset;
	CVI_S32 h265CrQpOffset;
	CVI_U32 enSuperFrmMode;
	CVI_U32 u32SuperIFrmBitsThr;
	CVI_U32 u32SuperPFrmBitsThr;
	CVI_S32 s32MaxReEncodeTimes;

	CVI_U8 aspectRatioInfoPresentFlag;
	CVI_U8 aspectRatioIdc;
	CVI_U8 overscanInfoPresentFlag;
	CVI_U8 overscanAppropriateFlag;
	CVI_U16 sarWidth;
	CVI_U16 sarHeight;

	CVI_U8 timingInfoPresentFlag;
	CVI_U8 fixedFrameRateFlag;
	CVI_U32 numUnitsInTick;
	CVI_U32 timeScale;

	CVI_U8 videoSignalTypePresentFlag;
	CVI_U8 videoFormat;
	CVI_U8 videoFullRangeFlag;
	CVI_U8 colourDescriptionPresentFlag;
	CVI_U8 colourPrimaries;
	CVI_U8 transferCharacteristics;
	CVI_U8 matrixCoefficients;

	CVI_U32 u32FrameQp;
	CVI_BOOL bTestUbrEn;

	CVI_BOOL bEsBufQueueEn;
	CVI_BOOL bIsoSendFrmEn;
	CVI_BOOL bSensorEn;

	CVI_U32 u32SliceCnt;
	CVI_U8 bDisableDeblk;
	CVI_S32 betaOffset;
	CVI_S32 alphaOffset;
	CVI_BOOL bIntraPred;
	CVI_S32 maxQuality;
	CVI_S32 minQuality;
	CVI_U32 u32ResetGop;

	CVI_BOOL svc_enable;
	CVI_BOOL fg_protect_en;
	CVI_S32  fg_dealt_qp;
	CVI_BOOL complex_scene_detect_en;
	CVI_U32  complex_scene_low_th;
	CVI_U32  complex_scene_hight_th;
	CVI_U32  middle_min_percent;
	CVI_U32  complex_min_percent;
	CVI_BOOL smart_ai_en;
} chnInputCfg;

typedef enum _CHN_STATE_ {
	CHN_STAT_NONE = 0,
	CHN_STAT_START,
	CHN_STAT_STOP,
} CHN_STATE;

typedef enum _BS_MODE_ {
	BS_MODE_QUERY_STAT = 0,
	BS_MODE_SELECT,
} BS_MODE;

typedef struct _SAMPLE_COMM_VENC_ROI_ATTR_ {
	VENC_ROI_ATTR_S stVencRoiAttr;
	CVI_U32 u32FrameStart;
	CVI_U32 u32FrameEnd;
} SAMPLE_COMM_VENC_ROI;

#define MAX_NUM_ROI 8

typedef struct _vencChnCtx_ {
	VENC_CHN VencChn;
	PIC_SIZE_E enSize;
	SIZE_S stSize;
	VIDEO_FRAME_INFO_S *pstFrameInfo;
	VIDEO_FRAME_S *pstVFrame;
	CVI_U32 u32LumaSize;
	CVI_U32 u32ChrmSize;
	CVI_U32 u32FrameSize;
	CVI_U32 num_frames;
	CVI_S32 s32ChnNum;
	CVI_U32 s32FbCnt;
	CVI_U32 u32Profile;
	PAYLOAD_TYPE_E enPayLoad;
	VENC_GOP_MODE_E enGopMode;
	VENC_GOP_ATTR_S stGopAttr;
	SAMPLE_RC_E enRcMode;
	FILE *fpSrc;
	long file_size;
	FILE *pFile;
	chnInputCfg chnIc;
	PIXEL_FORMAT_E enPixelFormat;
	CHN_STATE chnStat;
	CHN_STATE nextChnStat;
	SAMPLE_COMM_VENC_ROI vencRoi[MAX_NUM_ROI];
	CVI_U8 *pu8QpMap;
	CVI_BOOL bQpMapValid;
	CVI_S32 s32VencFd;
} vencChnCtx;

/*
 * Structure representing the configuration for initializing video input settings.
 * Contains source type, device number, sensor types, and various configurations for multiple sensors.
 */
typedef struct _SAMPLE_INI_CFG_S {
	VI_PIPE_FRAME_SOURCE_E enSource;                     /* Frame source type for the video input */
	CVI_U8 devNum;                                       /* Number of devices configured */
	CVI_U8 u8UseMultiSns;                               /* Flag indicating if multiple sensors are used */

	SNS_TYPE_E enSnsType[VI_MAX_DEV_NUM];               /* Array of sensor types for each device */
	WDR_MODE_E enWDRMode[VI_MAX_DEV_NUM];               /* Array of WDR modes for each device */
	CVI_S32 s32BusId[VI_MAX_DEV_NUM];                   /* Array of bus IDs for each device */
	CVI_S32 s32SnsI2cAddr[VI_MAX_DEV_NUM];              /* Array of I2C addresses for each sensor */
	combo_dev_t MipiDev[VI_MAX_DEV_NUM];                 /* Array of MIPI device configurations */
	CVI_S16 as16LaneId[VI_MAX_DEV_NUM][5];              /* Array of lane IDs for each device (up to 5 lanes) */
	CVI_S8 as8PNSwap[VI_MAX_DEV_NUM][5];                /* Array for pin swap configurations (up to 5 pins) */
	CVI_U8 u8HwSync[VI_MAX_DEV_NUM];                    /* Array of hardware synchronization flags for each device */
	SAMPLE_SENSOR_MCLK_ATTR_S stMclkAttr[VI_MAX_DEV_NUM]; /* Array of sensor MCLK attributes for each device */
	CVI_U8 u8Orien[VI_MAX_DEV_NUM];                      /* Array of orientation configurations for each device */
	CVI_U8 u8MuxDev[VI_MAX_DEV_NUM];                    /* Array of multiplexer device configurations */
	CVI_U8 u8AttachDev[VI_MAX_DEV_NUM];                 /* Array of attached device configurations */
	CVI_S16 s16SwitchPort[VI_MAX_DEV_NUM];              /* Array of switch port configurations for each device */
	CVI_S16 s16SwitchGpio[VI_MAX_DEV_NUM];              /* Array of GPIO configurations for switch control */
	CVI_S16 s16SwitchPol[VI_MAX_DEV_NUM];               /* Array of polarity settings for switch control */
	CVI_S32 s32RstPort[VI_MAX_DEV_NUM];                 /* Array of reset port configurations for each device */
	CVI_S32 s32RstPin[VI_MAX_DEV_NUM];                  /* Array of reset pin configurations for each device */
	CVI_S32 s32RstPol[VI_MAX_DEV_NUM];                  /* Array of reset polarity settings for each device */
	CVI_FLOAT f32FpsRate[VI_MAX_DEV_NUM];               /* Array of frames per second (FPS) rates for each device */
} SAMPLE_INI_CFG_S;


extern RGN_RGBQUARD_S overlay_palette[256];

/********************************************************
 *     function announce
 ********************************************************/
/**
 * @brief get picture size(w*h), according enPicSize
*/
CVI_S32 SAMPLE_COMM_SYS_GetPicSize(PIC_SIZE_E enPicSize, SIZE_S *pstSize);

/**
 * @brief vb exit & MMF system exit
*/
CVI_VOID SAMPLE_COMM_SYS_Exit(void);

/**
 * @brief vb init & MMF system init
*/
CVI_S32 SAMPLE_COMM_SYS_Init(VB_CONFIG_S *pstVbConfig);

CVI_S32 SAMPLE_COMM_VI_CreateIsp(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_DestroyIsp(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_Bind_VO(VI_PIPE ViPipe, VI_CHN ViChn, VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 SAMPLE_COMM_VI_UnBind_VO(VI_PIPE ViPipe, VI_CHN ViChn, VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 SAMPLE_COMM_VI_Bind_VPSS(VI_PIPE ViPipe, VI_CHN ViChn, VPSS_GRP VpssGrp);
CVI_S32 SAMPLE_COMM_VI_UnBind_VPSS(VI_PIPE ViPipe, VI_CHN ViChn, VPSS_GRP VpssGrp);
CVI_S32 SAMPLE_COMM_VI_Bind_VENC(VI_PIPE ViPipe, VI_CHN ViChn, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VI_UnBind_VENC(VI_PIPE ViPipe, VI_CHN ViChn, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VPSS_Bind_VO(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 SAMPLE_COMM_VPSS_UnBind_VO(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 SAMPLE_COMM_VPSS_Bind_VENC(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VPSS_UnBind_VENC(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VDEC_Bind_VPSS(VDEC_CHN VdecChn, VPSS_GRP VpssGrp);
CVI_S32 SAMPLE_COMM_VDEC_UnBind_VPSS(VDEC_CHN VdecChn, VPSS_GRP VpssGrp);
CVI_S32 SAMPLE_COMM_VPSS_Bind_VPSS(VPSS_GRP VpssGrpSrc, VPSS_CHN VpssChnSrc, VPSS_GRP VpssGrpDst);
CVI_S32 SAMPLE_COMM_VPSS_UnBind_VPSS(VPSS_GRP VpssGrpSrc, VPSS_CHN VpssChnSrc, VPSS_GRP VpssGrpDst);
CVI_S32 SAMPLE_COMM_VDEC_Bind_VENC(VDEC_CHN VdecChn, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VDEC_UnBind_VENC(VDEC_CHN VdecChn, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VDEC_Bind_VO(VDEC_CHN VdecChn, VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 SAMPLE_COMM_VDEC_UnBind_VO(VDEC_CHN VdecChn, VO_LAYER VoLayer, VO_CHN VoChn);

CVI_S32 SAMPLE_COMM_ISP_Run(CVI_U8 IspDev);
CVI_VOID SAMPLE_COMM_ISP_Stop(CVI_U8 IspDev);
CVI_VOID SAMPLE_COMM_All_ISP_Stop(void);
CVI_S32 SAMPLE_COMM_ISP_Aelib_Callback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_Aelib_UnCallback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_Awblib_Callback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_Sensor_Regiter_callback(ISP_DEV IspDev, CVI_U32 u32SnsId, CVI_S32 s32BusId,
						CVI_S32 s32I2cAddr);
CVI_S32 SAMPLE_COMM_ISP_Awblib_UnCallback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_Aflib_Callback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_Aflib_UnCallback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_Sensor_UnRegiter_callback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_GetIspAttrBySns(SNS_TYPE_E enSnsType, ISP_PUB_ATTR_S *pstPubAttr);
CVI_S32 SAMPLE_COMM_VI_GetYuvBypassSts(SNS_TYPE_E enSnsType);
//CVI_S32 SAMPLE_COMM_ISP_SetSensorMode(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_ISP_SetSensorMode(SAMPLE_VI_CONFIG_S *pstViConfig, CVI_S32 pstSnsNum);
CVI_S32 SAMPLE_COMM_ISP_SetSnsObj(CVI_U32 u32SnsId, SNS_TYPE_E enSnsType);
CVI_S32 SAMPLE_COMM_ISP_SetSnsInit(CVI_U32 u32SnsId, CVI_U8 u8HwSync);
CVI_S32 SAMPLE_COMM_ISP_PatchSnsObj(CVI_U32 u32SnsId, SAMPLE_SENSOR_INFO_S *pstSnsInfo);
CVI_VOID *SAMPLE_COMM_ISP_GetSnsObj(CVI_U32 u32SnsId);
CVI_VOID *SAMPLE_COMM_GetSnsObj(SNS_TYPE_E enSnsType);

CVI_S32 SAMPLE_AUDIO_DEBUG(void);

CVI_S32 SAMPLE_COMM_VI_GetDevAttrBySns(SNS_TYPE_E enSnsType, VI_DEV_ATTR_S *pstViDevAttr);
void SAMPLE_COMM_VI_GetSensorInfo(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_GetSizeBySensor(SNS_TYPE_E enMode, PIC_SIZE_E *penSize);
CVI_S32 SAMPLE_COMM_VI_GetChnAttrBySns(SNS_TYPE_E enSnsType, VI_CHN_ATTR_S *pstChnAttr);
CVI_S32 SAMPLE_COMM_VI_StartIsp(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_VI_StopIsp(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_VI_StartDev(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_VI_StopDev(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_VI_StartViChn(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_StopViChn(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_VI_StopViPipe(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_VI_DestroyVi(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_OPEN(void);
CVI_S32 SAMPLE_COMM_VI_CLOSE(void);
CVI_S32 SAMPLE_COMM_VI_StartSensor(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_StartMIPI(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_SensorProbe(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_SetIniPath(const CVI_CHAR *iniPath);
CVI_S32 SAMPLE_COMM_VI_ParseIni(SAMPLE_INI_CFG_S *pstIniCfg);
CVI_S32 SAMPLE_COMM_VI_DefaultConfig(void);
CVI_S32 SAMPLE_COMM_VI_IniToViCfg(SAMPLE_INI_CFG_S *pstIniCfg, SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_CHAR *SAMPLE_COMM_VI_GetSnsrTypeName(void);

CVI_S32 SAMPLE_COMM_VPSS_Start(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			       VPSS_CHN_ATTR_S *pastVpssChnAttr);
CVI_S32 SAMPLE_COMM_VPSS_Init(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr);
CVI_S32 SAMPLE_COMM_VPSS_WRAP_Start(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
				VPSS_CHN_ATTR_S *pastVpssChnAttr, VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap);

CVI_S32 SAMPLE_COMM_VPSS_Stop(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable);
CVI_S32 SAMPLE_COMM_VPSS_SendFrame(VPSS_GRP VpssGrp, SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename);

CVI_VOID SAMPLE_COMM_VENC_InitCommonInputCfg(commonInputCfg *pCic);
CVI_VOID SAMPLE_COMM_VENC_InitChnInputCfg(chnInputCfg *pIc);
CVI_S32 SAMPLE_COMM_VENC_SaveStream(PAYLOAD_TYPE_E enType,
		FILE *pFd, VENC_STREAM_S *pstStream);
CVI_S32 SAMPLE_COMM_VENC_SaveChannelStream(vencChnCtx *pvecc);
CVI_S32 SAMPLE_COMM_VENC_Stop(VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_GetGopAttr(VENC_GOP_MODE_E enGopMode, VENC_GOP_ATTR_S *pstGopAttr);
CVI_S32 SAMPLE_COMM_VENC_GetFilePostfix(PAYLOAD_TYPE_E enPayload, char *szFilePostfix);
CVI_S32 SAMPLE_COMM_VENC_InitVBPool(vencChnCtx *pvecc, VENC_CHN VencChnIdx);
CVI_S32 SAMPLE_COMM_VENC_CloseReEncode(VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetJpegParam(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetModParam(const commonInputCfg *pCic);
CVI_S32 SAMPLE_COMM_VENC_SetRoiAttr(VENC_CHN VencChn, PAYLOAD_TYPE_E enType);
CVI_S32 SAMPLE_COMM_VENC_SetQpMapByCfgFile(VENC_CHN VencChn,
		SAMPLE_COMM_VENC_ROI *vencRoi, CVI_U32 frame_idx,
		CVI_U8 *pu8QpMap, CVI_BOOL *pbQpMapValid,
		CVI_U32 u32Width, CVI_U32 u32Height);
CVI_S32 SAMPLE_COMM_VENC_SetRoiAttrByCfgFile(VENC_CHN VencChn, SAMPLE_COMM_VENC_ROI *vencRoi, CVI_U32 frame_idx);
CVI_S32 SAMPLE_COMM_VENC_LoadRoiCfgFile(SAMPLE_COMM_VENC_ROI *vencRoi, CVI_CHAR *cfgFileName);
CVI_S32 SAMPLE_COMM_VENC_SetH264Trans(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH265Trans(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH264Vui(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH265Vui(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH264SliceSplit(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH265SliceSplit(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH264Dblk(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH265Dblk(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH264IntraPred(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetChnParam(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_EnableSvc(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetSvcParam(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetChnParam(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_Start(
		chnInputCfg * pIc,
		VENC_CHN VencChn,
		PAYLOAD_TYPE_E enType,
		PIC_SIZE_E enSize,
		SAMPLE_RC_E enRcMode,
		CVI_U32 u32Profile,
		CVI_BOOL bRcnRefShareBuf,
		VENC_GOP_ATTR_S *pstGopAttr);
CVI_S32 SAMPLE_COMM_VDEC_InitVBPool(VDEC_CHN VdecChn, SAMPLE_VDEC_ATTR *pastSampleVdec);
CVI_VOID SAMPLE_COMM_VDEC_StartSendStream(VDEC_THREAD_PARAM_S *pstVdecSend,
		pthread_t *pVdecThread);
CVI_S32 SAMPLE_COMM_VDEC_SetVBPool(CVI_S32 Chn, CVI_U32 VBPoolID);
CVI_S32 SAMPLE_COMM_VDEC_GetVBPool(CVI_S32 Chn);
CVI_S32 SAMPLE_COMM_VDEC_SetVbMode(CVI_S32 VdecVbSrc);
CVI_S32 SAMPLE_COMM_VDEC_GetVbMode(void);
CVI_S32 SAMPLE_COMM_VDEC_Start(vdecChnCtx *pvdchnCtx);
CVI_VOID SAMPLE_COMM_VDEC_CmdCtrl(VDEC_THREAD_PARAM_S *pstVdecSend, pthread_t *pVdecThread);
CVI_VOID SAMPLE_COMM_VDEC_StopSendStream(VDEC_THREAD_PARAM_S *pstVdecSend, pthread_t *pVdecThread);
CVI_S32 SAMPLE_COMM_VDEC_Stop(CVI_S32 s32ChnNum);
CVI_VOID SAMPLE_COMM_VDEC_ExitVBPool(void);
CVI_VOID SAMPLE_COMM_VDEC_StartGetPic(VDEC_THREAD_PARAM_S *pstVdecGet,
		pthread_t *pVdecThread);
CVI_VOID SAMPLE_COMM_VDEC_StopGetPic(VDEC_THREAD_PARAM_S *pstVdecGet, pthread_t *pVdecThread);

CVI_S32 SAMPLE_COMM_VO_GetWH(VO_INTF_SYNC_E enIntfSync, CVI_U32 *pu32W, CVI_U32 *pu32H, CVI_U32 *pu32Frm);
CVI_S32 SAMPLE_COMM_VO_StartDev(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr);
CVI_S32 SAMPLE_COMM_VO_StopDev(VO_DEV VoDev);
CVI_S32 SAMPLE_COMM_VO_StartLayer(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);
CVI_S32 SAMPLE_COMM_VO_StopLayer(VO_LAYER VoLayer);
CVI_S32 SAMPLE_COMM_VO_StartChn(VO_LAYER VoLayer, VO_MODE_E enMode);
CVI_S32 SAMPLE_COMM_VO_StopChn(VO_LAYER VoLayer, VO_MODE_E enMode);
CVI_S32 SAMPLE_COMM_VO_GetDefConfig(SAMPLE_VO_CONFIG_S *pstVoConfig);
CVI_S32 SAMPLE_COMM_VO_StopVO(SAMPLE_VO_CONFIG_S *pstVoConfig);
CVI_S32 SAMPLE_COMM_VO_StartVO(SAMPLE_VO_CONFIG_S *pstVoConfig);
CVI_VOID SAMPLE_COMM_VO_Exit(void);

/* SAMPLE_COMM_REGION_Create:
 *   Create region.
 *
 * [in] HandleNum: Number of region handle.
 * [in] enType: Type of region.
 * [in] pixelFormat: Pixel format.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_Create(CVI_S32 HandleNum, RGN_TYPE_E enType, PIXEL_FORMAT_E pixelFormat);

/* SAMPLE_COMM_REGION_Destroy:
 *   Destroy region.
 *
 * [in] HandleNum: Number of region handle.
 * [in] enType: Type of region.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_Destroy(CVI_S32 HandleNum, RGN_TYPE_E enType);

/* SAMPLE_COMM_REGION_AttachToChn:
 *   Region apply to chn.
 *
 * [in] HandleNum: Number of region handle.
 * [in] enType: Type of region.
 * [in] pstChn: Module chn.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_AttachToChn(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn);

/* SAMPLE_COMM_REGION_DetachFrmChn:
 *   Cancel region on chn.
 *
 * [in] HandleNum: Number of region handle.
 * [in] enType: Type of region.
 * [in] pstChn: Module chn.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_DetachFrmChn(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn);

/* SAMPLE_COMM_REGION_SetBitMap:
 *   Set bitmap.
 *
 * [in] Handle: RGN ID.
 * [in] filename: Bitmap filename.
 * [in] pixelFormat: Pixel format.
 * [in] bCompressed: Compression or not.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_SetBitMap(RGN_HANDLE Handle, const char *filename,
		PIXEL_FORMAT_E pixelFormat, CVI_BOOL bCompressed);

/* SAMPLE_COMM_REGION_GetUpCanvas:
 *   Get canvas info and update canvas.
 *
 * [in] Handle: RGN ID.
 * [in] filename: Bitmap filename.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_GetUpCanvas(RGN_HANDLE Handle, const char *filename);

/* SAMPLE_COMM_REGION_GetMinHandle:
 *   Get the minimum handle for a region of this type.
 *
 * [in] enType: Type of region.
 * return: MinHandle if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_GetMinHandle(RGN_TYPE_E enType);

/* SAMPLE_COMM_REGION_MST_LoadBmp:
 *   Load the bitmap file.
 *
 * [in] filename: Bitmap filename.
 * [in] pstBitmap: Bitmap attribute.
 * [in] bFil: Filter or not.
 * [in] u16FilColor: Filter color.
 * [in] pixelFormat: Pixel format.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_MST_LoadBmp(const char *filename, BITMAP_S *pstBitmap, CVI_BOOL bFil,
			CVI_U32 u16FilColor, PIXEL_FORMAT_E enPixelFormat);

/* SAMPLE_COMM_REGION_MST_UpdateCanvas:
 *   Update canvas.
 *
 * [in] filename: Bitmap filename.
 * [in] pstBitmap: Bitmap attribute.
 * [in] bFil: Filter or not.
 * [in] u16FilColor: Filter color.
 * [in] pstSize: Canvas size.
 * [in] u32Stride: Stride length.
 * [in] pixelFormat: Pixel format.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_MST_UpdateCanvas(const char *filename, BITMAP_S *pstBitmap, CVI_BOOL bFil,
				CVI_U32 u16FilColor, SIZE_S *pstSize, CVI_U32 u32Stride, PIXEL_FORMAT_E enPixelFormat);

/* SAMPLE_COMM_ODEC_REGION_Create:
 *    Create ODEC region.
 *
 * [in] u32FileSize: File size.
 * [in] stSize: Region size.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_ODEC_REGION_Create(CVI_U32 u32FileSize, SIZE_S *stSize);

/* SAMPLE_COMM_ODEC_REGION_Destroy:
 *   Destroy ODEC region.
 *
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_ODEC_REGION_Destroy(void);

/* SAMPLE_COMM_ODEC_REGION_AttachToChn:
 *   ODEC region apply to chn.
 *
 * [in] pstChn: Module chn.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_ODEC_REGION_AttachToChn(MMF_CHN_S *pstChn);

/* SAMPLE_COMM_ODEC_REGION_DetachFrmChn:
 *   Cancel ODEC region on chn.
 *
 * [in] pstChn: Module chn.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_ODEC_REGION_DetachFrmChn(MMF_CHN_S *pstChn);

/* Create the thread to get frame from AI and send to AO */
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdAiAo(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn);
/* Create the thread to get frame from AI and send to AENC */
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdAiAenc(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn);
/* Create the thread to get frame from AENC and send to ADEC */
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdAencAdec(AENC_CHN AeChn, ADEC_CHN AdChn, FILE *pAecFd);
/* Get encoded audio data from AENC channel and write to file */
CVI_S32 SAMPLE_COMM_AUDIO_GetAenc(AENC_CHN AeChn, FILE *pAencFd);
/* Destroy the thread that gets encoded audio data from AENC channel */
CVI_S32 SAMPLE_COMM_AUDIO_DestoryGetAenc(AENC_CHN AeChn);
/* Create the thread to get frame from file and send to ADEC */
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdFileAdec(ADEC_CHN AdChn, FILE *pAdcFd);
/* Create the thread to get frame from ADEC and send to AO */
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdAdecAo(ADEC_CHN AdChn, AUDIO_DEV AoDev,  FILE *pFd);
/* Destroy the thread that handles audio decoding (ADEC) to audio output (AO) */
CVI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAdecAo(ADEC_CHN AdChn);
/* Create a thread to control the volume of audio output (AO) */
CVI_S32 SAMPLE_COMM_AUDIO_CreatTrdAoVolCtrl(AUDIO_DEV AoDev);
/* Destroy the thread that handles audio input (AI) */
CVI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAi(AUDIO_DEV AiDev, AI_CHN AiChn);
/* Destroy the thread that handles audio encoding (AENC) to audio decoding (ADEC) */
CVI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAencAdec(AENC_CHN AeChn);
/* Destroy the thread that handles audio decoding (ADEC) from file */
CVI_S32 SAMPLE_COMM_AUDIO_DestoryTrdFileAdec(ADEC_CHN AdChn);
/* Destroy the thread that controls the volume of audio output (AO) */
CVI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAoVolCtrl(AUDIO_DEV AoDev);
/* Destroy all audio-related threads */
CVI_S32 SAMPLE_COMM_AUDIO_DestoryAllTrd(void);
/* Bind an audio output (AO) channel to an audio decoding (ADEC) channel */
CVI_S32 SAMPLE_COMM_AUDIO_AoBindAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn);
/* Unbind an audio output (AO) channel from an audio decoding (ADEC) channel */
CVI_S32 SAMPLE_COMM_AUDIO_AoUnbindAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn);
/* Bind an audio input (AI) channel to an audio output (AO) channel */
CVI_S32 SAMPLE_COMM_AUDIO_AoBindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn);
/* Unbind an audio input (AI) channel from an audio output (AO) channel */
CVI_S32 SAMPLE_COMM_AUDIO_AoUnbindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn);
/* Bind an audio input (AI) channel to an audio encoding (AENC) channel */
CVI_S32 SAMPLE_COMM_AUDIO_AencBindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn);
/* Unbind an audio input (AI) channel from an audio encoding (AENC) channel */
CVI_S32 SAMPLE_COMM_AUDIO_AencUnbindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn);
/* Configure audio codec with specified attributes */
CVI_S32 SAMPLE_COMM_AUDIO_CfgAcodec(AIO_ATTR_S *pstAioAttr);
/* Start audio input (AI) with specified attributes, sample rate, and VQE settings */
CVI_S32 SAMPLE_COMM_AUDIO_StartAi(AUDIO_DEV AiDevId, CVI_S32 s32AiChn, AIO_ATTR_S *pstAioAttr,
                                  AUDIO_SAMPLE_RATE_E enOutSampleRate, CVI_BOOL bResampleEn, CVI_VOID *pstAiVqeAttr,
                                  CVI_U32 u32AiVqeType);
/* Stop audio input (AI) with specified resample and VQE settings */
CVI_S32 SAMPLE_COMM_AUDIO_StopAi(AUDIO_DEV AiDevId, CVI_S32 s32AiChn, CVI_BOOL bResampleEn, CVI_BOOL bVqeEn);
/* Start audio output (AO) with specified attributes and sample rate */
CVI_S32 SAMPLE_COMM_AUDIO_StartAo(AUDIO_DEV AoDevId, CVI_S32 s32AoChn, AIO_ATTR_S *pstAioAttr,
                                  AUDIO_SAMPLE_RATE_E enInSampleRate, CVI_BOOL bResampleEn);
/* Stop audio output (AO) with specified resample settings */
CVI_S32 SAMPLE_COMM_AUDIO_StopAo(AUDIO_DEV AoDevId, CVI_S32 s32AoChn, CVI_BOOL bResampleEn);
/* Start audio encoding (AENC) with specified attributes and payload type */
CVI_S32 SAMPLE_COMM_AUDIO_StartAenc(CVI_S32 s32AencChn, AIO_ATTR_S *pstAioAttr, PAYLOAD_TYPE_E enType);
/* Stop audio encoding (AENC) */
CVI_S32 SAMPLE_COMM_AUDIO_StopAenc(CVI_S32 s32AencChn);
/* Start audio decoding (ADEC) with specified payload type and attributes */
CVI_S32 SAMPLE_COMM_AUDIO_StartAdec(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType, ADEC_CHN_ATTR_S *pAdecAttr);
/* Stop audio decoding (ADEC) */
CVI_S32 SAMPLE_COMM_AUDIO_StopAdec(ADEC_CHN AdChn);


CVI_S32 SAMPLE_PLAT_SYS_INIT(SIZE_S stSize);
CVI_S32 SAMPLE_PLAT_VI_INIT(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_PLAT_VPSS_INIT(VPSS_GRP VpssGrp, SIZE_S stSizeIn, SIZE_S stSizeOut);
#if !defined(__CV180X__)
CVI_S32 SAMPLE_PLAT_VO_INIT(void);
CVI_S32 SAMPLE_PLAT_VO_INIT_BT656(void);
#endif
CVI_S32 SAMPLE_COMM_I2C_Write(CVI_S32 file, CVI_U16 addr, CVI_U16 reg, CVI_U16 val, CVI_U16 reg_w, CVI_U16 val_w);
CVI_S32 SAMPLE_COMM_I2C_Read(CVI_S32 file, CVI_U16 addr, CVI_U16 reg, CVI_U16 reg_w, CVI_U8 *r_val);
CVI_S32 SAMPLE_COMM_I2C_Open(CVI_CHAR *i2c_bus);
CVI_S32 SAMPLE_COMM_I2C_Close(CVI_S32 i2c_file);

CVI_S32 SAMPLE_COMM_VO_Init_BT656_MS7024(char *i2c_bus_str, uint8_t slave_addr, uint8_t selection);
CVI_S32 SAMPLE_COMM_VO_Init_MIPI_HX8394(void *pvData);
CVI_S32 SAMPLE_COMM_BIN_ReadParaFrombin(void);
CVI_S32 SAMPLE_COMM_BIN_ReadBlockParaFrombin(enum CVI_BIN_SECTION_ID id);
/* SAMPLE_COMM_FRAME_SaveToFile:
 *   Save videoframe to the file
 *
 * [in]filename: char string of the file to save data.
 * [IN]pstVideoFrame: the videoframe whose data will be saved to file.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_FRAME_SaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

/* SAMPLE_COMM_PrepareFrame:
 *   Prepare videoframe per size & format.
 *
 * [in]stSize: the size of videoframe
 * [in]enPixelFormat: pixel format of videoframes
 * [Out]pstVideoFrame: the videoframe generated.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_PrepareFrame(SIZE_S stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame);

/* SAMPLE_COMM_FRAME_CompareWithFile:
 *   Compare data with frame.
 *
 * [in]filename: file to read.
 * [in]pstVideoFrame: the video-frame to store data from file.
 * return: True if match; False if mismatch.
 */
CVI_BOOL SAMPLE_COMM_FRAME_CompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

/* SAMPLE_COMM_FRAME_LoadFromFile:
 *   Load data to frame, whose data loaded from given filename.
 *
 * [in]filename: file to read.
 * [in]pstVideoFrame: the video-frame to store data from file.
 * [in]stSize: size of image.
 * [in]enPixelFormat: format of image
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_FRAME_LoadFromFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame,
	SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat);

/* SAMPLE_COMM_CheckDDR64MBSize:
 *   Check if current board DDR size is 64 MB.
 *
 * [out]b64Ddr: true if current board DDR size is 64 MB.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_CheckDDR64MBSize(CVI_BOOL *b64Ddr);

int SAMPLE_COMM_GPIO_SetValue(unsigned int gpio, unsigned int value);
int SAMPLE_COMM_GPIO_GetValue(unsigned int gpio, unsigned int *value);

/* SAMPLE_COMM_DATAFIFO_Access:
 *   for reader to get datafifo.
 * [in]params:datafifo parameters
 * [in]phyAddr:physical address of datafifo writer initialized.
 * [out]hDataFifo:datafifo handle.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_DATAFIFO_Access(CVI_DATAFIFO_PARAMS_S* params, CVI_U64 phyAddr,
			CVI_DATAFIFO_HANDLE *hDataFifo);

/* SAMPLE_COMM_DATAFIFO_Init:
 *   for writer to init datafifo.
 * [in]params:datafifo parameters
 * [in]pArg:data release callback function writer registered.
 * [out]phyAddr:physical address of datafifo writer initialized.
 * [out]hDataFifo:datafifo handle.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_DATAFIFO_Init(CVI_DATAFIFO_PARAMS_S* params, CVI_U64 *phyAddr,
			CVI_DATAFIFO_HANDLE *hDataFifo, CVI_VOID *pArg);

/* SAMPLE_COMM_DATAFIFO_Read:
 *   for reader to read datafifo.
 * [in]hDataFifo:datafifo handle.
 * [in]pBuf:buffer datafifo read to.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_DATAFIFO_Read(CVI_DATAFIFO_HANDLE hDataFifo, CVI_CHAR *pBuf);

/* SAMPLE_COMM_DATAFIFO_Write:
 *   for reader to read datafifo.
 * [in]hDataFifo:datafifo handle.
 * [in]pBuf:buffer datafifo read to.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_DATAFIFO_Write(CVI_DATAFIFO_HANDLE hDataFifo, CVI_CHAR *pBuf);

/* SAMPLE_COMM_DATAFIFO_Exit:
 *   for reader to exit datafifo.
 * [in]hDataFifo:datafifo handle.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_DATAFIFO_Exit(CVI_DATAFIFO_HANDLE hDataFifo);

/* SAMPLE_COMM_IPCMSG_Init:
 *   Dual os message communication system initialization.
 * [in]pszServiceName:Service name.
 * [in]pstConnectAttr:Connect attribute.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_IPCMSG_Init(CVI_CHAR *pszServiceName, CVI_IPCMSG_CONNECT_S *pstConnectAttr);

/* SAMPLE_COMM_IPCMSG_Deinit:
 *   Dual os message communication system deinitialization.
 * [in]pszServiceName:Service name.
 * [in]s32Id:Handle of IPCMSG.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_IPCMSG_Deinit(CVI_CHAR *pszServiceName, CVI_S32 s32Id);

/* SAMPLE_COMM_IPCMSG_SendSync:
 *   Send message synchronously.
 * [in]s32Id:Handle of IPCMSG.
 * [in]u32Module:Module ID.
 * [in]u32CMD:CMD ID.
 * [in]pBody:Message body.
 * [in]u32BodyLen:Length of pBody.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_IPCMSG_SendSync(CVI_S32 s32Id, CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen);

/* SAMPLE_COMM_IPCMSG_SendAsync:
 *   Send message asynchronously.
 * [in]s32Id:Handle of IPCMSG.
 * [in]u32Module:Module ID.
 * [in]u32CMD:CMD ID.
 * [in]pBody:Message body.
 * [in]u32BodyLen:Length of pBody.
 * [in]respHandler:Callback function to receive response.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_IPCMSG_SendAsync(CVI_S32 s32Id, CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen, void (*respHandler)(CVI_IPCMSG_MESSAGE_S*));

/* SAMPLE_COMM_IPCMSG_SendAsync:
 *   Send message only.
 * [in]s32Id:Handle of IPCMSG.
 * [in]u32Module:Module ID.
 * [in]u32CMD:CMD ID.
 * [in]pBody:Message body.
 * [in]u32BodyLen:Length of pBody.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_IPCMSG_SendOnly(CVI_S32 s32Id, CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
