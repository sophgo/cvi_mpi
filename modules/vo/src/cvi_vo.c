#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/param.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include <math.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cvi_sys.h"
#include <cvi_comm_vo.h>
#include "cvi_buffer.h"

#include "cvi_sys_base.h"
#include "cvi_vo.h"
#include "cvi_gdc.h"
#include "cvi_vb.h"
#include "gdc_mesh.h"
#include <vo_uapi.h>
#include <cvi_vo_ctx.h>
#include "cvi_msg.h"
#include "msg_client.h"

#define CHECK_VO_DEV_VALID(VoDev) do {									\
		if ((VoDev >= VO_MAX_DEV_NUM) || (VoDev < 0)) {						\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) invalid.\n", VoDev);			\
			return CVI_ERR_VO_INVALID_DEVID;						\
		}											\
	} while (0)

#define CHECK_VO_LAYER_VALID(VoLayer) do {								\
		if ((VoLayer >= VO_MAX_LAYER_NUM) || (VoLayer < 0)) {					\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) invalid.\n", VoLayer);			\
			return CVI_ERR_VO_INVALID_LAYERID;						\
		}											\
	} while (0)

#define CHECK_VO_CHN_VALID(VoLayer, VoChn) do {							\
		if ((VoLayer >= VO_MAX_LAYER_NUM) || (VoLayer < 0)) {					\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) invalid.\n", VoLayer);			\
			return CVI_ERR_VO_INVALID_LAYERID;						\
		}											\
		if ((VoChn >= VO_MAX_CHN_NUM) || (VoChn < 0)) {						\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoChn(%d) invalid.\n", VoChn);			\
			return CVI_ERR_VO_INVALID_CHNID;						\
		}											\
	} while (0)

struct vo_pm_s {
	VO_PM_OPS_S	stOps;
	CVI_VOID	*pvData;
};
static struct vo_pm_s apstVoPm[VO_MAX_DEV_NUM] = { 0 };

enum i80_op_type {
	I80_OP_GO = 0,
	I80_OP_TIMER,
	I80_OP_DONE,
	I80_OP_MAX,
};

enum i80_ctrl_type {
	I80_CTRL_CMD = 0,
	I80_CTRL_DATA,
	I80_CTRL_EOL = I80_CTRL_DATA,
	I80_CTRL_EOF,
	I80_CTRL_END = I80_CTRL_EOF,
	I80_CTRL_MAX
};

CVI_U8 i80_ctrl[I80_CTRL_MAX] = { 0x31, 0x75, 0xff };

static CVI_S32 _check_vo_exist(CVI_VOID)
{
#if defined(__CV180X__)
	CVI_TRACE_VO(CVI_DBG_ERR, "No vo dev!\n");
	return CVI_FAILURE;
#endif
	return CVI_SUCCESS;
}

static CVI_S32 _vo_get_ctx(struct cvi_vo_ctx *pVoCtx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, 0, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GET_CTX, pVoCtx, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "MSG_CMD_VO_GET_CTX fail,s32Ret:%x\n", s32Ret);
	}

	return s32Ret;
}

/**************************************************************************
 *   Bin related APIs.
 **************************************************************************/
#define VO_BIN_GUARDMAGIC 0x12345678
static VO_BIN_INFO_S vo_bin_info = {
	.gamma_info = {
		.enable = CVI_FALSE,
		.osd_apply = CVI_FALSE,
		.value = {
			0,   3,   7,   11,  15,  19,  23,  27,
			31,  35,  39,  43,  47,  51,  55,  59,
			63,  67,  71,  75,  79,  83,  87,  91,
			95,  99,  103, 107, 111, 115, 119, 123,
			127, 131, 135, 139, 143, 147, 151, 155,
			159, 163, 167, 171, 175, 179, 183, 187,
			191, 195, 199, 203, 207, 211, 215, 219,
			223, 227, 231, 235, 239, 243, 247, 251,
			255
		}
	},
	.guard_magic = VO_BIN_GUARDMAGIC
};

VO_BIN_INFO_S *get_vo_bin_info_addr(void)
{
	return &vo_bin_info;
}

CVI_U32 get_vo_bin_guardmagic_code(void)
{
	return VO_BIN_GUARDMAGIC;
}
#if 0 // skip in FPGA test
static CVI_U32 _getFileSize(FILE *fp, CVI_U64 *size)
{
	CVI_S32 ret = CVI_SUCCESS;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, fp);
	MOD_CHECK_NULL_PTR(CVI_ID_VO, size);

	fseek(fp, 0L, SEEK_END);
	*size = ftell(fp);
	rewind(fp);

	return ret;
}

static CVI_S32 CVI_VO_SetVOParamFromBin(void)
{
	CVI_S32 ret = CVI_SUCCESS;
	FILE *fp = NULL;
	CVI_U8 *buf;
	CVI_CHAR binName[BIN_FILE_LENGTH];
	CVI_U64 file_size;

	ret = CVI_BIN_GetBinName(binName);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "GetBinName failure\n");
	}

	fp = fopen(binName, "rb");
	if (fp == NULL) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Cant find bin(%s)\n", binName);
		return CVI_FAILURE;
	}
	CVI_TRACE_VO(CVI_DBG_DEBUG, "Bin exist (%s)\n", binName);

	_getFileSize(fp, &file_size);

	buf = (CVI_U8 *)malloc(file_size);
	if (buf == NULL) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Allocae memory failed!\n");
		fclose(fp);
		return CVI_FAILURE;
	}

	//fread info buffer and calling CVI_BIN
	fread(buf, file_size, 1, fp);
	ret = CVI_BIN_LoadParamFromBin(CVI_BIN_ID_VO, buf);
	free(buf);

	{
		struct vdev *d;
		//set gamma with HW from bin or default value
		d = get_dev_info(VDEV_TYPE_DISP, 0);
		vo_set_gamma_ctrl(d->fd, &vo_bin_info.gamma_info);
	}
	return CVI_SUCCESS;
}
#endif

CVI_S32 CVI_VO_Suspend(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, 0, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SUSPEND, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VPSS_VO fail\n");
		return s32Ret;
    }

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_Resume(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, 0, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_RESUME, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_Resume fail\n");
		return s32Ret;
    }

	return CVI_SUCCESS;
}
/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 CVI_VO_SetPubAttr(VO_DEV VoDev, const VO_PUB_ATTR_S *pstPubAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstPubAttr);
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SET_PUBATTR, (CVI_VOID *)pstPubAttr, sizeof(*pstPubAttr), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_SetPubAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetPubAttr(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstPubAttr);
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GET_PUBATTR, pstPubAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetPubAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ShowPattern(VO_DEV VoDev, enum VO_PATTERN_MODE PatternId)
{
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = PatternId;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SHOW_PATTERN, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "ShowPattern fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_Enable(VO_DEV VoDev)
{
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_ENABLE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_Enable fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_Disable(VO_DEV VoDev)
{
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_DISABLE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_Disable fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetVideoLayerCSC(VO_LAYER VoLayer, const VO_CSC_S *pstVideoCSC)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoCSC);
	CHECK_VO_LAYER_VALID(VoLayer);

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetVideoLayerCSC(VO_LAYER VoLayer, VO_CSC_S *pstVideoCSC)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoCSC);
	CHECK_VO_LAYER_VALID(VoLayer);

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_EnableVideoLayer(VO_LAYER VoLayer)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_ENABLE_VIDEOLAYER, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_EnableVideoLayer fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_DisableVideoLayer(VO_LAYER VoLayer)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_DISABLE_VIDEOLAYER, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_DisableVideoLayer fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetVideoLayerAttr(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstLayerAttr);
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SET_VIDEOLAYERATTR, (CVI_VOID *)pstLayerAttr, sizeof(*pstLayerAttr), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_SetVideoLayerAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetVideoLayerAttr(VO_LAYER VoLayer, VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstLayerAttr);
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GET_VIDEOLAYERATTR, pstLayerAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_GetVideoLayerAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetLayerProcAmpCtrl(VO_LAYER VoLayer, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, ctrl);
	CHECK_VO_LAYER_VALID(VoLayer);
	struct cvi_vo_ctx voCtx;

	if (_vo_get_ctx(&voCtx) != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "_vo_get_ctx failed");
		return CVI_FAILURE;
	}

	if (!IS_FMT_YUV(voCtx.stLayerAttr.enPixFormat)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}
	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) ProcAmp type(%d) invalid.\n", VoLayer, type);
		return CVI_ERR_VO_ILLEGAL_PARAM;
	}

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	static PROC_AMP_CTRL_S ctrls[PROC_AMP_MAX] = {
		{ .minimum = 0, .maximum = 255, .step = 1, .default_value = 128 },
		{ .minimum = 0, .maximum = 255, .step = 1, .default_value = 128 },
		{ .minimum = 0, .maximum = 255, .step = 1, .default_value = 128 },
		{ .minimum = 0, .maximum = 359, .step = 1, .default_value = 0 },
	};

	*ctrl = ctrls[type];
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 *value)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, value);
	CHECK_VO_LAYER_VALID(VoLayer);
	struct cvi_vo_ctx voCtx;

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	if (_vo_get_ctx(&voCtx) != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "_vo_get_ctx failed");
		return CVI_FAILURE;
	}

	if (!IS_FMT_YUV(voCtx.stLayerAttr.enPixFormat)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Only YUV format support.\n", VoLayer);
		return CVI_ERR_VO_NOT_SUPPORT;
	}
	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) ProcAmp type(%d) invalid.\n", VoLayer, type);
		return CVI_ERR_VO_ILLEGAL_PARAM;
	}

	*value = voCtx.proc_amp[type];
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 value)
{
	UNUSED(VoLayer);
	UNUSED(type);
	UNUSED(value);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_ATTR_S *pstChnAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstChnAttr);
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SET_CHNATTR, (CVI_VOID *)pstChnAttr, sizeof(*pstChnAttr), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_SetChnAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_ATTR_S *pstChnAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstChnAttr);
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GET_CHNATTR, pstChnAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_GetChnAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetScreenFrame(VO_LAYER VoLayer, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoFrame);
	CHECK_VO_LAYER_VALID(VoLayer);

	UNUSED(s32MilliSec);
	return CVI_ERR_VO_NOT_SUPPORT;
}

CVI_S32 CVI_VO_ReleaseScreenFrame(VO_LAYER VoLayer, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoFrame);
	CHECK_VO_LAYER_VALID(VoLayer);
	return CVI_ERR_VO_NOT_SUPPORT;
}

CVI_S32 CVI_VO_SetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 u32BufLen)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);
	MSG_PRIV_DATA_S stPrivData;

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = u32BufLen;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SET_DISPLAYBUFLEN, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_SetDisplayBufLen fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 *pu32BufLen)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GET_DISPLAYBUFLEN, pu32BufLen, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_GetDisplayBufLen fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_EnableChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_ENABLE_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_EnableChn fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_DisableChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_DISABLE_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_DisableChn fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E enRotation)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = enRotation;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SET_CHNROTATION, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_SetChnRotation fail ret:%d\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E *penRotation)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GET_CHNROTATION, penRotation, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetChnRotation fail ret:%d\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SendFrame(VO_LAYER VoLayer, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoFrame);
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);
	MSG_PRIV_DATA_S stPrivData;

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = s32MilliSec;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SEND_FRAME, (CVI_VOID *)pstVideoFrame, sizeof(*pstVideoFrame), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_SendFrame fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ClearChnBuf(VO_LAYER VoLayer, VO_CHN VoChn, CVI_BOOL bClrAll)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = bClrAll;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_CLEAR_CHNBUF, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_ClearChnBuf fail ret:%d\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ShowChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SHOW_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_ShowChn fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_HideChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_HIDE_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_HideChn fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_CloseFd(void)
{
	CVI_TRACE_VO(CVI_DBG_ERR, "dual os not support close vo fd in linux.\n");
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_PauseChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_PAUSE_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_PauseChn fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ResumeChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_RESUME_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_ResumeChn fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_Get_Panel_Status(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U32 *is_init)
{
	UNUSED(VoLayer);
	UNUSED(VoChn);
	UNUSED(is_init);
	return CVI_FAILURE;
}

CVI_S32 CVI_VO_RegPmCallBack(VO_DEV VoDev, VO_PM_OPS_S *pstPmOps, void *pvData)
{
	CHECK_VO_DEV_VALID(VoDev);
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstPmOps);

	apstVoPm[VoDev].stOps = *pstPmOps;
	apstVoPm[VoDev].pvData = pvData;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_UnRegPmCallBack(VO_DEV VoDev)
{
	CHECK_VO_DEV_VALID(VoDev);

	memset(&apstVoPm[VoDev].stOps, 0, sizeof(apstVoPm[VoDev].stOps));
	apstVoPm[VoDev].pvData = NULL;
	return CVI_SUCCESS;
}

CVI_BOOL CVI_VO_IsEnabled(VO_DEV VoDev)
{
#if defined(__CV180X__)
	CVI_TRACE_VO(CVI_DBG_ERR, "No vo dev!\n");
	return false;
#endif
	CVI_S32 s32Ret;
	CVI_BOOL bIsEnable;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if ((VoDev >= VO_MAX_DEV_NUM) || (VoDev < 0)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) invalid.\n", VoDev);
		return CVI_FALSE;
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_DEV_IS_ENABLE, &bIsEnable, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_IsEnabled fail,s32Ret:%x\n", s32Ret);
		return CVI_FALSE;
	}

	return bIsEnable;
}

CVI_S32 CVI_VO_SetGammaInfo(VO_GAMMA_INFO_S *pinfo)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pinfo);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, 0, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GAMMA_LUT_UPDATE, (CVI_VOID *)pinfo, sizeof(*pinfo), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_SetGammaInfo fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetGammaInfo(VO_GAMMA_INFO_S *pinfo)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pinfo);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, 0, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GAMMA_LUT_READ, pinfo, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VO_GetGammaInfo fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}
