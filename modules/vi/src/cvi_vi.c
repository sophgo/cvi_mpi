#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <inttypes.h>

#include "cvi_buffer.h"
#include "cvi_sys_base.h"
#include "cvi_vi.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_gdc.h"
#include <vi_uapi.h>
#include "gdc_mesh.h"
#include "cvi_sns_ctrl.h"
#include "dump_register.h"
#include <cvi_vi_ctx.h>
#include "cvi_msg.h"
#include "msg_client.h"
#include <vi_isp.h>

#define CHECK_VI_PIPEID_VALID(x)						\
	do {									\
		if ((x) > (VI_MAX_PIPE_NUM - 1)) {				\
			CVI_TRACE_VI(CVI_DBG_ERR, " invalid pipe-id(%d)\n", x);	\
			return CVI_ERR_VI_INVALID_PIPEID;			\
		}								\
	} while (0)

#define CHECK_VI_DEVID_VALID(x)							\
	do {									\
		if ((x) > (VI_MAX_DEV_NUM - 1)) {				\
			CVI_TRACE_VI(CVI_DBG_ERR, " invalid dev-id(%d)\n", x);	\
			return CVI_ERR_VI_INVALID_DEVID;			\
		}								\
	} while (0)

#define CHECK_VI_CHNID_VALID(x)							\
	do {									\
		if ((x) > (VI_MAX_CHN_NUM - 1)) {				\
			CVI_TRACE_VI(CVI_DBG_ERR, " invalid chn-id(%d)\n", x);	\
			return CVI_ERR_VI_INVALID_CHNID;			\
		}								\
	} while (0)

#define CHECK_VI_NULL_PTR(ptr)							\
	do {									\
		if (ptr == NULL) {						\
			CVI_TRACE_VI(CVI_DBG_ERR, " Invalid null pointer\n");	\
			return CVI_ERR_VI_INVALID_NULL_PTR;			\
		}								\
	} while (0)

#define VIPIPE_TO_DEV(_pipe_id, _dev_id)					\
	do {									\
		if (_pipe_id == 0 || _pipe_id == 1)				\
			_dev_id = 0;						\
		else if (_pipe_id == 2 || _pipe_id == 3)			\
			_dev_id = 1;						\
	} while (0)

#define CHECK_VI_EXTCHNID_VALID(x)										\
	do {													\
		if (((x) < VI_EXT_CHN_START) || ((x) >= (VI_EXT_CHN_START + VI_MAX_EXT_CHN_NUM))) {		\
			CVI_TRACE_VI(CVI_DBG_ERR, " invalid extchn-id(%d)\n", x);				\
			return CVI_ERR_VI_INVALID_CHNID;							\
		}												\
	} while (0)

#define CHECK_VI_GDC_FMT(x)											\
	do {													\
		if (!GDC_SUPPORT_FMT(x)) {			\
			CVI_TRACE_VI(CVI_DBG_ERR, "invalid PixFormat(%d) for gdc.\n", (x));			\
			return CVI_ERR_VI_INVALID_PARA;								\
		}												\
	} while (0)

struct cvi_gdc_mesh g_vi_mesh[VI_MAX_CHN_NUM];

struct vi_pm_s {
	VI_PM_OPS_S	stOps;
	CVI_VOID	*pvData;
};
static struct vi_pm_s apstViPm[VI_MAX_DEV_NUM] = { 0 };

static inline CVI_S32 CHECK_VI_CTX_NULL_PTR(void *ptr)
{
	CVI_S32 ret = CVI_SUCCESS;

	if (ptr == NULL) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Call SetDevAttr first\n");
		ret = CVI_ERR_VI_FAILED_NOTCONFIG;
	}

	return ret;
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 CVI_VI_SetDevNum(CVI_U32 devNum)
{
	UNUSED(devNum);

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetDevNum(CVI_U32 *devNum)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, 0, 0);

	CHECK_VI_NULL_PTR(devNum);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_DEV_NUM, (CVI_VOID *)devNum,
				sizeof(CVI_U32), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI_GetDevNum fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_EnablePatt(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS, bStatus = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	CHECK_VI_PIPEID_VALID(ViPipe);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_ENABLE_PATTERN, (CVI_VOID *)&bStatus,
				sizeof(bStatus), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI_EnablePatt fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return bStatus;
}

CVI_S32 CVI_VI_QueryDevStatus(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS, bStatus = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	CHECK_VI_PIPEID_VALID(ViPipe);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_QUERY_DEV_STATUS, (CVI_VOID *)&bStatus,
				sizeof(bStatus), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI_QueryDevStatus fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return bStatus;
}

CVI_S32 CVI_VI_SetDevAttr(VI_DEV ViDev, const VI_DEV_ATTR_S *pstDevAttr)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevAttr);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_DEV_ATTR, (CVI_VOID *)pstDevAttr,
				sizeof(VI_DEV_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "set dev attr fail, ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetDevAttr(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevAttr);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_DEV_ATTR, pstDevAttr, sizeof(VI_DEV_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get dev attr fail, ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_EnableDev(VI_DEV ViDev)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CHECK_VI_DEVID_VALID(ViDev);
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_ENABLE_DEV, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "enable dev fail,ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_DisableDev(VI_DEV ViDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_DEVID_VALID(ViDev);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_DISABLE_DEV, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "diable dev fail,ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetDevTimingAttr(VI_DEV ViDev, const VI_DEV_TIMING_ATTR_S *pstTimingAttr)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	VI_DEV_TIMING_ATTR_S stTimingAttr;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstTimingAttr);

	stTimingAttr = *pstTimingAttr;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_DEV_TIMING_ATTR, (CVI_VOID *)&stTimingAttr,
				sizeof(VI_DEV_TIMING_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "set dev timing attr fail, ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetDevTimingAttr(VI_DEV ViDev, VI_DEV_TIMING_ATTR_S *pstTimingAttr)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstTimingAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_DEV_TIMING_ATTR, (CVI_VOID *)pstTimingAttr,
				sizeof(VI_DEV_TIMING_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get dev timing attr fail, ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/* 2 for vi pipe */
CVI_S32 CVI_VI_CreatePipe(VI_PIPE ViPipe, const VI_PIPE_ATTR_S *pstPipeAttr)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstPipeAttr);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_CREATE_PIPE, (CVI_VOID *)pstPipeAttr,
				sizeof(VI_PIPE_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "create pipe fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_DestroyPipe(VI_PIPE ViPipe)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CHECK_VI_PIPEID_VALID(ViPipe);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_DESTROY_PIPE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "destroy pipe fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_StartPipe(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_START_PIPE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "start pipe fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_VI_StopPipe(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CHECK_VI_PIPEID_VALID(ViPipe);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_STOP_PIPE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "stop pipe fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetPipeAttr(VI_PIPE ViPipe, const VI_PIPE_ATTR_S *pstPipeAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstPipeAttr);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_PIPE_ATTR, (CVI_VOID *)pstPipeAttr,
				sizeof(VI_PIPE_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Set pipe attr fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetPipeAttr(VI_PIPE ViPipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstPipeAttr);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_PIPE_ATTR, pstPipeAttr, sizeof(VI_PIPE_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get pipe attr fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetPipeDumpAttr(VI_PIPE ViPipe, const VI_DUMP_ATTR_S *pstDumpAttr)
{
	CVI_S32 s32Ret;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstDumpAttr);
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_PIPE_DUMP_ATTR, (CVI_VOID *)pstDumpAttr,
				sizeof(VI_DUMP_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Set pipe dump attr fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

CVI_S32 CVI_VI_GetPipeDumpAttr(VI_PIPE ViPipe, VI_DUMP_ATTR_S *pstDumpAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstDumpAttr);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_PIPE_DUMP_ATTR, pstDumpAttr, sizeof(VI_DUMP_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get pipe dump attr fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

// get bayer from preraw
CVI_S32 CVI_VI_GetPipeFrame(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	CHECK_VI_PIPEID_VALID(ViPipe);
	MOD_CHECK_NULL_PTR(CVI_ID_VI, pstFrameInfo);

	stPrivData.as32PrivData[0] = s32MilliSec;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_PIPE_FRAME, (CVI_VOID *)pstFrameInfo,
				sizeof(VIDEO_FRAME_INFO_S) * 2, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get VI Pipe frame fail,VIpipe:%d, s32Ret:%x\n",
			ViPipe, s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_VI_ReleasePipeFrame(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	CHECK_VI_PIPEID_VALID(ViPipe);
	MOD_CHECK_NULL_PTR(CVI_ID_VI, pstFrameInfo);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_RELEASE_PIPE_FRAME, (CVI_VOID *)pstFrameInfo,
				sizeof(VIDEO_FRAME_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Release VI Pipe frame fail,VIpipe:%d, s32Ret:%x\n",
			ViPipe, s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_VI_StartSmoothRawDump(const VI_SMOOTH_RAW_DUMP_INFO_S *pstDumpInfo)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, 0, 0, 1);
	CVI_U64 pAddr = 0;
	CVI_VOID *vir_addr = NULL;
	MSG_PRIV_DATA_S stPrivData;

	CHECK_VI_NULL_PTR(pstDumpInfo);

	s32Ret = CVI_SYS_IonAlloc_Cached(&pAddr, NULL, "SMOOTH_DUMP", pstDumpInfo->phy_size);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI ion alloc size(%u) failed.\n", pstDumpInfo->phy_size);
		return CVI_ERR_VI_NOMEM;
	}

	vir_addr = CVI_SYS_Mmap(pAddr, pstDumpInfo->phy_size);

	memcpy(vir_addr, pstDumpInfo->phy_addr_list, pstDumpInfo->phy_size);

	CVI_SYS_IonFlushCache(pAddr, vir_addr, pstDumpInfo->phy_size);

	stPrivData.as32PrivData[0] = pAddr & 0xFFFFFFFF;
	stPrivData.as32PrivData[1] = (pAddr >> 32) & 0xFFFFFFFF;
	stPrivData.as32PrivData[2] = pstDumpInfo->phy_size;

	CVI_TRACE_VI(CVI_DBG_INFO, "phyaddr high[%x] low[%x] size[%u]\n",
			stPrivData.as32PrivData[1], stPrivData.as32PrivData[0], pstDumpInfo->phy_size);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_START_SMOOTH_RAWDUMP, (CVI_VOID *)pstDumpInfo,
				sizeof(VI_SMOOTH_RAW_DUMP_INFO_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "StartSmoothRawDump fail, s32Ret:%x\n", s32Ret);
		CVI_SYS_Munmap(vir_addr, pstDumpInfo->phy_size);
		CVI_SYS_IonFree(pAddr, NULL);
		return s32Ret;
	}

	CVI_SYS_Munmap(vir_addr, pstDumpInfo->phy_size);
	s32Ret = CVI_SYS_IonFree(pAddr, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "CVI_SYS_IonFree fail, s32Ret:%x\n", s32Ret);
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_StopSmoothRawDump(const VI_SMOOTH_RAW_DUMP_INFO_S *pstDumpInfo)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, 0, 0);

	CHECK_VI_NULL_PTR(pstDumpInfo);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_STOP_SMOOTH_RAWDUMP, (CVI_VOID *)pstDumpInfo,
				sizeof(VI_SMOOTH_RAW_DUMP_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "StopSmoothRawDump fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetSmoothRawDump(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, ViPipe, 0, 1);

	CHECK_VI_PIPEID_VALID(ViPipe);
	MOD_CHECK_NULL_PTR(CVI_ID_VI, pstVideoFrame);

	stPrivData.as32PrivData[0] = s32MilliSec;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_SMOOTH_RAWDUMP, (CVI_VOID *)pstVideoFrame,
				sizeof(VIDEO_FRAME_INFO_S) * 2, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "GetSmoothRawDump fail, VIpipe:%d, s32Ret:%x\n",
			ViPipe, s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_VI_PutSmoothRawDump(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, ViPipe, 0, 1);

	CHECK_VI_PIPEID_VALID(ViPipe);
	MOD_CHECK_NULL_PTR(CVI_ID_VI, pstVideoFrame);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_PUT_SMOOTH_RAWDUMP, (CVI_VOID *)pstVideoFrame,
				sizeof(VIDEO_FRAME_INFO_S) * 2, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "PutSmoothRawDump fail, VIpipe:%d, s32Ret:%x\n",
			ViPipe, s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_VI_SetPipeFrameSource(VI_PIPE ViPipe, const VI_PIPE_FRAME_SOURCE_E enSource)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	VI_PIPE_FRAME_SOURCE_E source = enSource;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	CHECK_VI_PIPEID_VALID(ViPipe);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_PIPE_FRM_SRC, &source,
				sizeof(VI_PIPE_FRAME_SOURCE_E), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "SetPipeFrameSource fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetPipeFrameSource(VI_PIPE ViPipe, VI_PIPE_FRAME_SOURCE_E *penSource)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(penSource);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_PIPE_FRM_SRC, (CVI_VOID *)penSource,
				sizeof(VI_PIPE_FRAME_SOURCE_E), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "GetPipeFrameSource fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SendPipeYUV(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstVideoFrame);

	UNUSED(pstVideoFrame);
	UNUSED(s32MilliSec);

	CVI_TRACE_VI(CVI_DBG_ERR, "Not supported.\n");
	return CVI_FAILURE;
}

CVI_S32 CVI_VI_SendPipeRaw(CVI_U32 u32PipeNum, VI_PIPE PipeId[], const VIDEO_FRAME_INFO_S *pstVideoFrame[],
			   CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	VIDEO_FRAME_INFO_S stVideoFrame[VI_MAX_PIPE_NUM] = {0};
	CVI_U32 i = 0;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, 0, 0, 1);

	MOD_CHECK_NULL_PTR(CVI_ID_VI, pstVideoFrame);

	stPrivData.as32PrivData[0] = u32PipeNum;

	for (; i < u32PipeNum; ++i) {
		stPrivData.as32PrivData[i + 1] = PipeId[i];
		stVideoFrame[i] = *pstVideoFrame[i];
	}

	stPrivData.as32PrivData[i + 1] = s32MilliSec;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SEND_PIPE_RAW, (CVI_VOID *)&stVideoFrame,
		sizeof(stVideoFrame[VI_MAX_PIPE_NUM]), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "SendPipeRaw fail, VIpipe:%d, s32Ret:%x\n",
			u32PipeNum, s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_VI_SetPipeCrop(VI_PIPE ViPipe, const CROP_INFO_S *pstCropInfo)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstCropInfo);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_PIPE_CROP, (CVI_VOID *)pstCropInfo,
				sizeof(CROP_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Set pipe crop fail, ViPipe:%d s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetPipeCrop(VI_PIPE ViPipe, CROP_INFO_S *pstCropInfo)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstCropInfo);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_PIPE_CROP, pstCropInfo,
				sizeof(CROP_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get pipe crop fail, ViPipe:%d s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetRgbMapLeBuf(VI_PIPE ViPipe, struct VI_MEMBLOCK *pstMemblk)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstMemblk);
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_RGBMAP_LE_BUF, pstMemblk,
				sizeof(struct VI_MEMBLOCK), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get LE rgb map buf fail, ViPipe:%d s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetRgbMapSeBuf(VI_PIPE ViPipe, struct VI_MEMBLOCK *pstMemblk)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstMemblk);
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_RGBMAP_SE_BUF, pstMemblk,
				sizeof(struct VI_MEMBLOCK), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get SE rgb map buf fail, ViPipe:%d s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstChnAttr);

	if (pstChnAttr->stFrameRate.s32SrcFrameRate != pstChnAttr->stFrameRate.s32DstFrameRate) {
		CVI_TRACE_VI(CVI_DBG_WARN, "FrameRate ctrl, src(%d) dst(%d), not support yet.\n"
				, pstChnAttr->stFrameRate.s32SrcFrameRate, pstChnAttr->stFrameRate.s32DstFrameRate);
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_CHN_ATTR, (CVI_VOID *)pstChnAttr,
				sizeof(VI_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Set chn attr fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstChnAttr);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_ATTR, pstChnAttr, sizeof(VI_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get chn attr fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_EnableChn(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_ENABLE_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "enable chn fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_DisableChn(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	struct cvi_gdc_mesh *pmesh = &g_vi_mesh[ViChn];

	if (pmesh->paddr && (pmesh->paddr != DEFAULT_MESH_PADDR)) {
		CVI_SYS_IonFree(pmesh->paddr, pmesh->vaddr);
		pmesh->paddr = 0;
		pmesh->vaddr = NULL;
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_DISABLE_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "disable chn fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetChnCrop(VI_PIPE ViPipe, VI_CHN ViChn, const VI_CROP_INFO_S  *pstCropInfo)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstCropInfo);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_CHN_CROP, (CVI_VOID *)pstCropInfo,
				sizeof(VI_CROP_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Set chn crop fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetChnCrop(VI_PIPE ViPipe, VI_CHN ViChn, VI_CROP_INFO_S  *pstCropInfo)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstCropInfo);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_CROP, pstCropInfo,
				sizeof(VI_CROP_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get chn crop fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetChnFrame(VI_PIPE ViPipe, VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, ViPipe, ViChn, 1);

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	MOD_CHECK_NULL_PTR(CVI_ID_VI, pstFrameInfo);

	stPrivData.as32PrivData[0] = s32MilliSec;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_FRAME, (CVI_VOID *)pstFrameInfo,
			sizeof(VIDEO_FRAME_INFO_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get VI chn frame fail,VIpipe:%d, VIChn:%d, s32Ret:%x\n",
			ViPipe, ViChn, s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_VI_ReleaseChnFrame(VI_PIPE ViPipe, VI_CHN ViChn, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	MOD_CHECK_NULL_PTR(CVI_ID_VI, pstFrameInfo);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_RELEASE_CHN_FRAME, (CVI_VOID *)pstFrameInfo,
				sizeof(VIDEO_FRAME_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Release VI chn frame fail,VIpipe:%d, VIChn:%d, s32Ret:%x\n",
			ViPipe, ViChn, s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_VI_SetChnRotation(VI_PIPE ViPipe, VI_CHN ViChn, const ROTATION_E enRotation)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);
	ROTATION_E rotation = enRotation;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_CHN_ROTATION, &rotation,
				sizeof(ROTATION_E), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI_SetChnRotation fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetChnRotation(VI_PIPE ViPipe, VI_CHN ViChn, ROTATION_E *penRotation)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_ROTATION, (CVI_VOID *)penRotation,
				sizeof(ROTATION_E), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI_GetChnRotation fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetChnLDCAttr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, ViPipe, ViChn, 1);

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	MOD_CHECK_NULL_PTR(CVI_ID_VI, pstLDCAttr);

	VI_CHN_ATTR_S stChnAttr;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U64 paddr = 0;
	CVI_U64 paddr_old = 0;
	CVI_VOID *vaddr = NULL;
	CVI_VOID *vaddr_old = NULL;
	struct cvi_gdc_mesh *pmesh = &g_vi_mesh[ViChn];

	s32Ret = CVI_VI_GetChnAttr(ViPipe, ViChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VIpipe:%d, VIChn:%d, Get chn attr fail\n",
			     ViPipe, ViChn);
		return s32Ret;
	}

	if (pstLDCAttr->bEnable) {
		s32Ret = CVI_GDC_GenLDCMesh(stChnAttr.stSize.u32Width, stChnAttr.stSize.u32Height, &pstLDCAttr->stAttr,
					    "vi_mesh", &paddr, &vaddr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "VIpipe:%d, VIChn:%d gen mesh fail\n",
				     ViPipe, ViChn);
			return s32Ret;
		}
	} else {
		paddr = DEFAULT_MESH_PADDR;
	}

	pthread_mutex_lock(&pmesh->lock);
	if (pmesh->paddr) {
		paddr_old = pmesh->paddr;
		vaddr_old = pmesh->vaddr;
	} else {
		paddr_old = 0;
		vaddr_old = NULL;
	}
	pmesh->paddr = paddr;
	pmesh->vaddr = vaddr;
	pthread_mutex_unlock(&pmesh->lock);

	stPrivData.as32PrivData[0] = (CVI_S32)(paddr & 0xFFFFFFF);
	stPrivData.as32PrivData[1] = (CVI_S32)((paddr >> 28) & 0xFFFFFFF);
	stPrivData.as32PrivData[2] = (CVI_S32)((paddr >> 56) & 0xFF);

	s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VI_SET_CHN_LDC_ATTR, (CVI_VOID *)pstLDCAttr,
		sizeof(VI_LDC_ATTR_S), &stPrivData, -1);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Set VI chn LDC ATTR fail,VIpipe:%d, VIChn:%d, s32Ret:%x\n",
			     ViPipe, ViChn, s32Ret);
	}


	if (paddr_old != paddr && vaddr_old != vaddr && (paddr_old != DEFAULT_MESH_PADDR))
		CVI_SYS_IonFree(paddr_old, vaddr_old);

	return s32Ret;
}

CVI_S32 CVI_VI_GetChnLDCAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, ViPipe, ViChn, 1);

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	MOD_CHECK_NULL_PTR(CVI_ID_VI, pstLDCAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_LDC_ATTR, (CVI_VOID *)pstLDCAttr,
		sizeof(VI_LDC_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get VI chn LDC ATTR fail,VIpipe:%d, VIChn:%d, s32Ret:%x\n",
			ViPipe, ViChn, s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_VI_SetChnFlipMirror(VI_PIPE ViPipe, VI_CHN ViChn, CVI_BOOL bFlip, CVI_BOOL bMirror)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	stPrivData.as32PrivData[0] = bFlip;
	stPrivData.as32PrivData[1] = bMirror;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_CHN_FLIP_MIRROR, NULL,
				0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "SetChnFlipMirror fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetChnFlipMirror(VI_PIPE ViPipe, VI_CHN ViChn, CVI_BOOL *pbFlip, CVI_BOOL *pbMirror)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);
	CVI_U32 bMirrFilp[2] = {0, 0};

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pbFlip);
	CHECK_VI_NULL_PTR(pbMirror);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_FLIP_MIRROR, bMirrFilp,
				sizeof(bMirrFilp), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "GetChnFlipMirror fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}
	*pbFlip = bMirrFilp[0];
	*pbMirror = bMirrFilp[1];

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_DumpHwRegisterToFile(VI_PIPE ViPipe, FILE *fp, VI_DUMP_REGISTER_TABLE_S *pstRegTbl)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(fp);
	//CHECK_VI_NULL_PTR(pstRegTbl);

#define DUMP_MAX_SIZE 300*1024
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U64 pAddr = 0;
	MSG_PRIV_DATA_S stPrivData;
	CVI_VOID *vir_addr = NULL;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, ViPipe, 0, 1);

	UNUSED(pstRegTbl);

	s32Ret = CVI_SYS_IonAlloc_Cached(&pAddr, NULL, "VI_DMA_BUF", DUMP_MAX_SIZE);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI ion alloc size(%d) failed.\n", DUMP_MAX_SIZE);
		return CVI_ERR_VI_NOMEM;
	}

	stPrivData.as32PrivData[0] = pAddr & 0xFFFFFFFF;
	stPrivData.as32PrivData[1] = (pAddr >> 32) & 0xFFFFFFFF;
	stPrivData.as32PrivData[2] = DUMP_MAX_SIZE;

	CVI_TRACE_VI(CVI_DBG_INFO, "high_addr[%x] low_addr[%x]\n",
			stPrivData.as32PrivData[1], stPrivData.as32PrivData[0]);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_DUMP_HW_REG_TO_FILE, NULL,
			0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "dump hw register fail:%d, s32Ret:%x\n",
			ViPipe, s32Ret);
		CVI_SYS_IonFree(pAddr, NULL);
		return s32Ret;
	}

	vir_addr = CVI_SYS_Mmap(pAddr, DUMP_MAX_SIZE);

	CVI_SYS_IonInvalidateCache(pAddr, vir_addr, DUMP_MAX_SIZE);

	fwrite(vir_addr, 1, DUMP_MAX_SIZE, fp);

	fflush(fp);

	CVI_SYS_Munmap(vir_addr, DUMP_MAX_SIZE);

	s32Ret = CVI_SYS_IonFree(pAddr, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "CVI_SYS_IonFree fail:%d, s32Ret:%x\n",
			ViPipe, s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_VI_QueryPipeStatus(VI_PIPE ViPipe, VI_PIPE_STATUS_S *pstStatus)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstStatus);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_PIPE_STATUS, pstStatus, sizeof(VI_PIPE_STATUS_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "query pipe status fail, ViPipe:%d, s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_QueryChnStatus(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_STATUS_S *pstChnStatus)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstChnStatus);

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_STATUS, pstChnStatus, sizeof(VI_CHN_STATUS_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "query chn status fail, ViPipe:%d, ViChn:%d s32Ret:%x\n",
				ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetBypassFrm(CVI_U32 snr_num, CVI_U8 bypass_num)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	VI_PIPE pipe = snr_num;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, pipe, 0);

	CHECK_VI_PIPEID_VALID(pipe);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_BYPASS_FRM, &bypass_num,
				sizeof(CVI_U8), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "set bypass frm fail, ViPipe:%d,s32Ret:%x\n", pipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_Suspend(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SUSPEND, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "CVI_VI_Suspend fail\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_VI_Resume(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_RESUME, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "CVI_VI_Resume fail\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_VI_RegPmCallBack(VI_DEV ViDev, VI_PM_OPS_S *pstPmOps, void *pvData)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstPmOps);
	CHECK_VI_NULL_PTR(pvData);

	memcpy(&apstViPm[ViDev].stOps, pstPmOps, sizeof(VI_PM_OPS_S));
	apstViPm[ViDev].pvData = pvData;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_UnRegPmCallBack(VI_DEV ViDev)
{
	CHECK_VI_DEVID_VALID(ViDev);

	memset(&apstViPm[ViDev].stOps, 0, sizeof(VI_PM_OPS_S));
	apstViPm[ViDev].pvData = NULL;
	return CVI_SUCCESS;
}

/**
 * Api for debug
 */
CVI_S32 CVI_VI_SetTuningDis(CVI_S32 ctrl, CVI_S32 fe, CVI_S32 be, CVI_S32 post)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, 0, 0);
	CVI_S32 tuning_dis[] = {ctrl, fe, be, post};

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_DBG_SET_TUNING_DIS, &tuning_dis,
				sizeof(tuning_dis), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "set tuning dis fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_AttachVbPool(VI_PIPE ViPipe, VI_CHN ViChn, VB_POOL VbPool)
{
	UNUSED(ViPipe);
	UNUSED(ViChn);
	UNUSED(VbPool);
#if defined(POSSIBLE_DEAD_CODE)
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	CVI_S32 fd = get_vi_fd();
	struct vi_vb_pool_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;
	cfg.VbPool = VbPool;
	return vi_sdk_attach_vbpool(fd, &cfg);
#endif
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_DetachVbPool(VI_PIPE ViPipe, VI_CHN ViChn)
{
	UNUSED(ViPipe);
	UNUSED(ViChn);
#if defined(POSSIBLE_DEAD_CODE)
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	CVI_S32 fd = get_vi_fd();
	struct vi_vb_pool_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;
	return vi_sdk_detach_vbpool(fd, &cfg);
#endif
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_GetPipeFd(VI_PIPE ViPipe)
{
	UNUSED(ViPipe);

	CVI_TRACE_VI(CVI_DBG_ERR, "dual os not support open vi fd in linux.\n");
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_CloseFd(void)
{
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_Trig_AHD(VI_PIPE ViPipe, CVI_U8 u8AHDSignal)
{
	UNUSED(ViPipe);
	UNUSED(u8AHDSignal);
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_SetExtChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_EXT_CHN_ATTR_S *pstExtChnAttr)
{
	UNUSED(pstExtChnAttr);
	UNUSED(ViPipe);
	UNUSED(ViChn);

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_GetExtChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_EXT_CHN_ATTR_S *pstExtChnAttr)
{
	UNUSED(pstExtChnAttr);
	UNUSED(ViPipe);
	UNUSED(ViChn);

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_SetDevAttrEx(VI_DEV ViDev, const VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	UNUSED(ViDev);
	UNUSED(pstDevAttrEx);

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_GetDevAttrEx(VI_DEV ViDev, VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	UNUSED(ViDev);
	UNUSED(pstDevAttrEx);

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_SetMipiBindDev(VI_DEV ViDev, MIPI_DEV MipiDev)
{
	UNUSED(ViDev);
	UNUSED(MipiDev);
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_GetMipiBindDev(VI_DEV ViDev, MIPI_DEV *pMipiDev)
{
	UNUSED(ViDev);
	UNUSED(pMipiDev);

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_SetDevBindPipe(VI_DEV ViDev, const VI_DEV_BIND_PIPE_S *pstDevBindPipe)
{
	UNUSED(ViDev);
	UNUSED(pstDevBindPipe);

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_GetDevBindPipe(VI_DEV ViDev, VI_DEV_BIND_PIPE_S *pstDevBindPipe)
{
	UNUSED(ViDev);
	UNUSED(pstDevBindPipe);

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_SetChnEarlyInterrupt(VI_PIPE ViPipe, VI_CHN ViChn, const VI_EARLY_INTERRUPT_S *pstEarlyInterrupt)
{
	UNUSED(pstEarlyInterrupt);
	UNUSED(ViPipe);
	UNUSED(ViChn);

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_GetChnEarlyInterrupt(VI_PIPE ViPipe, VI_CHN ViChn, VI_EARLY_INTERRUPT_S *pstEarlyInterrupt)
{
	CHECK_VI_NULL_PTR(pstEarlyInterrupt);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_RegChnFlipMirrorCallBack(VI_PIPE ViPipe, VI_DEV ViDev, void *pvData)
{
	UNUSED(ViPipe);
	UNUSED(ViDev);
	UNUSED(pvData);
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_UnRegChnFlipMirrorCallBack(VI_PIPE ViPipe, VI_DEV ViDev)
{
	UNUSED(ViPipe);
	UNUSED(ViDev);
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**************************************************************************
 *   Internal APIs for other modules
 **************************************************************************/
/**
 * @deprecated
 */
CVI_VOID CVI_VI_SetMotionLV(struct mlv_info mlevel_i)
{
	UNUSED(mlevel_i);
}

/**
 * @deprecated
 */
CVI_VOID CVI_VI_SET_DIS_INFO(struct dis_info dis_i)
{
	UNUSED(dis_i);
}
