#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/select.h>
#include <assert.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <inttypes.h>
#include "cvi_venc.h"
#include "sample_comm.h"
#include "sample_venc_testcase.h"
#include "sample_venc_lib.h"
#include <limits.h>

#include <sys/ioctl.h>
#include "cvi_msg_client.h"

#define MAX_VENC_OPTIONS	256
#define MAX_STRING_LEN		255
#define MAX_FILENAME_LEN	64

#define Q_TABLE_MAX 99
#define Q_TABLE_MIN 1

#if defined(__GNUC__) && defined(__riscv)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation=" /* Or  "-Wformat-overflow"  */
#endif

#define max(a, b)\
		({__typeof__(a) _a = (a);\
		__typeof__(b) _b = (b);\
		(_a) > (_b) ? (_a) : (_b); })
#define min(a, b)\
		({__typeof__(a) _a = (a);\
		__typeof__(b) _b = (b);\
		(_a) < (_b) ? (_a) : (_b); })
#define clip(a, b, v) (min((max(a, v)), b))

typedef enum _ARG_TYPE_ {
	ARG_INT = 0,
	ARG_UINT,
	ARG_STRING,
} ARG_TYPE;

typedef struct _optionExt_ {
	struct option opt;
	int type;
	int64_t min;
	int64_t max;
	const char *help;
} optionExt;

typedef union {
	CVI_S32 ival;
	CVI_U32 uval;
} SAMPLE_ARG;

static optionExt venc_long_option_ext[] = {
	{{"codec",     optional_argument, NULL, 'c'}, ARG_STRING, 0,   0,
		"265 = h.265, jpg = jpeg, mjp = motion jpeg" },
	{{"width",     optional_argument, NULL, 'w'}, ARG_UINT,    352, 3840,
		"width"},
	{{"height",    optional_argument, NULL, 'h'}, ARG_UINT,    352, 3840,
		"height"},
	{{"input",     optional_argument, NULL, 'i'}, ARG_STRING, 0,   0,
		"source yuv file"},
	{{"output",    optional_argument, NULL, 'o'}, ARG_STRING, 0,   0,
		"output bitstream"},
	{{"frame_num", optional_argument, NULL, 'n'}, ARG_UINT,    0,   1000000000,
		"number of frame to be encode"},
	{{"getBsMode", optional_argument, NULL, 0},   ARG_UINT,    0,   1,
		"get-bitstream mode, 0 = query status, 1 = select"},
	{{"profile",   optional_argument, NULL, 0},   ARG_INT,
		CVI_H264_PROFILE_MIN,   CVI_H264_PROFILE_MAX,
		"profile, 0 = h264 baseline, 1 = h264 main, 2 = h264 high, Default = 2"},
	{{"rcMode",    optional_argument, NULL, 0},   ARG_INT,    0,   6,
		"rate control mode, 0 = CBR, 1 = VBR, 2 = AVBR, 4 = FIXQP, 5 = QPMAP, 6 = UBR (User BR), default = 4"},
	{{"iqp",       optional_argument, NULL, 0},   ARG_INT,    0,   51,
		"I frame QP"},
	{{"pqp",       optional_argument, NULL, 0},   ARG_INT,    0,   51,
		"P frame QP"},
	{{"ipQpDelta",       optional_argument, NULL, 0},   ARG_INT,
		CVI_H26X_NORMALP_IP_QP_DELTA_MIN,   CVI_H26X_NORMALP_IP_QP_DELTA_MAX,
		"QP Delta between P frame and I frame"},
	{{"bgQpDelta", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_SMARTP_BG_QP_DELTA_MIN, CVI_H26X_SMARTP_BG_QP_DELTA_MAX,
		"Smart-P QP delta between P frame and BG (background) frame. [-10, 30], default = 0"},
	{{"viQpDelta", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_SMARTP_VI_QP_DELTA_MIN, CVI_H26X_SMARTP_VI_QP_DELTA_MAX,
		"Smart-P QP delta between P frame and VI (virtual I) frame. [-10, 30], default = 0"},
	{{"gop",       optional_argument, NULL, 0},   ARG_INT,
		CVI_H26X_GOP_MIN, CVI_H26X_GOP_MAX,
		"The period of one gop"},
	{{"gopMode",   optional_argument, NULL, 0},   ARG_UINT,
		CVI_H26X_GOP_MODE_MIN, CVI_H26X_GOP_MODE_MAX,
		"GOP mode. 0: Normal P, 2: Smart P, Default: 0"},
	{{"bitrate",   optional_argument, NULL, 0},   ARG_INT,    1,   1000000,
		"The average target bitrate (kbits)"},
	{{"initQp",    optional_argument, NULL, 0},   ARG_INT,    0,   100,
		"The Start Qp of 1st frame, 63 = default"},
	{{"minQp",     optional_argument, NULL, 0},   ARG_INT,    0,   51,
		"Minimum Qp for one frame"},
	{{"maxQp",     optional_argument, NULL, 0},   ARG_INT,    0,   51,
		"Maximum Qp for one frame"},
	{{"minIqp",    optional_argument, NULL, 0},   ARG_INT,    0,   51,
		"Minimum Qp for I frame"},
	{{"maxIqp",    optional_argument, NULL, 0},   ARG_INT,    0,   51,
		"Maximum Qp for I frame"},
	{{"srcFramerate", optional_argument, NULL, 0},   ARG_INT,	  0,   240,
		"source frame rate"},
	{{"framerate", optional_argument, NULL, 0},   ARG_INT,	  0,   INT_MAX,
		"destination frame rate"},
	{{"vfps", required_argument, NULL, 0},     ARG_INT,	  0,   1,
		"enable variable FPS"},
	{{"quality", required_argument, NULL, 0},     ARG_INT,	  Q_TABLE_MIN,   Q_TABLE_MAX,
		"jpeg encode quality"},
	{{"maxbitrate", optional_argument, NULL, 0},  ARG_INT,	  0,   1000000,
		"Maximum output bit rate (kbits)"},
	{{"changePos", optional_argument, NULL, 0},  ARG_INT,	  50,   100,
		"Ratio to change Qp"},
	{{"minStillPercent", optional_argument, NULL, 0},  ARG_INT,	  5,   100,
		"Percentage of target bitrate in low motion"},
	{{"maxStillQp", optional_argument, NULL, 0},  ARG_UINT,	  0,   51,
		"Maximum Qp in low motion"},
	{{"motionSense", optional_argument, NULL, 0},  ARG_UINT,	  0,   100,
		"Motion sensitivity"},
	{{"avbrFrmLostOpen", optional_argument, NULL, 0},  ARG_INT,	  0,   100,
		"avbrFrmLostOpen"},
	{{"avbrFrmGap", optional_argument, NULL, 0},  ARG_INT,	  0,   100,
		"avbrFrmGap"},
	{{"avbrPureStillThr", optional_argument, NULL, 0},  ARG_INT,	  0,   100,
		"avbrPureStillThr"},
	{{"bgEnhanceEn", optional_argument, NULL, 0},  ARG_INT,
		CVI_H26X_BG_ENHANCE_EN_MIN,   CVI_H26X_BG_ENHANCE_EN_MAX,
		"Enable background enhancement"},
	{{"bgDeltaQp", optional_argument, NULL, 0},  ARG_INT,
		CVI_H26X_BG_DELTA_QP_MIN,   CVI_H26X_BG_DELTA_QP_MAX,
		"background delta qp"},
	{{"statTime", optional_argument, NULL, 0},    ARG_INT,	  0,   240,
		"statistics time in seconds"},
	{{"testMode", optional_argument, NULL, 0},    ARG_UINT,	  0,   (SAMPLE_VENC_TEST_MODE_MAX - 1),
		"samele_venc test mode"},
	{{"getstream-timeout", optional_argument, NULL, 0},    ARG_INT,	  -1,   100000,
		"samele_venc getstream-timeout   -1:block mode, 0:try_once, >0 timeout in ms"},
	{{"sendframe-timeout", optional_argument, NULL, 0},    ARG_INT,	  -1,   100000,
		"samele_venc sendframe-timeout   -1:block mode, 0:try_once, >0 timeout in ms"},
	{{"ifInitVb", optional_argument, NULL, 0},    ARG_INT,	  0,   1,
		"if enable VB pool or not"},
	{{"vbMode", optional_argument, NULL, 0},    ARG_INT,	  0,   3,
		"if enable VB pool mode. 0 = common, 1 = module, 2 = private, 3 = user"},
	{{"yuvFolder", optional_argument, NULL, 0},   ARG_STRING, 0,   256,
		"yuv files folder"},
	{{"bindmode", optional_argument, NULL, 0},    ARG_UINT,	  0,   2,
		"bind mode"},
	{{"pixel_format", optional_argument, NULL, 0}, ARG_INT,	  0,   3,
		"0: 420 planar, 1: 422 planar, 2: NV12, 3: NV21"},
	{{"posX", optional_argument, NULL, 0},        ARG_INT,    0, 3840,
		"x axis of start position, need to be multiple of 16 (used for crop)"},
	{{"posY", optional_argument, NULL, 0},        ARG_INT,    0, 3840,
		"y axis of start position, need to be multiple of 16 (used for crop)"},
	{{"inWidth", optional_argument, NULL, 0},     ARG_INT,    352, 3840,
		"width of input frame (used for crop)"},
	{{"inHeight", optional_argument, NULL, 0},    ARG_INT,    288, 3840,
		"height of input frame (used for crop)"},
	{{"bufSize", optional_argument, NULL, 0}, ARG_UINT,    0,   1000000000,
		"bitstream Buffer size"},
	{{"single_LumaBuf", optional_argument, NULL, 0}, ARG_INT,	  0,   1,
		"0: disable, 1: use single luma buffer for H264"},
	{{"single_core", optional_argument, NULL, 0}, ARG_INT,	  0,   1,
		"0: disable, 1: use single core(h264 or h265 only)"},
	{{"forceIdr", optional_argument, NULL, 0}, ARG_INT,	  0,   1000000000,
		"0: disable, > 0: set force idr at number of frame"},
	{{"resetGop", optional_argument, NULL, 0}, ARG_UINT,	  0,   1,
		"0: not reset, 1: reset gop, reset when forceIdr > 0"},
	{{"chgNum", optional_argument, NULL, 0},	  ARG_INT,	  0, 1000000,
		"frame num to change attr"},
	{{"chgBitrate", optional_argument, NULL, 0},  ARG_INT,	  1, 1000000,
		"change bitrate  (kbits)"},
	{{"chgFramerate", optional_argument, NULL, 0},	ARG_INT,	  0, 240,
		"change dstframerate"},
	{{"tempLayer", optional_argument, NULL, 0}, ARG_INT,	  0,   3,
		"tempLayer"},
	{{"roiCfgFile", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"ROI configuration file"},
	{{"qpMapCfgFile", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"Roi-based qpMap file"},
	{{"bgInterval", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_SMARTP_BG_INTERVAL_MIN, CVI_H26X_SMARTP_BG_INTERVAL_MAX,
		"bgInterval"},
	{{"frame_lost", optional_argument, NULL, 0}, ARG_INT,	  0,   1,
		"0: disable, 1: use frame lost(h264 or h265 only)"},
	{{"frame_lost_gap", optional_argument, NULL, 0}, ARG_UINT,	  0,   65536,
		"The gap between 2 frame_lost frames(h264 or h265 only)"},
	{{"frame_lost_thr", optional_argument, NULL, 0}, ARG_UINT,	  0,   1200000000,
		"frame_lost bsp threshold(h264 or h265 only)"},
	{{"MCUPerECS", required_argument, NULL, 0},   ARG_INT,	  0,   1000000,
		"jpeg encode MCUPerECS"},
	{{"single_EsBuf", optional_argument, NULL, 0}, ARG_INT,	  0,   1,
		"0: disable, 1: use single stream buffer (jpege)"},
	{{"single_EsBuf_264", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"0: disable, 1: use single stream buffer (h264e)"},
	{{"single_EsBuf_265", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"0: disable, 1: use single stream buffer (h265e)"},
	{{"single_EsBufSize", optional_argument, NULL, 0}, ARG_INT, 0, 1000000000,
		"single stream buffer size (jpege)"},
	{{"single_EsBufSize_264", optional_argument, NULL, 0}, ARG_INT, 0, 1000000000,
		"single stream buffer size (h264e)"},
	{{"single_EsBufSize_265", optional_argument, NULL, 0}, ARG_INT, 0, 1000000000,
		"single stream buffer size (h265e)"},
	{{"numChn",    optional_argument, NULL, 0},   ARG_INT,    1,   VENC_MAX_CHN_NUM,
		"number of channels to encode"},
	{{"chn",       optional_argument, NULL, 0},   ARG_UINT,    0,   VENC_MAX_CHN_NUM - 1,
		"set channel-id to configure the following parameters"},
	{{"viWidth",       optional_argument, NULL, 0},   ARG_UINT,    0,   4096,
		"for VI input width"},
	{{"viHeight",       optional_argument, NULL, 0},   ARG_UINT,    0,   2304,
		"for VI input height"},
	{{"vpssWidth",       optional_argument, NULL, 0},   ARG_UINT,    0,   4096,
		"for Vpss output width"},
	{{"vpssHeight",       optional_argument, NULL, 0},   ARG_UINT,    0,   2304,
		"for VPss output height"},
	{{"vpssSrcPath", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"source file path for vpss"},
	{{"user_data1", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"user data binary file 1"},
	{{"user_data2", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"user data binary file 2"},
	{{"user_data3", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"user data binary file 3"},
	{{"user_data4", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"user data binary file 4"},
	{{"h265RefreshType", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"0: IDR, 1: CRA, default = 0"},
	{{"initialDelay", optional_argument, NULL, 0}, ARG_INT,
		CVI_INITIAL_DELAY_MIN, CVI_INITIAL_DELAY_MAX,
		"rc initial delay in ms, default = 1000"},
	{{"jpegMarkerOrder", optional_argument, NULL, 0}, ARG_INT, 0, 2,
		"0: Cvitek, 1: SOI-JFIF-DQT_MERGE-SOF0-DHT_MERGE-DRI, 2: Cvitek w/ JFIF, default = 0"},
	{{"intraCost", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_INTRACOST_MIN, CVI_H26X_INTRACOST_MAX,
		"intraCost, the extra cost of intra mode"},
	{{"thrdLv", optional_argument, NULL, 0},
		ARG_UINT, CVI_H26X_THRDLV_MIN, CVI_H26X_THRDLV_MAX,
		"thrdLv, threhold to control block qp"},
	{{"h264EntropyMode", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H264_ENTROPY_MIN, CVI_H264_ENTROPY_MAX,
		"0: CAVLC, 1: CABAC, default = 1"},
	{{"h264ChromaQpOffset", optional_argument, NULL, 0}, ARG_INT, -12, 12,
		"H264 Chroma QP offset [-12, 12], default = 0"},
	{{"h265CbQpOffset", optional_argument, NULL, 0}, ARG_INT, -12, 12,
		"H265 Cb QP offset [-12, 12], default = 0"},
	{{"h265CrQpOffset", optional_argument, NULL, 0}, ARG_INT, -12, 12,
		"H265 Cr QP offset [-12, 12], default = 0"},
	{{"maxIprop", optional_argument, NULL, 0}, ARG_INT, 1, 100,
		"max I frame bitrate ratio to P frame, default = 100"},
	{{"rowQpDelta", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_ROW_QP_DELTA_MIN, CVI_H26X_ROW_QP_DELTA_MAX,
		"rowQpDelta [0, 10], default = 1"},
	{{"superFrmMode", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_SUPER_FRM_MODE_MIN, CVI_H26X_SUPER_FRM_MODE_MAX,
		"superFrmMode, 0 = disable, 3 = encode to IDR, default = 0"},
	{{"superIBitsThr", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_SUPER_I_BITS_THR_MIN, CVI_H26X_SUPER_I_BITS_THR_MAX,
		"superIBitsThr [1000, 33554432], default = 4000000"},
	{{"superPBitsThr", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_SUPER_P_BITS_THR_MIN, CVI_H26X_SUPER_P_BITS_THR_MAX,
		"superPBitsThr [1000, 33554432], default = 4000000"},
	{{"maxReEnc", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_MAX_RE_ENCODE_MIN, CVI_H26X_MAX_RE_ENCODE_MAX,
		"maxReEnc [0, 3], default = 0"},
	{{"aspectRatioInfoPresentFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_ASPECT_RATIO_INFO_PRESENT_FLAG_MIN, CVI_H26X_ASPECT_RATIO_INFO_PRESENT_FLAG_MAX,
		"aspect ratio info present flag [0, 1], default = 0"},
	{{"aspectRatioIdc", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_ASPECT_RATIO_IDC_MIN, CVI_H26X_ASPECT_RATIO_IDC_MAX,
		"aspect ratio idc [0, 255], default = 1"},
	{{"overscanInfoPresentFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_OVERSCAN_INFO_PRESENT_FLAG_MIN, CVI_H26X_OVERSCAN_INFO_PRESENT_FLAG_MAX,
		"overscan info present flag [0, 1], default = 0"},
	{{"overscanAppropriateFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_OVERSCAN_APPROPRIATE_FLAG_MIN, CVI_H26X_OVERSCAN_APPROPRIATE_FLAG_MAX,
		"overscan appropriate flag [0, 1], default = 0"},
	{{"sarWidth", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_SAR_WIDTH_MIN, CVI_H26X_SAR_WIDTH_MAX,
		"sar width [0, 65535], default = 1"},
	{{"sarHeight", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_SAR_HEIGHT_MIN, CVI_H26X_SAR_HEIGHT_MAX,
		"sar height [0, 65535], default = 1"},
	{{"timingInfoPresentFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_TIMING_INFO_PRESENT_FLAG_MIN, CVI_H26X_TIMING_INFO_PRESENT_FLAG_MAX,
		"timing info present flag [0, 1], default = 0"},
	{{"fixedFrameRateFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H264_FIXED_FRAME_RATE_FLAG_MIN, CVI_H264_FIXED_FRAME_RATE_FLAG_MAX,
		"fixed frame rate flag [0, 1], default = 0"},
	{{"numUnitsInTick", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_NUM_UNITS_IN_TICK_MIN, CVI_H26X_NUM_UNITS_IN_TICK_MAX,
		"num units in tick [0, 4294967295], default = 1"},
	{{"timeScale", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_TIME_SCALE_MIN, CVI_H26X_TIME_SCALE_MAX,
		"time scale [0, 4294967295], default = 60"},
	{{"videoSignalTypePresentFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_VIDEO_SIGNAL_TYPE_PRESENT_FLAG_MIN, CVI_H26X_VIDEO_SIGNAL_TYPE_PRESENT_FLAG_MAX,
		"video signal type present flag [0, 1], default = 0"},
	{{"videoFormat", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_VIDEO_FORMAT_MIN, CVI_H264_VIDEO_FORMAT_MAX,
		"video format [0, 7], default = 5"},
	{{"videoFullRangeFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_VIDEO_FULL_RANGE_FLAG_MIN, CVI_H26X_VIDEO_FULL_RANGE_FLAG_MAX,
		"video full range flag [0, 1], default = 0"},
	{{"colourDescriptionPresentFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_COLOUR_DESCRIPTION_PRESENT_FLAG_MIN, CVI_H26X_COLOUR_DESCRIPTION_PRESENT_FLAG_MAX,
		"colour description present flag [0, 1], default = 0"},
	{{"colourPrimaries", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_COLOUR_PRIMARIES_MIN, CVI_H26X_COLOUR_PRIMARIES_MAX,
		"colour primaries [0, 255], default = 2"},
	{{"transferCharacteristics", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_TRANSFER_CHARACTERISTICS_MIN, CVI_H26X_TRANSFER_CHARACTERISTICS_MAX,
		"transfer characteristics [0, 255], default = 2"},
	{{"matrixCoefficients", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_MATRIX_COEFFICIENTS_MIN, CVI_H26X_MATRIX_COEFFICIENTS_MAX,
		"matrix coefficients [0, 255], default = 2"},
	{{"testUbrEn", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_TEST_UBR_EN_MIN, CVI_H26X_TEST_UBR_EN_MAX,
		"enable to test ubr [0, 1], default = 0"},
	{{"frameQp", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_FRAME_QP_MIN, CVI_H26X_FRAME_QP_MAX,
		"frameQp [0, 51], default = 38"},
	{{"esBufQueueEn", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_ES_BUFFER_QUEUE_MIN, CVI_H26X_ES_BUFFER_QUEUE_MAX,
		"esBufQueueEn [0, 1], default = 1"},
	{{"isoSendFrmEn", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_ISO_SEND_FRAME_MIN, CVI_H26X_ISO_SEND_FRAME_MAX,
		"isoSendFrmEn [0, 1], default = 1"},
	{{"sensorEn", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_SENSOR_EN_MIN, CVI_H26X_SENSOR_EN_MAX,
		"sensorEn [0, 1], default = 0"},
	{{"sliceSplitCnt", optional_argument, NULL, 0}, ARG_INT, 0, 5,
		"sliceSplitCnt [1, 5], default = 1"},
	{{"disabledblk", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"disabledblk [0, 1], default = 0"},
	{{"betaOffset", optional_argument, NULL, 0}, ARG_INT, -6, 6,
		"betaOffset [-6, 6], default = 0"},
	{{"alphaoffset", optional_argument, NULL, 0}, ARG_INT, -6, 6,
		"alphaoffset [-6, 6], default = 0"},
	{{"intraPred", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"intraPred [0, 1], default = 0"},
	{{"maxQuality", optional_argument, NULL, 0}, ARG_INT, Q_TABLE_MIN, Q_TABLE_MAX,
		"maxQuality of mjpeg rc[1, 99], default = 1"},
	{{"minQuality", optional_argument, NULL, 0}, ARG_INT, Q_TABLE_MIN, Q_TABLE_MAX,
		"minQuality of mjpeg rc[1, 99], default = 99"},
	{{"svcEnable", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"svcEnable [0, 1], default = 0"},
	{{"fgProtectEn", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"fgProtectEn [0, 1], default = 0"},
	{{"fgDealtQp", optional_argument, NULL, 0}, ARG_INT, 0, 8,
		"fgDealtQp [0, 8], default = 0"},
	{{"cplxSceneDetectEn", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"cplxSceneDetectEn [0, 1], default = 0"},
	{{"cplxSceneLowTh", optional_argument, NULL, 0}, ARG_INT, 0, 512,
		"cplxSceneLowTh [0, 512], default = 256"},

	{{"cplxSceneHightTh", optional_argument, NULL, 0}, ARG_INT, 0, 512,
		"cplxSceneHightTh [0, 512], default = 318"},

	{{"middleMinPercent", optional_argument, NULL, 0}, ARG_INT, 0, 100,
		"middleMinPercent [0, 100], default = 50"},

	{{"cplxMinPercent", optional_argument, NULL, 0}, ARG_INT, 0, 100,
		"cplxMinPercent [0, 100], default = 80"},

	{{"smartAiEn", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"smartAiEn [0, 1], default = 0"},
	{{NULL, 0, NULL, 0}, ARG_INT, 0, 0, ""}
};

#ifdef OUTPUT_SCALED_PIC
static CVI_S32 _SAMPLE_VENC_writeFrame(VIDEO_FRAME_INFO_S *pstVideoFrame, FILE *fp);
#endif
static VB_POOL vpssVB[VENC_MAX_CHN_NUM] = {[0 ...(VENC_MAX_CHN_NUM - 1)] = VB_INVALID_POOLID};
static CVI_S32 checkArg(CVI_S32 entryIdx, SAMPLE_ARG *pArg);
static CVI_U32 _SAMPLE_VENC_INIT_CHANNEL(sampleVenc *psv, CVI_U32 chnNum);
static CVI_S32 initSysAndVb(sampleVenc *psv);
static CVI_S32 _SAMPLE_VENC_LoadCfgFile(vencChnCtx *pvecc);
static CVI_S32 SAMPLE_VENC_StartGetStream(vencChnCtx *pvecc, CVI_S32 s32ChnIdx);
static CVI_S32 _getNonBindModeSrcFrame(vencChnCtx *pvecc,
		VIDEO_FRAME_INFO_S *pstVideoFrame);
static CVI_VOID _venc_unbind_source(chnInputCfg *pIc, VENC_CHN VencChn);
static CVI_S32 _releaseNonBindModeSrcFrame(vencChnCtx *pvecc);
static CVI_VOID *SAMPLE_VENC_GetVencStreamProc(CVI_VOID *pArgs);
static CVI_S32 _SAMPLE_VENC_SendFrame(vencChnCtx *pvecc, CVI_U32 i);
static CVI_S32 _SAMPLE_VENC_GetStream(vencChnCtx *pvecc);
static CVI_S32 cviReadSrcFrame(VIDEO_FRAME_S *pstVFrame, FILE *fp);
static CVI_VOID exitSysAndVb(CVI_VOID);
static VIDEO_FRAME_INFO_S *allocate_frame(SIZE_S stSize, PIXEL_FORMAT_E pixel_format);
static CVI_S32 free_frame(VIDEO_FRAME_INFO_S *pstVideoFrame);
static CVI_S32 _SAMPLE_VENC_SendOneFrame(vencChnCtx *pvecc);
static CVI_S32 SAMPLE_COMM_VENC_SetUserFrameLevelRc(chnInputCfg *pIc, VENC_CHN VencChn);
static CVI_VOID SAMPLE_VENC_InsertUserData(VENC_CHN chn, chnInputCfg *pIc);

static PIC_SIZE_E getEnSize(CVI_U32 u32Width, CVI_U32 u32Height)
{
	if (u32Width == 352 && u32Height == 288)
		return PIC_CIF;
	else if (u32Width == 720 && u32Height == 576)
		return PIC_D1_PAL;
	else if (u32Width == 720 && u32Height == 480)
		return PIC_D1_NTSC;
	else if (u32Width == 1280 && u32Height == 720)
		return PIC_720P;
	else if (u32Width == 1920 && u32Height == 1080)
		return PIC_1080P;
	else if (u32Width == 2592 && u32Height == 1520)
		return PIC_2592x1520;
	else if (u32Width == 2592 && u32Height == 1536)
		return PIC_2592x1536;
	else if (u32Width == 2592 && u32Height == 1944)
		return PIC_2592x1944;
	else if (u32Width == 2716 && u32Height == 1524)
		return PIC_2716x1524;
	else if (u32Width == 3840 && u32Height == 2160)
		return PIC_3840x2160;
	else if (u32Width == 4096 && u32Height == 2160)
		return PIC_4096x2160;
	else if (u32Width == 3000 && u32Height == 3000)
		return PIC_3000x3000;
	else if (u32Width == 4000 && u32Height == 3000)
		return PIC_4000x3000;
	else if (u32Width == 3840 && u32Height == 8640)
		return PIC_3840x8640;
	else if (u32Width == 640 && u32Height == 480)
		return PIC_640x480;
	else
		return PIC_CUSTOMIZE;
}

// Map command line input pixel format to PIXEL_FORMAT_E.
static PIXEL_FORMAT_E vencMapPixelFormat(CVI_S32 pixel_format)
{
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;

	switch (pixel_format) {
	case 0:
		enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		break;
	case 1:
		enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_422;
		break;
	case 2:
		enPixelFormat = PIXEL_FORMAT_NV12;
		break;
	case 3:
		enPixelFormat = PIXEL_FORMAT_NV21;
		break;
	default:
		CVI_VENC_WARN("Unknown input pixel format. Assume YUV420P.\n");
		enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		break;
	}

	return enPixelFormat;
}

static CVI_U32 getSrcFrameSizeByPixelFormat(CVI_U32 width, CVI_U32 height, PIXEL_FORMAT_E enPixelFormat)
{
	CVI_U32 size = 0;

	switch (enPixelFormat) {
	case PIXEL_FORMAT_YUV_PLANAR_422:
		size = width * height * 2;
		break;
	case PIXEL_FORMAT_YUV_PLANAR_420:
	case PIXEL_FORMAT_NV12:
	case PIXEL_FORMAT_NV21:
		size = width * height * 3 / 2;
		break;
	default:
		CVI_VENC_WARN("Unknown pixel format. Assume YUV420P.\n");
		size = width * height * 3 / 2;
		break;
	}

	return size;
}

void print_help(char * const *argv)
{
	CVI_U32 idx;

	printf("// ------------------------------------------------\n");
	printf("// %s -c codec -w width -h height -i src.yuv -o enc\n", argv[0]);
	printf("EX.\n");
	printf("sample_venc -c 265 -w 1920 -h 1080 -i ReadySteadyGo_1920x1080_600.yuv -o enc\n");
	printf("// ------------------------------------------------\n");

	for (idx = 0; idx < sizeof(venc_long_option_ext) / sizeof(optionExt); idx++) {
		if (venc_long_option_ext[idx].opt.name == NULL)
			break;

		printf("--%s\n", venc_long_option_ext[idx].opt.name);
		printf("    %s\n", venc_long_option_ext[idx].help);
	}
}

CVI_S32 venc_main(int argc, char **argv)
{
	sampleVenc sv, *psv = &sv;
	commonInputCfg *pcic = &psv->commonIc;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_MSG_Init();

	s32Ret = SAMPLE_VENC_INIT_CFG(psv, argc, argv);
	if (s32Ret < 0) {
		CVI_VENC_ERR("SAMPLE_VENC_INIT_CFG\n");
		return s32Ret;
	}

	if (pcic->testMode == JPEG_CONTI_ENCODE_MODE) {
		int num_testcase = getNumTestcase(JPEG_CONTI_ENCODE_MODE);

		for (int idx = 0; idx < num_testcase; idx++) {
			chnInputCfg *pTestIc = getInputCfgTestcase(JPEG_CONTI_ENCODE_MODE);
			CVI_S32 bCreateChn = psv->chnCtx[0].chnIc.bCreateChn;
			char yuvFilename[MAX_FILENAME_LEN];

			memcpy(&psv->chnCtx[0].chnIc, &pTestIc[idx], sizeof(chnInputCfg));
			psv->chnCtx[0].chnIc.getstream_timeout = -1;
			psv->chnCtx[0].chnIc.sendframe_timeout = 20000;
			snprintf(yuvFilename, MAX_STRING_LEN, "%s", psv->chnCtx[0].chnIc.input_path);
			snprintf(psv->chnCtx[0].chnIc.input_path, MAX_STRING_LEN, "%s%s",
				 pcic->yuvFolder, yuvFilename);
			psv->chnCtx[0].chnIc.bCreateChn = bCreateChn;

			s32Ret = checkInputCfg(&psv->chnCtx[0].chnIc);
			if (s32Ret) {
				CVI_VENC_ERR("failure\n");
				return -1;
			}

			s32Ret = SAMPLE_VENC_START(psv);
			if (s32Ret < 0) {
				CVI_VENC_ERR("SAMPLE_VENC_START\n");
				return s32Ret;
			}

			s32Ret = SAMPLE_VENC_STOP(psv);
			if (s32Ret < 0) {
				CVI_VENC_ERR("SAMPLE_VENC_STOP\n");
				return s32Ret;
			}
			CVI_VENC_INFO("test case idx %d done, total %d\n", (idx + 1), num_testcase);
		}

		s32Ret = API_COST_TIME_LOG(CVI_VENC_DestroyChn(0), "CVI_VENC_DestroyChn");
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_VENC_DestroyChn vechn[%d] failed with %#x!\n",
					0, s32Ret);
			return CVI_FAILURE;
		}
	} else {
		s32Ret = SAMPLE_VENC_START(psv);
		if (s32Ret < 0) {
			CVI_VENC_ERR("SAMPLE_VENC_START\n");
			return s32Ret;
		}

		s32Ret = SAMPLE_VENC_STOP(psv);
		if (s32Ret < 0) {
			CVI_VENC_ERR("SAMPLE_VENC_STOP\n");
			return s32Ret;
		}
	}

	if (pcic->ifInitVb)
		exitSysAndVb();

	return s32Ret;
}

CVI_S32 SAMPLE_VENC_INIT_CFG(sampleVenc *psv, int argc, char **argv)
{
	chnInputCfg *pIc;
	commonInputCfg *pcic = &psv->commonIc;

	cviGetMask();

	memset(psv, 0, sizeof(*psv));

	for (CVI_S32 s32ChnIdx = 0; s32ChnIdx < VENC_MAX_CHN_NUM; s32ChnIdx++) {
		pIc = &psv->chnCtx[s32ChnIdx].chnIc;
		initInputCfg(pcic, pIc);
	}

	pIc = &psv->chnCtx[0].chnIc;
	if (parseEncArgv(psv, pIc, argc, argv) < 0) {
		CVI_VENC_ERR("parseEncArgv\n");
		return -1;
	}

	if (pcic->numChn == 0) {
		if (pcic->testMode == VCODEC_SINGLE_STREAM_MODE ||
			pcic->testMode == JPEG_CONTI_ENCODE_MODE)
			pcic->numChn = 1;
	}

	if (pcic->numChn < 0) {
		CVI_VENC_ERR("vcodec config failed\n");
		return -1;
	}

	for (CVI_S32 s32ChnIdx = 0; s32ChnIdx < pcic->numChn; s32ChnIdx++) {
		pIc = &psv->chnCtx[s32ChnIdx].chnIc;
		if (checkInputCfg(pIc) < 0) {
			CVI_VENC_ERR("checkInput (chn %d) failure\n", s32ChnIdx);
			return -1;
		}
	}

	return 0;
}

CVI_VOID initInputCfg(commonInputCfg *pcic, chnInputCfg *pIc)
{
	SAMPLE_COMM_VENC_InitCommonInputCfg(pcic);
	SAMPLE_COMM_VENC_InitChnInputCfg(pIc);
}

CVI_S32 parseEncArgv(sampleVenc *psv, chnInputCfg *pIc, CVI_S32 argc, char **argv)
{
	CVI_S32 ch, idx, ret;
	SAMPLE_ARG arg = { 0 };
	struct option long_options[MAX_VENC_OPTIONS + 1];

	memset((void *)long_options, 0, sizeof(long_options));

	CVI_VENC_TRACE("\n");

	for (idx = 0; idx < MAX_VENC_OPTIONS; idx++) {
		if (venc_long_option_ext[idx].opt.name == NULL)
			break;

		if (idx >= MAX_VENC_OPTIONS) {
			CVI_VENC_ERR("too many options\n");
			return -1;
		}

		memcpy(&long_options[idx], &venc_long_option_ext[idx].opt, sizeof(struct option));
	}

	while ((ch = getopt_long(argc, argv, "c:w:h:i:o:n:", long_options, &idx)) != -1) {
		CVI_VENC_TRACE("ch = %c, idx = %d, optind = %d\n", ch, idx, optind);

		switch (ch) {
		case 'c':
			strcpy(pIc->codec, optarg);
			break;
		case 'w':
			pIc->width = atoi(optarg);
			break;
		case 'h':
			pIc->height = atoi(optarg);
			break;
		case 'i':
			strcpy(pIc->input_path, optarg);
			break;
		case 'o':
			strcpy(pIc->output_path, optarg);
			break;
		case 'n':
			pIc->num_frames = atoi(optarg);
			break;
		case 0:
			ret = checkArg(idx, &arg);
			if (ret != CVI_SUCCESS) {
				CVI_VENC_ERR("checkArg, %d\n", ret);
				print_help(argv);
				return ret;
			}

			if (!strcmp(long_options[idx].name, "getBsMode")) {
				pIc->bsMode = arg.ival;
				CVI_VENC_TRACE("bsMode = %d\n", pIc->bsMode);
			} else if (!strcmp(long_options[idx].name, "profile")) {
				pIc->u32Profile = arg.uval;
			} else if (!strcmp(long_options[idx].name, "rcMode")) {
				pIc->rcMode = arg.ival;
			} else if (!strcmp(long_options[idx].name, "iqp")) {
				pIc->iqp = arg.ival;
			} else if (!strcmp(long_options[idx].name, "pqp")) {
				pIc->pqp = arg.ival;
			} else if (!strcmp(long_options[idx].name, "ipQpDelta")) {
				pIc->s32IPQpDelta = arg.ival;
			} else if (!strcmp(long_options[idx].name, "bgQpDelta")) {
				pIc->s32BgQpDelta = arg.ival;
			} else if (!strcmp(long_options[idx].name, "viQpDelta")) {
				pIc->s32ViQpDelta = arg.ival;
			} else if (!strcmp(long_options[idx].name, "gop")) {
				pIc->gop = arg.ival;
			} else if (!strcmp(long_options[idx].name, "gopMode")) {
				pIc->gopMode = arg.uval;
			} else if (!strcmp(long_options[idx].name, "bitrate")) {
				pIc->bitrate = arg.ival;
				CVI_VENC_CFG("bitrate = %d\n", pIc->bitrate);
			} else if (!strcmp(long_options[idx].name, "initQp")) {
				pIc->firstFrmstartQp = arg.ival;
			} else if (!strcmp(long_options[idx].name, "minIqp")) {
				pIc->minIqp = arg.ival;
			} else if (!strcmp(long_options[idx].name, "maxIqp")) {
				pIc->maxIqp = arg.ival;
			} else if (!strcmp(long_options[idx].name, "minQp")) {
				pIc->minQp = arg.ival;
			} else if (!strcmp(long_options[idx].name, "maxQp")) {
				pIc->maxQp = arg.ival;
			} else if (!strcmp(long_options[idx].name, "srcFramerate")) {
				pIc->srcFramerate = arg.ival;
			} else if (!strcmp(long_options[idx].name, "framerate")) {
				pIc->framerate = arg.ival;
			} else if (!strcmp(long_options[idx].name, "vfps")) {
				pIc->bVariFpsEn = arg.ival;
			} else if (!strcmp(long_options[idx].name, "quality")) {
				pIc->quality = arg.ival;
			} else if (!strcmp(long_options[idx].name, "maxbitrate")) {
				pIc->maxbitrate = arg.ival;
			} else if (!strcmp(long_options[idx].name, "changePos")) {
				pIc->s32ChangePos = arg.ival;
			} else if (!strcmp(long_options[idx].name, "minStillPercent")) {
				pIc->s32MinStillPercent = arg.ival;
			} else if (!strcmp(long_options[idx].name, "maxStillQp")) {
				pIc->u32MaxStillQP = arg.uval;
			} else if (!strcmp(long_options[idx].name, "motionSense")) {
				pIc->u32MotionSensitivity = arg.uval;
			} else if (!strcmp(long_options[idx].name, "avbrFrmLostOpen")) {
				pIc->s32AvbrFrmLostOpen = arg.ival;
			} else if (!strcmp(long_options[idx].name, "avbrFrmGap")) {
				pIc->s32AvbrFrmGap = arg.ival;
			} else if (!strcmp(long_options[idx].name, "avbrPureStillThr")) {
				pIc->s32AvbrPureStillThr = arg.ival;
			} else if (!strcmp(long_options[idx].name, "statTime")) {
				pIc->statTime = arg.ival;
			} else if (!strcmp(long_options[idx].name, "testMode")) {
				psv->commonIc.testMode = arg.uval;
			} else if (!strcmp(long_options[idx].name, "getstream-timeout")) {
				pIc->getstream_timeout = arg.ival;
			} else if (!strcmp(long_options[idx].name, "sendframe-timeout")) {
				pIc->sendframe_timeout = arg.ival;
			} else if (!strcmp(long_options[idx].name, "ifInitVb")) {
				psv->commonIc.ifInitVb = arg.ival;
			} else if (!strcmp(long_options[idx].name, "vbMode")) {
				psv->commonIc.vbMode = arg.ival;
			} else if (!strcmp(long_options[idx].name, "yuvFolder")) {
				strcpy(psv->commonIc.yuvFolder, optarg);
			} else if (!strcmp(long_options[idx].name, "bindmode")) {
				psv->commonIc.bindmode = arg.uval;
			} else if (!strcmp(long_options[idx].name, "pixel_format")) {
				pIc->pixel_format = arg.ival;
			} else if (!strcmp(long_options[idx].name, "posX")) {
				pIc->posX = arg.ival;
			} else if (!strcmp(long_options[idx].name, "posY")) {
				pIc->posY = arg.ival;
			} else if (!strcmp(long_options[idx].name, "inWidth")) {
				pIc->inWidth = arg.ival;
			} else if (!strcmp(long_options[idx].name, "inHeight")) {
				pIc->inHeight = arg.ival;
			} else if (!strcmp(long_options[idx].name, "bufSize")) {
				pIc->bitstreamBufSize = arg.uval;
			} else if (!strcmp(long_options[idx].name, "single_core")) {
				pIc->single_core = arg.ival;
			} else if (!strcmp(long_options[idx].name, "single_LumaBuf")) {
				pIc->single_LumaBuf = arg.ival;
			} else if (!strcmp(long_options[idx].name, "forceIdr")) {
				pIc->forceIdr = arg.ival;
			} else if (!strcmp(long_options[idx].name, "resetGop")) {
				pIc->u32ResetGop = arg.uval;
			} else if (!strcmp(long_options[idx].name, "chgNum")) {
				pIc->chgNum = arg.ival;
			} else if (!strcmp(long_options[idx].name, "chgBitrate")) {
				pIc->chgBitrate = arg.ival;
			} else if (!strcmp(long_options[idx].name, "chgFramerate")) {
				pIc->chgFramerate = arg.ival;
			} else if (!strcmp(long_options[idx].name, "tempLayer")) {
				pIc->tempLayer = arg.ival;
			} else if (!strcmp(long_options[idx].name, "roiCfgFile")) {
				strcpy(pIc->roiCfgFile, optarg);
			} else if (!strcmp(long_options[idx].name, "qpMapCfgFile")) {
				strcpy(pIc->qpMapCfgFile, optarg);
			} else if (!strcmp(long_options[idx].name, "bgInterval")) {
				pIc->bgInterval = arg.ival;
			} else if (!strcmp(long_options[idx].name, "frame_lost")) {
				pIc->frameLost = arg.ival;
			} else if (!strcmp(long_options[idx].name, "frame_lost_gap")) {
				pIc->frameLostGap = arg.uval;
			} else if (!strcmp(long_options[idx].name, "frame_lost_thr")) {
				pIc->frameLostBspThr = arg.uval;
			} else if (!strcmp(long_options[idx].name, "MCUPerECS")) {
				pIc->MCUPerECS = arg.ival;
			} else if (!strcmp(long_options[idx].name, "single_EsBuf")) {
				psv->commonIc.bSingleEsBuf_jpege = arg.ival;
			} else if (!strcmp(long_options[idx].name, "single_EsBuf_264")) {
				psv->commonIc.bSingleEsBuf_h264e = arg.ival;
			} else if (!strcmp(long_options[idx].name, "single_EsBuf_265")) {
				psv->commonIc.bSingleEsBuf_h265e = arg.ival;
			} else if (!strcmp(long_options[idx].name, "single_EsBufSize")) {
				psv->commonIc.singleEsBufSize_jpege = arg.ival;
			} else if (!strcmp(long_options[idx].name, "single_EsBufSize_264")) {
				psv->commonIc.singleEsBufSize_h264e = arg.ival;
			} else if (!strcmp(long_options[idx].name, "single_EsBufSize_265")) {
				psv->commonIc.singleEsBufSize_h265e = arg.ival;
			} else if (!strcmp(long_options[idx].name, "numChn")) {
				psv->commonIc.numChn = arg.ival;
			} else if (!strcmp(long_options[idx].name, "chn")) {
				pIc = &psv->chnCtx[arg.uval].chnIc;
			} else if (!strcmp(long_options[idx].name, "viWidth")) {
				psv->commonIc.u32ViWidth = arg.uval;
			} else if (!strcmp(long_options[idx].name, "viHeight")) {
				psv->commonIc.u32ViHeight = arg.uval;
			} else if (!strcmp(long_options[idx].name, "vpssWidth")) {
				psv->commonIc.u32VpssWidth = arg.uval;
			} else if (!strcmp(long_options[idx].name, "vpssHeight")) {
				psv->commonIc.u32VpssHeight = arg.uval;
			} else if (!strcmp(long_options[idx].name, "vpssSrcPath")) {
				strcpy(pIc->vpssSrcPath, optarg);
			} else if (!strcmp(long_options[idx].name, "user_data1")) {
				strcpy(pIc->user_data[0], optarg);
			} else if (!strcmp(long_options[idx].name, "user_data2")) {
				strcpy(pIc->user_data[1], optarg);
			} else if (!strcmp(long_options[idx].name, "user_data3")) {
				strcpy(pIc->user_data[2], optarg);
			} else if (!strcmp(long_options[idx].name, "user_data4")) {
				strcpy(pIc->user_data[3], optarg);
			} else if (!strcmp(long_options[idx].name, "h265RefreshType")) {
				psv->commonIc.h265RefreshType = arg.ival;
			} else if (!strcmp(long_options[idx].name, "initialDelay")) {
				pIc->initialDelay = arg.ival;
			} else if (!strcmp(long_options[idx].name, "jpegMarkerOrder")) {
				psv->commonIc.jpegMarkerOrder = arg.ival;
			} else if (!strcmp(long_options[idx].name, "intraCost")) {
				pIc->u32IntraCost = arg.uval;
			} else if (!strcmp(long_options[idx].name, "thrdLv")) {
				pIc->u32ThrdLv = arg.uval;
			} else if (!strcmp(long_options[idx].name, "bgEnhanceEn")) {
				pIc->bBgEnhanceEn = arg.ival;
			} else if (!strcmp(long_options[idx].name, "bgDeltaQp")) {
				pIc->s32BgDeltaQp = arg.ival;
			} else if (!strcmp(long_options[idx].name, "h264EntropyMode")) {
				pIc->h264EntropyMode = arg.uval;
			} else if (!strcmp(long_options[idx].name, "h264ChromaQpOffset")) {
				pIc->h264ChromaQpOffset = arg.ival;
			} else if (!strcmp(long_options[idx].name, "h265CbQpOffset")) {
				pIc->h265CbQpOffset = arg.ival;
			} else if (!strcmp(long_options[idx].name, "h265CrQpOffset")) {
				pIc->h265CrQpOffset = arg.ival;
			} else if (!strcmp(long_options[idx].name, "maxIprop")) {
				pIc->maxIprop = arg.ival;
			} else if (!strcmp(long_options[idx].name, "rowQpDelta")) {
				pIc->u32RowQpDelta = arg.uval;
			} else if (!strcmp(long_options[idx].name, "superFrmMode")) {
				pIc->enSuperFrmMode = arg.uval;
			} else if (!strcmp(long_options[idx].name, "superIBitsThr")) {
				pIc->u32SuperIFrmBitsThr = arg.uval;
			} else if (!strcmp(long_options[idx].name, "superPBitsThr")) {
				pIc->u32SuperPFrmBitsThr = arg.uval;
			} else if (!strcmp(long_options[idx].name, "maxReEnc")) {
				pIc->s32MaxReEncodeTimes = arg.ival;
			} else if (!strcmp(long_options[idx].name, "aspectRatioInfoPresentFlag")) {
				pIc->aspectRatioInfoPresentFlag = arg.uval;
			} else if (!strcmp(long_options[idx].name, "aspectRatioIdc")) {
				pIc->aspectRatioIdc = arg.uval;
			} else if (!strcmp(long_options[idx].name, "overscanInfoPresentFlag")) {
				pIc->overscanInfoPresentFlag = arg.uval;
			} else if (!strcmp(long_options[idx].name, "overscanAppropriateFlag")) {
				pIc->overscanAppropriateFlag = arg.uval;
			} else if (!strcmp(long_options[idx].name, "sarWidth")) {
				pIc->sarWidth = arg.uval;
			} else if (!strcmp(long_options[idx].name, "sarHeight")) {
				pIc->sarHeight = arg.uval;
			} else if (!strcmp(long_options[idx].name, "timingInfoPresentFlag")) {
				pIc->timingInfoPresentFlag = arg.uval;
			} else if (!strcmp(long_options[idx].name, "fixedFrameRateFlag")) {
				pIc->fixedFrameRateFlag = arg.uval;
			} else if (!strcmp(long_options[idx].name, "numUnitsInTick")) {
				pIc->numUnitsInTick = arg.uval;
			} else if (!strcmp(long_options[idx].name, "timeScale")) {
				pIc->timeScale = arg.uval;
			} else if (!strcmp(long_options[idx].name, "videoSignalTypePresentFlag")) {
				pIc->videoSignalTypePresentFlag = arg.uval;
			} else if (!strcmp(long_options[idx].name, "videoFormat")) {
				pIc->videoFormat = arg.uval;
			} else if (!strcmp(long_options[idx].name, "videoFullRangeFlag")) {
				pIc->videoFullRangeFlag = arg.uval;
			} else if (!strcmp(long_options[idx].name, "colourDescriptionPresentFlag")) {
				pIc->colourDescriptionPresentFlag = arg.uval;
			} else if (!strcmp(long_options[idx].name, "colourPrimaries")) {
				pIc->colourPrimaries = arg.uval;
			} else if (!strcmp(long_options[idx].name, "transferCharacteristics")) {
				pIc->transferCharacteristics = arg.uval;
			} else if (!strcmp(long_options[idx].name, "matrixCoefficients")) {
				pIc->matrixCoefficients = arg.uval;
			} else if (!strcmp(long_options[idx].name, "testUbrEn")) {
				pIc->bTestUbrEn = arg.ival;
			} else if (!strcmp(long_options[idx].name, "frameQp")) {
				pIc->u32FrameQp = arg.uval;
			} else if (!strcmp(long_options[idx].name, "esBufQueueEn")) {
				pIc->bEsBufQueueEn = arg.uval;
			} else if (!strcmp(long_options[idx].name, "isoSendFrmEn")) {
				pIc->bIsoSendFrmEn = arg.uval;
			} else if (!strcmp(long_options[idx].name, "sensorEn")) {
				pIc->bSensorEn = arg.uval;
			} else if (!strcmp(long_options[idx].name, "sliceSplitCnt")) {
				pIc->u32SliceCnt = arg.uval;
			} else if (!strcmp(long_options[idx].name, "disabledblk")) {
				pIc->bDisableDeblk = arg.uval;
			} else if (!strcmp(long_options[idx].name, "betaOffset")) {
				pIc->betaOffset = arg.ival;
			} else if (!strcmp(long_options[idx].name, "alphaoffset")) {
				pIc->alphaOffset = arg.ival;
			} else if (!strcmp(long_options[idx].name, "intraPred")) {
				pIc->bIntraPred = arg.uval;
			} else if (!strcmp(long_options[idx].name, "maxQuality")) {
				pIc->maxQuality = arg.uval;
			} else if (!strcmp(long_options[idx].name, "minQuality")) {
				pIc->minQuality = arg.uval;
			} else if (!strcmp(long_options[idx].name, "svcEnable")) {
				pIc->svc_enable = arg.uval;
			} else if (!strcmp(long_options[idx].name, "fgProtectEn")) {
				pIc->fg_protect_en = arg.uval;
			} else if (!strcmp(long_options[idx].name, "fgDealtQp")) {
				pIc->fg_dealt_qp = arg.uval;
			} else if (!strcmp(long_options[idx].name, "cplxSceneDetectEn")) {
				pIc->complex_scene_detect_en = arg.uval;
			} else if (!strcmp(long_options[idx].name, "cplxSceneLowTh")) {
				pIc->complex_scene_low_th = arg.uval;
			} else if (!strcmp(long_options[idx].name, "cplxSceneHightTh")) {
				pIc->complex_scene_hight_th = arg.uval;
			} else if (!strcmp(long_options[idx].name, "middleMinPercent")) {
				pIc->middle_min_percent = arg.uval;
			} else if (!strcmp(long_options[idx].name, "cplxMinPercent")) {
				pIc->complex_min_percent = arg.uval;
			}else {
				CVI_VENC_TRACE("not exist name = %s\n", long_options[idx].name);
				print_help(argv);
				return -1;
			}
			break;
		default:
			CVI_VENC_TRACE("ch = %c\n", ch);
			print_help(argv);
			break;
		}
	}

	if (optind < argc)
		print_help(argv);

	return 0;
}

#define BYPASS_SB 0
#define VI_UT_MMAP_SIZE	0x2000
#define QBUF_NUM	2

typedef struct _VI_UT_CTX {
	SIZE_S stSize;
	PIXEL_FORMAT_E enPixelFormat;
	COMPRESS_MODE_E enCompressMode;
	CVI_U32 u32Align;
	VB_BLK blk[QBUF_NUM];
	CVI_U32 is_enable_sensor	: 1;
	CVI_U32 is_be_online		: 1;
	CVI_U32 is_post_online		: 1;
	CVI_U32 is_patgen_enable	: 1;
	CVI_U32 is_hdr_enable		: 1;
	CVI_U32 is_vpss_online		: 1;
} VI_UT_CTX;

int vi_fd = -1;
static SAMPLE_VI_CONFIG_S stViConfig;
VI_UT_CTX vi_ut_ctx;

CVI_U64 plane_0_phy_addr;
CVI_U64 plane_1_phy_addr;

CVI_S32 _sys_config_online_mode(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_VPSS_MODE_S	stVIVPSSMode;
	VPSS_MODE_S	stVPSSMode;

	/************************************************
	 * Config vpss online mode
	 ************************************************/
	stVPSSMode.enMode = VPSS_MODE_DUAL;

	for (CVI_U8 i = 0; i < VPSS_IP_NUM; ++i) {
		stVPSSMode.aenInput[i] = VPSS_INPUT_MEM;
		stVPSSMode.ViPipe[i] = 0;
	}

	stVIVPSSMode.aenMode[0] = stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_ONLINE;
	stVPSSMode.aenInput[1] = VPSS_INPUT_ISP;

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_SYS_SetVPSSModeEx(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_SetVPSSModeEx failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 vi_ut_plat_vi_init(void)
{
	CVI_S32 s32Ret;
	VI_PIPE ViPipe = 0;
	VI_PIPE_ATTR_S stPipeAttr;
	CVI_S32 i = 0, j = 0;
	CVI_S32 s32DevNum;

	if (vi_ut_ctx.is_enable_sensor) {
		s32Ret = SAMPLE_COMM_VI_StartSensor(&stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "system start sensor failed with %#x\n", s32Ret);
			goto error;
		}
	}

	s32Ret = SAMPLE_COMM_VI_StartDev(&stViConfig.astViInfo[0]);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "VI_StartDev failed with %#x!\n", s32Ret);
		goto error;
	}

	if (vi_ut_ctx.is_enable_sensor) {
		s32Ret = SAMPLE_COMM_VI_StartMIPI(&stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "system start MIPI failed with %#x\n", s32Ret);
			goto error;
		}

		s32Ret = SAMPLE_COMM_VI_SensorProbe(&stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "system sensor probe failed with %#x\n", s32Ret);
			goto error;
		}
	}

	for (i = 0; i < stViConfig.s32WorkingViNum; i++) {
		SAMPLE_VI_INFO_S *pstViInfo = NULL;

		s32DevNum = stViConfig.as32WorkingViId[i];
		pstViInfo = &stViConfig.astViInfo[s32DevNum];
		stPipeAttr.bYuvBypassPath = SAMPLE_COMM_VI_GetYuvBypassSts(pstViInfo->stSnsInfo.enSnsType);

		for (j = 0; j < WDR_MAX_PIPE_NUM; j++) {
			if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
				ViPipe = pstViInfo->stPipeInfo.aPipe[j];
				s32Ret = CVI_VI_CreatePipe(ViPipe, &stPipeAttr);
				if (s32Ret != CVI_SUCCESS) {
					CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
					goto error;
				}

				s32Ret = CVI_VI_StartPipe(ViPipe);
				if (s32Ret != CVI_SUCCESS) {
					CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_StartPipe failed with %#x!\n", s32Ret);
					goto error;
				}
			}
		}
	}

	if (vi_ut_ctx.is_enable_sensor) {
		s32Ret = SAMPLE_COMM_VI_CreateIsp(&stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "VI_CreateIsp failed with %#x!\n", s32Ret);
			goto error;
		}
	}

#if defined(__CV181X__)
	if (vi_ut_ctx.is_patgen_enable)
		system("echo 1,0 > /sys/module/mars_vi/parameters/csi_patgen_en");
	else
		system("echo 0,0 > /sys/module/mars_vi/parameters/csi_patgen_en");
#elif defined(__CV180X__)
	if (vi_ut_ctx.is_patgen_enable)
		system("echo 1,0 > /sys/module/phobos_vi/parameters/csi_patgen_en");
	else
		system("echo 0,0 > /sys/module/phobos_vi/parameters/csi_patgen_en");
#endif

	s32Ret = SAMPLE_COMM_VI_StartViChn(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "VI_StartViChn failed with %#x!\n", s32Ret);
		goto error;
	}
error:
	return s32Ret;
}

#ifdef OUTPUT_SCALED_PIC
static CVI_S32 _SAMPLE_VENC_writeFrame(VIDEO_FRAME_INFO_S *pstVideoFrame, FILE *fp)
{
	VIDEO_FRAME_S *pstVFrame = &pstVideoFrame->stVFrame;
	CVI_U32 u32len, u32DataLen;

	for (int i = 0; i < 3; ++i) {
		u32DataLen = pstVFrame->u32Stride[i] * pstVFrame->u32Height;
		if (u32DataLen == 0)
			continue;

		if (i > 0 && ((pstVFrame->enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(pstVFrame->enPixelFormat == PIXEL_FORMAT_NV12) ||
			(pstVFrame->enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		pstVFrame->pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVFrame->u64PhyAddr[i], pstVFrame->u32Length[i]);

		CVI_TRACE_LOG(CVI_DBG_INFO, "plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, pstVFrame->u64PhyAddr[i],
			   pstVFrame->pu8VirAddr[i],
			   pstVFrame->u32Stride[i]);
		CVI_TRACE_LOG(CVI_DBG_INFO, " data_len(%d) plane_len(%d)\n",
			      u32DataLen, pstVFrame->u32Length[i]);
		u32len = fwrite(pstVFrame->pu8VirAddr[i], u32DataLen, 1, fp);
		if (u32len <= 0) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "fwrite data(%d) error\n", i);
			break;
		}
		CVI_SYS_Munmap(pstVFrame->pu8VirAddr[i], pstVFrame->u32Length[i]);
	}

	return CVI_SUCCESS;
}
#endif

CVI_S32 SAMPLE_VENC_START(sampleVenc *psv)
{
	commonInputCfg *pcic = &psv->commonIc;
	chnInputCfg *pIc = &psv->chnCtx[0].chnIc;
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (!strcmp(pIc->codec, "265") ||
		!strcmp(pIc->codec, "264") ||
		!strcmp(pIc->codec, "mjp") ||
		!strcmp(pIc->codec, "jpg") ||
		!strcmp(pIc->codec, "multi")) {

		if (pcic->ifInitVb) {
			exitSysAndVb();

			s32Ret = initSysAndVb(psv);
			if (s32Ret != CVI_SUCCESS) {
				CVI_VENC_ERR("fail to init sys and vb, 0x%x\n", s32Ret);
				return s32Ret;
			}
		}

		s32Ret = SAMPLE_COMM_VENC_SetModParam(pcic);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("SAMPLE_COMM_VENC_SetModParam failure\n");
			return CVI_FAILURE;
		}

		for (CVI_S32 s32ChnIdx = 0; s32ChnIdx < pcic->numChn; s32ChnIdx++) {
			s32Ret = _SAMPLE_VENC_INIT_CHANNEL(psv, s32ChnIdx);
			if (s32Ret) {
				CVI_VENC_ERR("[Chn %d]sample venc init failed\n", s32ChnIdx);
				return CVI_FAILURE;
			}

			if (pcic->testMode == JPEG_CONTI_ENCODE_MODE) {
				VENC_CHN_ATTR_S stChnAttr;

				s32Ret = API_COST_TIME_LOG(CVI_VENC_GetChnAttr(s32ChnIdx, &stChnAttr), "CVI_VENC_GetChnAttr");
				if (s32Ret != CVI_SUCCESS) {
					CVI_VENC_ERR("CVI_VENC_GetChnAttr [%d] failed with %d\n",
							s32ChnIdx, s32Ret);
					return s32Ret;
				}

				stChnAttr.stVencAttr.u32PicWidth = pIc->width;
				stChnAttr.stVencAttr.u32PicHeight = pIc->height;

				s32Ret = API_COST_TIME_LOG(CVI_VENC_SetChnAttr(s32ChnIdx, &stChnAttr), "CVI_VENC_SetChnAttr");
				if (s32Ret != CVI_SUCCESS) {
					CVI_VENC_ERR("CVI_VENC_SetChnAttr [%d] failed with %d\n",
							s32ChnIdx, s32Ret);
					return s32Ret;
				}
			}
		}

		if (pcic->bThreadDisable == CVI_FALSE) {
			for (CVI_S32 s32ChnIdx = 0; s32ChnIdx < pcic->numChn; s32ChnIdx++)
				SAMPLE_VENC_StartGetStream(&psv->chnCtx[s32ChnIdx], s32ChnIdx);
		}
	} else {
		CVI_VENC_ERR("codec = %s\n", pIc->codec);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_VENC_STOP(sampleVenc *psv)
{
	commonInputCfg *pcic = &psv->commonIc;
	chnInputCfg *pIc = &psv->chnCtx[0].chnIc;
	vencChnCtx *pvecc;
	CVI_S32 s32ChnIdx;

	if (!strcmp(pIc->codec, "265") ||
		!strcmp(pIc->codec, "264") ||
		!strcmp(pIc->codec, "mjp") ||
		!strcmp(pIc->codec, "jpg") ||
		!strcmp(pIc->codec, "multi")) {

		for (s32ChnIdx = 0; s32ChnIdx < pcic->numChn; s32ChnIdx++) {
			pvecc = &psv->chnCtx[s32ChnIdx];
			pIc = &pvecc->chnIc;

			#if 0
			_venc_unbind_source(pIc, s32ChnIdx);
			#endif
			if (pcic->testMode == JPEG_CONTI_ENCODE_MODE) {
				CVI_S32 s32Ret;

				if (pcic->bThreadDisable == CVI_FALSE && gs_VencTask[s32ChnIdx] != 0) {
					pthread_join(gs_VencTask[s32ChnIdx], CVI_NULL);
					CVI_VENC_SYNC("GetVencStreamProc done\n");

					gs_VencTask[s32ChnIdx] = 0;
				}

				s32Ret = API_COST_TIME_LOG(CVI_VENC_StopRecvFrame(s32ChnIdx), "CVI_VENC_StopRecvFrame");
				if (s32Ret != CVI_SUCCESS) {
					CVI_VENC_ERR("CVI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",
							s32ChnIdx, s32Ret);
					return CVI_FAILURE;
				}

				s32Ret = API_COST_TIME_LOG(CVI_VENC_ResetChn(s32ChnIdx), "CVI_VENC_ResetChn");
				if (s32Ret != CVI_SUCCESS) {
					CVI_VENC_ERR("CVI_VENC_ResetChn vechn[%d] failed with %#x!\n",
							s32ChnIdx, s32Ret);
					return CVI_FAILURE;
				}
			} else {
				SAMPLE_COMM_VENC_Stop(s32ChnIdx);
			}
		}

		for (s32ChnIdx = 0; s32ChnIdx < pcic->numChn; s32ChnIdx++) {
			pvecc = &psv->chnCtx[s32ChnIdx];
			pIc = &pvecc->chnIc;

			if (pvecc->pFile) {
				fclose(pvecc->pFile);
				pvecc->pFile = NULL;
			}

			if (pIc->bind_mode == VENC_BIND_DISABLE) {
				if (pvecc->fpSrc) {
					fclose(pvecc->fpSrc);
					pvecc->fpSrc = NULL;
				}
			}

			if (pcic->ifInitVb) {
				CVI_VENC_TRACE("s32ChnIdx = %d, pstFrameInfo = 0x%p\n",
						s32ChnIdx, pvecc->pstFrameInfo);
				free_frame(pvecc->pstFrameInfo);
			}

			if (pvecc->pu8QpMap) {
				free(pvecc->pu8QpMap);
				pvecc->pu8QpMap = NULL;
			}
			if (vpssVB[s32ChnIdx] != VB_INVALID_POOLID) {
				CVI_VPSS_DetachVbPool(0, s32ChnIdx);
				CVI_VB_DestroyPool(vpssVB[s32ChnIdx]);
				vpssVB[s32ChnIdx] = VB_INVALID_POOLID;
			}
		}
	} else {
		CVI_VENC_ERR("codec = %s\n", pIc->codec);
		return -1;
	}

	return 0;
}

CVI_S32 SAMPLE_VENC_MOVE_TO_STOP_STATE(sampleVenc *psv)
{
	int idx;
	commonInputCfg *pcic = &psv->commonIc;
	vencChnCtx *pvecc;

	for (idx = 0; idx < pcic->numChn; idx++) {
		pvecc = &psv->chnCtx[idx];
		pvecc->chnStat = CHN_STAT_STOP;
	}
	return 0;
}

static CVI_S32 checkArg(CVI_S32 entryIdx, SAMPLE_ARG *pArg)
{
	CVI_VENC_TRACE("entryIdx = %d\n", entryIdx);

	if (venc_long_option_ext[entryIdx].type == ARG_INT) {
		pArg->ival = strtoimax(optarg, NULL, 10);
		if ((int64_t)(pArg->ival) < venc_long_option_ext[entryIdx].min ||
		    (int64_t)(pArg->ival) > venc_long_option_ext[entryIdx].max) {
			CVI_VENC_ERR("%s = %d, min = %"PRId64", max = %"PRId64"\n",
					venc_long_option_ext[entryIdx].opt.name,
					pArg->ival,
					venc_long_option_ext[entryIdx].min,
					venc_long_option_ext[entryIdx].max);
			return CVI_FAILURE;
		}
	} else if (venc_long_option_ext[entryIdx].type == ARG_UINT) {
		pArg->uval = strtoumax(optarg, NULL, 10);
		if ((int64_t)(pArg->uval) < venc_long_option_ext[entryIdx].min ||
		    (int64_t)(pArg->uval) > venc_long_option_ext[entryIdx].max) {
			CVI_VENC_ERR("%s = %u, min = %"PRId64", max = %"PRId64"\n",
					venc_long_option_ext[entryIdx].opt.name,
					pArg->uval,
					venc_long_option_ext[entryIdx].min,
					venc_long_option_ext[entryIdx].max);
			return CVI_FAILURE;
		}
	} else if (venc_long_option_ext[entryIdx].type == ARG_STRING) {
		if (optarg == NULL) {
			CVI_VENC_ERR("%s = NULL\n", venc_long_option_ext[entryIdx].opt.name);
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 checkInputCfg(chnInputCfg *pIc)
{
	if (!strcmp(pIc->codec, "264") || !strcmp(pIc->codec, "265")) {

		CVI_VENC_CFG("framerate = %d\n", pIc->framerate);

		if (pIc->gop < 1) {
			if (!strcmp(pIc->codec, "264"))
				pIc->gop = DEF_264_GOP;
			else
				pIc->gop = DEF_GOP;
		}
		CVI_VENC_CFG("gop = %d\n", pIc->gop);

		if (!strcmp(pIc->codec, "265")) {
			if (pIc->single_LumaBuf > 0) {
				CVI_VENC_ERR("single_LumaBuf only supports H.264\n");
				pIc->single_LumaBuf = 0;
			}
		}
		pIc->iqp = (pIc->iqp >= 0) ? pIc->iqp : DEF_IQP;
		pIc->pqp = (pIc->pqp >= 0) ? pIc->pqp : DEF_PQP;

		if (pIc->rcMode == -1) {
			pIc->rcMode = SAMPLE_RC_FIXQP;
		}

		if (pIc->rcMode == SAMPLE_RC_CBR ||
			pIc->rcMode == SAMPLE_RC_VBR ||
			pIc->rcMode == SAMPLE_RC_AVBR ||
			pIc->rcMode == SAMPLE_RC_QPMAP ||
			pIc->rcMode == SAMPLE_RC_UBR) {

			if (pIc->rcMode == SAMPLE_RC_CBR ||
				pIc->rcMode == SAMPLE_RC_UBR) {
				if (pIc->bitrate <= 0) {
					CVI_VENC_ERR("CBR / UBR bitrate must be not less than 0");
					return -1;
				}
				CVI_VENC_CFG("bitrate = %d\n", pIc->bitrate);
			} else if (pIc->rcMode == SAMPLE_RC_VBR) {
				if (pIc->maxbitrate <= 0) {
					CVI_VENC_ERR("VBR must be not less than 0");
					return -1;
				}
				CVI_VENC_CFG("RC_VBR, maxbitrate = %d\n", pIc->maxbitrate);
			}

			pIc->firstFrmstartQp =
				(pIc->firstFrmstartQp < 0 ||
				 pIc->firstFrmstartQp > 51) ? 63 : pIc->firstFrmstartQp;
			CVI_VENC_CFG("firstFrmstartQp = %d\n", pIc->firstFrmstartQp);

			pIc->maxIqp = (pIc->maxIqp >= 0) ? pIc->maxIqp : DEF_264_MAXIQP;
			pIc->minIqp = (pIc->minIqp >= 0) ? pIc->minIqp : DEF_264_MINIQP;
			pIc->maxQp = (pIc->maxQp >= 0) ? pIc->maxQp : DEF_264_MAXQP;
			pIc->minQp = (pIc->minQp >= 0) ? pIc->minQp : DEF_264_MINQP;
			CVI_VENC_CFG("maxQp = %d, minQp = %d, maxIqp = %d, minIqp = %d\n",
					pIc->maxQp,
					pIc->minQp,
					pIc->maxIqp,
					pIc->minIqp);

			if (pIc->statTime == 0) {
				pIc->statTime = DEF_STAT_TIME;
			}
			CVI_VENC_CFG("statTime = %d\n", pIc->statTime);
		} else if (pIc->rcMode == SAMPLE_RC_FIXQP) {
			if (pIc->firstFrmstartQp != -1) {
				CVI_VENC_WARN("firstFrmstartQp is invalid in FixQP mode\n");
				pIc->firstFrmstartQp = -1;
			}

			pIc->bitrate = 0;
			CVI_VENC_CFG("RC_FIXQP, iqp = %d, pqp = %d\n",
				pIc->iqp,
				pIc->pqp);
		} else {
			CVI_VENC_ERR("codec = %s, rcMode = %d, not supported RC mode\n", pIc->codec, pIc->rcMode);
			return -1;
		}
	} else if (!strcmp(pIc->codec, "mjp") || !strcmp(pIc->codec, "jpg")) {
		if (pIc->rcMode == -1) {
			pIc->rcMode = SAMPLE_RC_FIXQP;
			pIc->quality = (pIc->quality != -1) ? pIc->quality : 0;
		} else if (pIc->rcMode == SAMPLE_RC_FIXQP) {
			clip(Q_TABLE_MIN, Q_TABLE_MAX, pIc->quality);
		} else if (pIc->rcMode == SAMPLE_RC_CBR || pIc->rcMode == SAMPLE_RC_VBR) {
			clip(Q_TABLE_MIN, Q_TABLE_MAX, pIc->minQuality);
			clip(pIc->minQuality, Q_TABLE_MAX, pIc->maxQuality);
		}
	} else {
		CVI_VENC_ERR("codec = %s\n", pIc->codec);
		return -1;
	}

	return 0;
}

static CVI_U32 _SAMPLE_VENC_INIT_CHANNEL(sampleVenc *psv, CVI_U32 chnNum)
{
	commonInputCfg *pcic = &psv->commonIc;
	chnInputCfg *pIc = &psv->chnCtx[chnNum].chnIc;
	vencChnCtx *pvecc = &psv->chnCtx[chnNum];
	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S inFrmSize;
	char file_ext[16];

	pvecc->enPixelFormat = vencMapPixelFormat(pIc->pixel_format);
	pvecc->VencChn = chnNum;
	pvecc->enGopMode = pIc->gopMode;
	pvecc->s32FbCnt = 1;

	if (!strcmp(pIc->codec, "265"))
		pvecc->enPayLoad = PT_H265;
	else if (!strcmp(pIc->codec, "264"))
		pvecc->enPayLoad = PT_H264;
	else if (!strcmp(pIc->codec, "mjp"))
		pvecc->enPayLoad = PT_MJPEG;
	else if (!strcmp(pIc->codec, "jpg"))
		pvecc->enPayLoad = PT_JPEG;

	if (pvecc->enPayLoad == PT_H264)
		pvecc->u32Profile = pIc->u32Profile;
	else
		pvecc->u32Profile = 0;

	pvecc->stSize.u32Width = pIc->width;
	pvecc->stSize.u32Height = pIc->height;

	if (pIc->inWidth || pIc->inHeight) {
		inFrmSize.u32Width = pIc->inWidth;
		inFrmSize.u32Height = pIc->inHeight;
	} else {
		inFrmSize = pvecc->stSize;
	}

	pvecc->u32FrameSize = getSrcFrameSizeByPixelFormat(
		pvecc->stSize.u32Width, pvecc->stSize.u32Height, pvecc->enPixelFormat);

	pvecc->enRcMode = pIc->rcMode;
	CVI_VENC_TRACE("enPayLoad = %d, enRcMode = %d, bsMode = %d\n",
		       pvecc->enPayLoad, pvecc->enRcMode, pIc->bsMode);

	pvecc->enSize = getEnSize(pIc->width, pIc->height);
	if (pvecc->enSize == PIC_BUTT) {
		CVI_VENC_ERR("unsupport size %d x %d\n", pIc->width, pIc->height);
		return CVI_FAILURE;
	}

	if (pvecc->enPayLoad != PT_JPEG) {
		s32Ret = SAMPLE_COMM_VENC_GetGopAttr(pvecc->enGopMode, &pvecc->stGopAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("Venc Get GopAttr for %#x!\n", s32Ret);
			return CVI_FAILURE;
		}

		if ((pvecc->enPayLoad != PT_JPEG) && (pvecc->enPayLoad != PT_MJPEG)) {
			if (pcic->vbMode == VB_SOURCE_USER) {
				//create vb pool
				CVI_VENC_TRACE("vbMode VB_SOURCE_USER_MODE\n");
				s32Ret =  SAMPLE_COMM_VENC_InitVBPool(pvecc, chnNum);
				if (s32Ret != CVI_SUCCESS)
					CVI_VENC_ERR("init mod common vb fail\n");
				//should attach vb pool after chn create
			}
		}
	}

	s32Ret = SAMPLE_COMM_VENC_Start(
			pIc,
			pvecc->VencChn,
			pvecc->enPayLoad,
			pvecc->enSize,
			pvecc->enRcMode,
			pvecc->u32Profile,
			CVI_FALSE,
			&pvecc->stGopAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("Venc Start failed for %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = _SAMPLE_VENC_LoadCfgFile(pvecc);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("_SAMPLE_VENC_LoadCfgFile %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	if (pcic->ifInitVb) {
		pvecc->pstFrameInfo = allocate_frame(inFrmSize, pvecc->enPixelFormat);
		CVI_VENC_TRACE("pstFrameInfo = 0x%p\n", pvecc->pstFrameInfo);

		if (!pvecc->pstFrameInfo) {
			CVI_VENC_ERR("allocate_frame\n");
			return CVI_FAILURE;
		}

		pvecc->pstVFrame = &pvecc->pstFrameInfo->stVFrame;
	}

	if (pIc->bind_mode == VENC_BIND_DISABLE && strlen(pIc->input_path) != 0) {
		pvecc->fpSrc = fopen(pIc->input_path, "rb");

		if (pvecc->fpSrc == NULL) {
			CVI_VENC_ERR("Input file %s open failed !\n", pIc->input_path);
			return CVI_FAILURE;
		}

		// Get the yuv file size
		fseek(pvecc->fpSrc, 0, SEEK_END);
		pvecc->file_size = ftell(pvecc->fpSrc);
		pvecc->num_frames = pvecc->file_size / pvecc->u32FrameSize;
		CVI_VENC_INFO("file_size %ld\n", pvecc->file_size);
		CVI_VENC_INFO("u32FrameSize %d\n", pvecc->u32FrameSize);

		if ((pIc->num_frames > 0) && (pvecc->num_frames > (CVI_U32)pIc->num_frames))
			pvecc->num_frames = pIc->num_frames;

		CVI_VENC_INFO("chnNum = %d, num_frames = %d\n", chnNum, pvecc->num_frames);

		rewind(pvecc->fpSrc);

		CVI_VENC_TRACE(
				"input_path = %s, file_size = 0x%lX, num_frames = %d, u32FrameSize = 0x%X\n",
				pIc->input_path,
				pvecc->file_size,
				pvecc->num_frames,
				pvecc->u32FrameSize);
	} else {
		pvecc->num_frames = pIc->num_frames;
	}

	SAMPLE_COMM_VENC_GetFilePostfix(pvecc->enPayLoad, file_ext);
	snprintf(pIc->outputFileName, MAX_STRING_LEN, "%s%s",
			pIc->output_path, file_ext);

	CVI_VENC_FLOW("Begin to send frames ..., bsMode = %d\n", pIc->bsMode);

	pvecc->pFile = fopen(pIc->outputFileName, "wb");
	if (pvecc->pFile == NULL) {
		CVI_VENC_ERR("open file err, %s\n", pIc->outputFileName);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 initSysAndVb(sampleVenc *psv)
{
	commonInputCfg *pcic = &psv->commonIc;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32BlkSize;
	VB_CONFIG_S stVbConf;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	stVbConf.u32MaxPoolCnt = 0;

	for (int i = 0; i < pcic->numChn; i++) {
		SIZE_S stSize;
		PIXEL_FORMAT_E enPixelFormat;
		chnInputCfg *pIc;
		CVI_BOOL bRepeated = CVI_FALSE;

		pIc = &psv->chnCtx[i].chnIc;
		enPixelFormat = vencMapPixelFormat(pIc->pixel_format);

		if (pIc->inWidth || pIc->inHeight) {
			stSize.u32Width = pIc->inWidth;
			stSize.u32Height = pIc->inHeight;
		} else {
			stSize.u32Width = pIc->width;
			stSize.u32Height = pIc->height;
		}
		u32BlkSize = VENC_GetPicBufferSize(
						stSize.u32Width,
						stSize.u32Height,
						enPixelFormat,
						DATA_BITWIDTH_8,
						COMPRESS_MODE_NONE);
		u32BlkSize += 0x1000 * 3;
		CVI_VENC_FLOW("u32BlkSize[%d]align[%d]\n",
			u32BlkSize, VENC_ALIGN_W);

		for (CVI_U32 j = 0; j < stVbConf.u32MaxPoolCnt; j++) {
			if (u32BlkSize == stVbConf.astCommPool[j].u32BlkSize) {
				stVbConf.astCommPool[j].u32BlkCnt++;
				bRepeated = CVI_TRUE;
				break;
			}
		}

		if (bRepeated == CVI_FALSE) {
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize = u32BlkSize;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt  = 1;
			stVbConf.u32MaxPoolCnt++;
		}

		if (stVbConf.u32MaxPoolCnt > VB_MAX_COMM_POOLS) {
			CVI_VENC_ERR("u32MaxPoolCnt %d > , VB_MAX_COMM_POOLS %d\n",
				stVbConf.u32MaxPoolCnt, VB_MAX_COMM_POOLS);
			return CVI_FAILURE;
		}
	}

	for (CVI_U32 j = 0; j < stVbConf.u32MaxPoolCnt; j++) {
		SAMPLE_PRT("[Pool %d] u32BlkSize: %d, u32BlkCnt: %d\n",
			j, stVbConf.astCommPool[j].u32BlkSize, stVbConf.astCommPool[j].u32BlkCnt);
	}

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SAMPLE_COMM_SYS_Init, %d\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

static CVI_S32 _SAMPLE_VENC_LoadCfgFile(vencChnCtx *pvecc)
{
	chnInputCfg *pIc = &pvecc->chnIc;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i;

	for (i = 0; i < MAX_NUM_ROI; i++) {
		pvecc->vencRoi[i].u32FrameStart = (CVI_U32)(-1);
		pvecc->vencRoi[i].u32FrameEnd = 0;
	}

	if ((pvecc->enPayLoad == PT_H265) && strlen(pIc->qpMapCfgFile) > 0) {
		CVI_S32 w, h;

		s32Ret = SAMPLE_COMM_VENC_LoadRoiCfgFile(pvecc->vencRoi, pIc->qpMapCfgFile);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("Venc LoadRoiCfgFile failed for %#x!\n", s32Ret);
			return CVI_FAILURE;
		}

		w = (((pvecc->stSize.u32Width + 63) & ~63) >> 6);
		h = (((pvecc->stSize.u32Height + 63) & ~63) >> 6);

		pvecc->pu8QpMap = (CVI_U8 *) malloc(w * h);
		if (!pvecc->pu8QpMap) {
			CVI_VENC_ERR("malloc pu8QpMap\n");
			return CVI_FAILURE;
		}

	} else if ((pvecc->enPayLoad == PT_H264 || pvecc->enPayLoad == PT_H265) && strlen(pIc->roiCfgFile) > 0) {
		s32Ret = SAMPLE_COMM_VENC_LoadRoiCfgFile(pvecc->vencRoi, pIc->roiCfgFile);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("Venc LoadRoiCfgFile failed for %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

static CVI_S32 _SAMPLE_VENC_SetChnParam(vencChnCtx *pVencChnCtx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_CHN VencChn = pVencChnCtx->VencChn;
	chnInputCfg *pIc = &pVencChnCtx->chnIc;

	VENC_CHN_ATTR_S stChnAttr;

	CVI_VENC_GetChnAttr(VencChn, &stChnAttr);

	if (stChnAttr.stVencAttr.enType == PT_H265) {
		stChnAttr.stRcAttr.stH265Cbr.u32BitRate = pIc->chgBitrate;
		stChnAttr.stRcAttr.stH265Cbr.fr32DstFrameRate = pIc->chgFramerate;
	} else if (stChnAttr.stVencAttr.enType == PT_H264) {
		stChnAttr.stRcAttr.stH264Cbr.u32BitRate = pIc->chgBitrate;
		stChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = pIc->chgFramerate;
	} else if (stChnAttr.stVencAttr.enType == PT_MJPEG) {
		stChnAttr.stRcAttr.stMjpegCbr.u32BitRate = pIc->chgBitrate;
		stChnAttr.stRcAttr.stMjpegCbr.fr32DstFrameRate = pIc->chgFramerate;
	}

	CVI_VENC_SetChnAttr(VencChn, &stChnAttr);

	return s32Ret;
}

static CVI_S32 SAMPLE_VENC_StartGetStream(vencChnCtx *pvecc, CVI_S32 s32ChnIdx)
{
	struct sched_param param;
	pthread_attr_t attr;

	CVI_VENC_TRACE("SAMPLE_VENC_StartGetStream\n");

	gs_VencTask[s32ChnIdx] = 0;

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(
			&gs_VencTask[s32ChnIdx],
			&attr,
			SAMPLE_VENC_GetVencStreamProc,
			(CVI_VOID *)pvecc);

	return CVI_SUCCESS;
}

static CVI_S32 _getNonBindModeSrcFrame(vencChnCtx *pvecc,
		VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_CHN VencChn = pvecc->VencChn;
	chnInputCfg *pIc = &pvecc->chnIc;

	// non-bind mode
	//	+ input_path != 0, VENC-only test, read source file
	//	+ input_path == 0, test with VPSS
	//		* vpssSrcPath == 0, VPSS source comes from sensor.
	//	      Need to get / release frame
	//		* vpssSrcPath != 0, VPSS source comes from file.
	//	      User gets / releases frame
	if (strlen(pIc->input_path) != 0) {
		s32Ret = cviReadSrcFrame(pvecc->pstVFrame, pvecc->fpSrc);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("(chn %d) cviReadSrcFrame fail\n", VencChn);
			return s32Ret;
		}
	} else if (strlen(pIc->vpssSrcPath) == 0) {
		pvecc->pstFrameInfo = pstVideoFrame;
		s32Ret = CVI_VPSS_GetChnFrame(0, VencChn, pvecc->pstFrameInfo, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
			return s32Ret;
		}
	} else {
		pvecc->pstFrameInfo = pstVideoFrame;
	}

	return s32Ret;
}

static CVI_VOID _venc_unbind_source(chnInputCfg *pIc, VENC_CHN VencChn)
{
	if (pIc->bind_mode == VENC_BIND_VI) {
		CVI_U8 VencchnID = VencChn, VichnID = VencChn;

		CVI_VENC_TRACE("SAMPLE_COMM_VI_UnBind_VENC\n");
		SAMPLE_COMM_VI_UnBind_VENC(0, VichnID, VencchnID);
	} else if (pIc->bind_mode == VENC_BIND_VPSS) {
		CVI_VENC_TRACE("SAMPLE_COMM_VPSS_UnBind_VENC\n");
		SAMPLE_COMM_VPSS_UnBind_VENC(pIc->vpssGrp, pIc->vpssChn, VencChn);
	}
}

static CVI_S32 _releaseNonBindModeSrcFrame(vencChnCtx *pvecc)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_CHN VencChn = pvecc->VencChn;
	chnInputCfg *pIc = &pvecc->chnIc;

	// non-bind mode
	//	+ input_path != 0, VENC-only test, read source file
	//	+ input_path == 0, test with VPSS
	//		* vpssSrcPath == 0, VPSS source comes from sensor.
	//	      Need to get / release frame
	//		* vpssSrcPath != 0, VPSS source comes from file.
	//	      User gets / releases frame
	if (strlen(pIc->input_path) == 0) {
		if (strlen(pIc->vpssSrcPath) == 0) {
			s32Ret = CVI_VPSS_ReleaseChnFrame(0, VencChn, pvecc->pstFrameInfo);
			if (s32Ret != CVI_SUCCESS) {
				CVI_VENC_ERR("CVI_VPSS_ReleaseChnFrame fail\n");
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

static CVI_VOID *SAMPLE_VENC_GetVencStreamProc(CVI_VOID *pArgs)
{
	vencChnCtx *pvecc = (vencChnCtx *)pArgs;
	VENC_CHN VencChn = pvecc->VencChn;
	chnInputCfg *pIc = &pvecc->chnIc;
	VIDEO_FRAME_INFO_S stVideoFrame, *pstFrameInfo = &stVideoFrame;
	CVI_CHAR TaskName[64];
	CVI_S32 s32Ret;
	CVI_U32 i;

	sprintf(TaskName, "chn%dVencGetStream", VencChn);
	prctl(PR_SET_NAME, TaskName, 0, 0, 0);

	CVI_VENC_FLOW("venc task%d start\n", VencChn);

	usleep(1000);

	pvecc->chnStat = CHN_STAT_START;
	pvecc->nextChnStat = CHN_STAT_START;
	pvecc->s32VencFd = -1;

	if (pIc->bsMode == BS_MODE_SELECT) {
			pvecc->s32VencFd = CVI_VENC_GetFd(VencChn);
			if (pvecc->s32VencFd < 0) {
					SAMPLE_PRT("CVI_VENC_GetFd failed with%#x!\n", pvecc->s32VencFd);
			}
	}

	i = 0;
	while (pvecc->chnStat == CHN_STAT_START && pvecc->pFile && i < pvecc->num_frames) {
		if (pIc->bind_mode == VENC_BIND_DISABLE) {
			s32Ret = _getNonBindModeSrcFrame(pvecc, pstFrameInfo);
			if (s32Ret != CVI_SUCCESS) {
				CVI_VENC_ERR("(chn %d) _getNonBindModeSrcFrame fail\n", VencChn);
				break;
			}
		}

		s32Ret = _SAMPLE_VENC_SendFrame(pvecc, i);
		if (s32Ret == CVI_ERR_VENC_FRC_NO_ENC) {
			continue;
		} else if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("_SAMPLE_VENC_SendFrame, %d\n", s32Ret);
			break;
		}

		s32Ret = _SAMPLE_VENC_GetStream(pvecc);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("_SAMPLE_VENC_GetStream, %d\n", s32Ret);
			break;
		}

		usleep(100);

		i++;
	}

	CVI_VENC_FLOW("venc task%d end\n", pvecc->VencChn);

	if (pvecc->s32VencFd >= 0) {
		CVI_VENC_CloseFd(VencChn);
	}

	_venc_unbind_source(pIc, VencChn);

	return (CVI_VOID *) CVI_SUCCESS;
}

static CVI_S32 _SAMPLE_VENC_SendFrame(vencChnCtx *pvecc, CVI_U32 i)
{
	chnInputCfg *pIc = &pvecc->chnIc;
	VENC_CHN VencChn = pvecc->VencChn;
	CVI_U32 enableQpMap =
		(pvecc->enPayLoad == PT_H265 && pvecc->enRcMode == SAMPLE_RC_QPMAP);
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_VENC_TRACE("[Chn%d] frame %d (%d)\n", VencChn, i, pvecc->num_frames);

	if (pIc->forceIdr > 0 && pIc->forceIdr == (CVI_S32)i) {
		CVI_BOOL bInstant = pIc->u32ResetGop;

		CVI_VENC_RequestIDR(VencChn, bInstant);
		CVI_VENC_TRACE("CVI_VENC_RequestIDR, resetGop:%d\n", bInstant);
	}

	if (enableQpMap) {
		s32Ret = SAMPLE_COMM_VENC_SetQpMapByCfgFile(VencChn,
				pvecc->vencRoi, i, pvecc->pu8QpMap, &pvecc->bQpMapValid,
				pvecc->stSize.u32Width, pvecc->stSize.u32Height);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("SAMPLE_COMM_VENC_SetQpMapByCfgFile, %d\n", s32Ret);
			return s32Ret;
		}
	} else if ((pvecc->enPayLoad == PT_H264) ||
			(pvecc->enPayLoad == PT_H265 && pvecc->enRcMode != SAMPLE_RC_FIXQP)) {
		s32Ret = SAMPLE_COMM_VENC_SetRoiAttrByCfgFile(VencChn, pvecc->vencRoi, i);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("SAMPLE_COMM_VENC_SetRoiAttrByCfgFile, %d\n", s32Ret);
			return s32Ret;
		}
	}

	if (pIc->bind_mode == VENC_BIND_DISABLE) {
		if ((pIc->chgNum != -1) &&  ((CVI_S32)i == pIc->chgNum)) {
			_SAMPLE_VENC_SetChnParam(pvecc);
			pIc->chgNum = -1;
		}

		pvecc->pstFrameInfo->stVFrame.u64PTS = (CVI_U64) i;

RETRY_SEND_FRAME:
		s32Ret = _SAMPLE_VENC_SendOneFrame(pvecc);
		if (s32Ret == CVI_ERR_VENC_FRC_NO_ENC) {
			CVI_VENC_FRC("no encode\n");
			return s32Ret;
		} else if (s32Ret == CVI_ERR_VENC_BUSY) {
			CVI_VENC_TRACE("send frame timeout ..retry[%d]\n",
					pIc->sendframe_timeout);
			if (pIc->sendframe_timeout == 0) {
				//try once mode
				//user should retry
				CVI_VENC_TRACE("sendframe_timeout==0 failure ..retry\n");
			}
			goto RETRY_SEND_FRAME;
		} else if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_VENC_SendFrame, VencChn = %d, s32Ret = %d\n",
					VencChn, s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

static CVI_S32 _SAMPLE_VENC_GetStream(vencChnCtx *pvecc)
{
	chnInputCfg *pIc = &pvecc->chnIc;
#if (CONFIG_CVI_LOG_TRACE_SUPPORT == 1)
	VENC_CHN VencChn = pvecc->VencChn;
#endif
	CVI_S32 s32Ret;

	if (pIc->bsMode == BS_MODE_SELECT && pvecc->s32VencFd >= 0) {
		struct timeval TimeoutVal;
		fd_set read_fds;

		FD_ZERO(&read_fds);
		FD_SET(pvecc->s32VencFd, &read_fds);
		TimeoutVal.tv_sec  = 10;
		TimeoutVal.tv_usec = 0;
		s32Ret = select(pvecc->s32VencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
		if (s32Ret < 0) {
			SAMPLE_PRT("select failed!\n");
			return s32Ret;
		} else if (s32Ret == 0) {
			SAMPLE_PRT("select time out??\n");
		} else if (!FD_ISSET(pvecc->s32VencFd, &read_fds)) {
			SAMPLE_PRT("select return not set??\n");
		}
	}

	s32Ret = SAMPLE_COMM_VENC_SaveChannelStream(pvecc);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_WARN("SAMPLE_COMM_VENC_SaveChannelStream, VencChn = %d, s32Ret = %d\n",
				VencChn, s32Ret);
		return s32Ret;
	}

	if (pIc->bind_mode == VENC_BIND_DISABLE) {
		s32Ret = _releaseNonBindModeSrcFrame(pvecc);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("_releaseNonBindModeSrcFrame fail\n");
			return s32Ret;
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 cviReadSrcFrame(VIDEO_FRAME_S *pstVFrame, FILE *fp)
{
	size_t read_byte;
	CVI_U8 *frm_ptr;
	CVI_U32 j;
	CVI_U32 u32CbCrReadSrcHeight;
	CVI_U32 bCbWidthShift, bCrWidthShift;
	CVI_U32 to_read; // bytes

	CVI_VENC_TRACE("\n");

	switch (pstVFrame->enPixelFormat) {
	case PIXEL_FORMAT_YUV_PLANAR_422:
		u32CbCrReadSrcHeight = pstVFrame->u32Height;
		bCbWidthShift = 1;
		bCrWidthShift = 1;
		break;
	case PIXEL_FORMAT_NV12:
	case PIXEL_FORMAT_NV21:
		u32CbCrReadSrcHeight = pstVFrame->u32Height >> 1;
		bCbWidthShift = 0;
		bCrWidthShift = 31;
		break;
	case PIXEL_FORMAT_YUV_PLANAR_420:
	default:
		u32CbCrReadSrcHeight = pstVFrame->u32Height >> 1;
		bCbWidthShift = 1;
		bCrWidthShift = 1;
		break;
	}

	if (pstVFrame->u32Width == pstVFrame->u32Stride[0]) {
		// Luma
		frm_ptr = pstVFrame->pu8VirAddr[0];
		to_read = pstVFrame->u32Width * pstVFrame->u32Height;
		read_byte = fread((void *)frm_ptr, 1, to_read, fp);
		if (read_byte != to_read) {
			CVI_VENC_ERR("Luma, fread %zu %d failed\n", read_byte, to_read);
			return CVI_FAILURE;
		}

		// Cb
		frm_ptr = pstVFrame->pu8VirAddr[1];
		to_read = (pstVFrame->u32Width * u32CbCrReadSrcHeight) >> bCbWidthShift;
		read_byte = fread((void *)frm_ptr, 1, to_read, fp);
		if (read_byte != to_read) {
			CVI_VENC_ERR("Cb, fread %zu %d failed\n", read_byte, to_read);
			return CVI_FAILURE;
		}

		// Cr
		frm_ptr = pstVFrame->pu8VirAddr[2];
		to_read = (pstVFrame->u32Width * u32CbCrReadSrcHeight) >> bCrWidthShift;
		read_byte = fread((void *)frm_ptr, 1, to_read, fp);
		if (read_byte != to_read) {
			CVI_VENC_ERR("Cr, fread %zu %d failed\n", read_byte, to_read);
			return CVI_FAILURE;
		}
	} else {
		// Luma
		for (j = 0; j < pstVFrame->u32Height; j++) {
			frm_ptr = pstVFrame->pu8VirAddr[0] + j * pstVFrame->u32Stride[0];
			to_read = pstVFrame->u32Width;
			read_byte = fread((void *)frm_ptr, 1, to_read, fp);
			if (read_byte != to_read) {
				CVI_VENC_ERR("Luma, (row %d) fread %zu %d failed\n",
					j, read_byte, to_read);
				return CVI_FAILURE;
			}
		}

		// Cb
		for (j = 0; j < u32CbCrReadSrcHeight; j++) {
			frm_ptr = pstVFrame->pu8VirAddr[1] + j * (pstVFrame->u32Stride[0] >> bCbWidthShift);
			to_read = pstVFrame->u32Width >> bCbWidthShift;
			read_byte = fread((void *)frm_ptr, 1, to_read, fp);
			if (read_byte != to_read) {
				CVI_VENC_ERR("Cb, (row %d) fread %zu %d failed\n",
					j, read_byte, to_read);
				return CVI_FAILURE;
			}
		}

		// Cr
		for (j = 0; j < u32CbCrReadSrcHeight; j++) {
			frm_ptr = pstVFrame->pu8VirAddr[2] + j * (pstVFrame->u32Stride[0] >> bCrWidthShift);
			to_read = pstVFrame->u32Width >> bCrWidthShift;
			read_byte = fread((void *)frm_ptr, 1, to_read, fp);
			if (read_byte != to_read) {
				CVI_VENC_ERR("Cr, (row %d) fread %zu %d failed\n",
					j, read_byte, to_read);
				return CVI_FAILURE;
			}
		}
	}

	return CVI_SUCCESS;
}

static CVI_VOID exitSysAndVb(CVI_VOID)
{
	CVI_SYS_Exit();
	CVI_VB_Exit();
}

static VIDEO_FRAME_INFO_S *allocate_frame(SIZE_S stSize, PIXEL_FORMAT_E enPixelFormat)
{
	VIDEO_FRAME_INFO_S *pstVideoFrame;
	VIDEO_FRAME_S *pstVFrame;
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCfg;

	pstVideoFrame = (VIDEO_FRAME_INFO_S *)calloc(sizeof(*pstVideoFrame), 1);
	if (pstVideoFrame == NULL) {
		CVI_VENC_ERR("Failed to allocate VIDEO_FRAME_INFO_S\n");
		return NULL;
	}

	memset(&stVbCfg, 0, sizeof(stVbCfg));
	VENC_GetPicBufferConfig(stSize.u32Width,
				stSize.u32Height,
				enPixelFormat,
				DATA_BITWIDTH_8,
				COMPRESS_MODE_NONE,
				&stVbCfg);

	pstVFrame = &pstVideoFrame->stVFrame;

	pstVFrame->enCompressMode = COMPRESS_MODE_NONE;
	pstVFrame->enPixelFormat = enPixelFormat;
	pstVFrame->enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVFrame->enColorGamut = COLOR_GAMUT_BT709;
	pstVFrame->u32Width = stSize.u32Width;
	pstVFrame->u32Height = stSize.u32Height;
	pstVFrame->u32TimeRef = 0;
	pstVFrame->u64PTS = 0;
	pstVFrame->enDynamicRange = DYNAMIC_RANGE_SDR8;

	if (pstVFrame->u32Width % VENC_ALIGN_W) {
		CVI_VENC_INFO("u32Width is not algined to %d\n", VENC_ALIGN_W);
	}

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCfg.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		CVI_VENC_ERR("Can't acquire vb block\n");
		free(pstVideoFrame);
		return NULL;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
	pstVFrame->u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	pstVFrame->u32Stride[0] = stVbCfg.u32MainStride;
	pstVFrame->u32Length[0] = stVbCfg.u32MainYSize;
	pstVFrame->pu8VirAddr[0] = (CVI_U8 *)CVI_SYS_Mmap(pstVFrame->u64PhyAddr[0], pstVFrame->u32Length[0]);
	memset(pstVFrame->pu8VirAddr[0], 0, pstVFrame->u32Length[0]);

	if (stVbCfg.plane_num > 1) {
		pstVFrame->u64PhyAddr[1] = ALIGN(pstVFrame->u64PhyAddr[0] + stVbCfg.u32MainYSize, stVbCfg.u16AddrAlign);
		pstVFrame->u32Stride[1] = stVbCfg.u32CStride;
		pstVFrame->u32Length[1] = stVbCfg.u32MainCSize;
		pstVFrame->pu8VirAddr[1] = (CVI_U8 *)CVI_SYS_Mmap(pstVFrame->u64PhyAddr[1], pstVFrame->u32Length[1]);
		memset(pstVFrame->pu8VirAddr[1], 0, pstVFrame->u32Length[1]);
	}

	if (stVbCfg.plane_num > 2) {
		pstVFrame->u64PhyAddr[2] = ALIGN(pstVFrame->u64PhyAddr[1] + stVbCfg.u32MainCSize, stVbCfg.u16AddrAlign);
		pstVFrame->u32Stride[2] = stVbCfg.u32CStride;
		pstVFrame->u32Length[2] = stVbCfg.u32MainCSize;
		pstVFrame->pu8VirAddr[2] = (CVI_U8 *)CVI_SYS_Mmap(pstVFrame->u64PhyAddr[2], pstVFrame->u32Length[2]);
		memset(pstVFrame->pu8VirAddr[2], 0, pstVFrame->u32Length[2]);
	}

	CVI_VENC_TRACE("phy addr(%#llx, %#llx, %#llx), Size %x\n", (long long)pstVFrame->u64PhyAddr[0]
		, (long long)pstVFrame->u64PhyAddr[1], (long long)pstVFrame->u64PhyAddr[2], stVbCfg.u32VBSize);
	CVI_VENC_TRACE("vir addr(%p, %p, %p), Size %x\n", pstVFrame->pu8VirAddr[0]
		, pstVFrame->pu8VirAddr[1], pstVFrame->pu8VirAddr[2], stVbCfg.u32MainSize);

	return pstVideoFrame;
}

static CVI_S32 free_frame(VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VIDEO_FRAME_S *pstVFrame = &pstVideoFrame->stVFrame;
	VB_BLK blk;

	if (pstVFrame->pu8VirAddr[0])
		CVI_SYS_Munmap((CVI_VOID *)pstVFrame->pu8VirAddr[0], pstVFrame->u32Length[0]);
	if (pstVFrame->pu8VirAddr[1])
		CVI_SYS_Munmap((CVI_VOID *)pstVFrame->pu8VirAddr[1], pstVFrame->u32Length[1]);
	if (pstVFrame->pu8VirAddr[2])
		CVI_SYS_Munmap((CVI_VOID *)pstVFrame->pu8VirAddr[2], pstVFrame->u32Length[2]);

	blk = CVI_VB_PhysAddr2Handle(pstVFrame->u64PhyAddr[0]);
	if (blk != VB_INVALID_HANDLE) {
		CVI_VB_ReleaseBlock(blk);
	}

	free(pstVideoFrame);

	return CVI_SUCCESS;
}

static CVI_S32 _SAMPLE_VENC_SendOneFrame(vencChnCtx *pvecc)
{
	VENC_CHN VencChn = pvecc->VencChn;
	chnInputCfg *pIc = &pvecc->chnIc;
	CVI_U32 enableQpMap =
		(pvecc->enPayLoad == PT_H265 && pvecc->enRcMode == SAMPLE_RC_QPMAP);
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pIc->rcMode == SAMPLE_RC_UBR) {
		s32Ret = SAMPLE_COMM_VENC_SetUserFrameLevelRc(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("(chn %d) SAMPLE_COMM_VENC_SetUserFrameLevelRc\n", VencChn);
			return s32Ret;
		}
	}

	SAMPLE_VENC_InsertUserData(VencChn, pIc);

	if (enableQpMap) {
		USER_FRAME_INFO_S userFrameInfo;

		userFrameInfo.stUserFrame = *pvecc->pstFrameInfo;
		userFrameInfo.stUserRcInfo.bQpMapValid = pvecc->bQpMapValid;
		userFrameInfo.stUserRcInfo.u64QpMapPhyAddr =
			(uintptr_t) pvecc->pu8QpMap;

		CVI_VENC_TRACE("u64QpMapPhyAddr = %"PRIx64"\n",
				userFrameInfo.stUserRcInfo.u64QpMapPhyAddr);
		CVI_VENC_TRACE("pu8QpMap = %p\n", pvecc->pu8QpMap);

		s32Ret = API_COST_TIME_LOG(CVI_VENC_SendFrameEx(VencChn, &userFrameInfo,
			pIc->sendframe_timeout), "CVI_VENC_SendFrameEx");
	} else {
		s32Ret = API_COST_TIME_LOG(CVI_VENC_SendFrame(VencChn, pvecc->pstFrameInfo,
			pIc->sendframe_timeout), "CVI_VENC_SendFrame");
	}

	return s32Ret;
}

static CVI_S32 SAMPLE_COMM_VENC_SetUserFrameLevelRc(chnInputCfg *pIc, VENC_CHN VencChn)
{
	VENC_FRAME_PARAM_S stFrameParam, *pstFrameParam = &stFrameParam;
	CVI_S32 s32Ret = CVI_SUCCESS;
	s32Ret = API_COST_TIME_LOG(CVI_VENC_GetFrameParam(VencChn, pstFrameParam), "CVI_VENC_GetFrameParam");
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_GetFrameParam\n");
		return s32Ret;
	}

	if (pIc->bTestUbrEn == 1) {
		// use CVI CBR
		s32Ret = API_COST_TIME_LOG(CVI_VENC_CalcFrameParam(VencChn, pstFrameParam), "CVI_VENC_CalcFrameParam");
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_VENC_CalcFrameParam\n");
			return s32Ret;
		}
	} else {
		// use User-defined RC
		pstFrameParam->u32FrameQp = pIc->u32FrameQp;
		pstFrameParam->u32FrameBits = pIc->bitrate * 1000 / pIc->framerate;
	}

	CVI_VENC_TRACE("u32FrameQp = %d, u32FrameBits = %d\n",
			pstFrameParam->u32FrameQp, pstFrameParam->u32FrameBits);
	s32Ret = API_COST_TIME_LOG(CVI_VENC_SetFrameParam(VencChn, pstFrameParam), "CVI_VENC_SetFrameParam");
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_SetFrameParam fail\n");
		return s32Ret;
	}

	return s32Ret;
}

static CVI_VOID SAMPLE_VENC_InsertUserData(VENC_CHN chn, chnInputCfg *pIc)
{
	CVI_U8 buf[65536] = { 0 };
	FILE *fp;
	int i, len;

	for (i = 0; i < NUM_OF_USER_DATA_BUF; ++i) {
		if (strlen(pIc->user_data[i]) == 0)
			continue;

		fp = fopen(pIc->user_data[i], "rb");
		if (!fp)
			continue;

		do {
			fseek(fp, 0, SEEK_END);
			len = ftell(fp);
			if (len <= 0 || len > 65536)
				break;

			fseek(fp, 0, SEEK_SET);
			fread(buf, 1, len, fp);
			CVI_VENC_InsertUserData(chn, buf, len);
		} while (0);

		fclose(fp);
	}
}
#if defined(__GNUC__) && defined(__riscv)
#pragma GCC diagnostic pop
#endif
