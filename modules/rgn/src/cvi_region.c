#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/queue.h>
#include <pthread.h>
#include <stdatomic.h>
#include <inttypes.h>

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "cvi_sys_base.h"
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cvi_vo.h"
#include "cvi_region.h"
#include "hashmap.h"
#include "osdc.h"

#include "cvi_msg.h"
#include "msg_client.h"

#define RGN_MAX_MSG_SEND 3

struct rgn_canvas {
	STAILQ_ENTRY(rgn_canvas) stailq;
	RGN_HANDLE Handle;
	CVI_U64 u64PhyAddr;
	CVI_U8 *pu8VirtAddr;
	CVI_U32 u32Size;
};

struct cvi_rgn_ctx {
	RGN_HANDLE Handle;
	CVI_BOOL bCreated;
	CVI_BOOL bUsed;
	RGN_ATTR_S stRegion;
	MMF_CHN_S stChn;
	RGN_CHN_ATTR_S stChnAttr;
	RGN_CANVAS_INFO_S stCanvasInfo[RGN_MAX_BUF_NUM];
	CVI_U32 u32MaxNeedIon;
	CVI_U32 ion_len;
	CVI_U8 canvas_idx;
	CVI_BOOL canvas_get;
	CVI_BOOL odec_data_valid;
};

struct cvi_rgn_bitmap {
	CVI_VOID *pBitmapVAddr;
	CVI_U32 u32BitmapSize;
};

static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_mutex_t canvas_q_lock = PTHREAD_MUTEX_INITIALIZER;
STAILQ_HEAD(rgn_canvas_q, rgn_canvas) canvas_q;

static CVI_S32 _cvi_rgn_get_ctx_(RGN_HANDLE Handle, struct cvi_rgn_ctx *pCtx);

static void rgn_init(void)
{
	STAILQ_INIT(&canvas_q);
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 CVI_RGN_Create(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 0);
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstRegion);
	if (pstRegion->unAttr.stOverlay.bFlip || pstRegion->unAttr.stOverlay.bMirror) {
		if (pstRegion->unAttr.stOverlay.stCompressInfo.enOSDCompressMode != OSD_COMPRESS_MODE_NONE) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Only non-compress mode support flip and mirror.\n");
			return CVI_ERR_RGN_NOT_SUPPORT;
		}
	}

	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_CREATE,
				(CVI_VOID *)pstRegion, sizeof(RGN_ATTR_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	pthread_mutex_lock(&canvas_q_lock);
	pthread_once(&once, rgn_init);
	pthread_mutex_unlock(&canvas_q_lock);

	return s32Ret;
}

CVI_S32 CVI_RGN_Destroy(RGN_HANDLE Handle)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 0);
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_DESTROY,
				NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_RGN_GetAttr(RGN_HANDLE Handle, RGN_ATTR_S *pstRegion)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 1);
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstRegion);

	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_GET_ATTR,
				pstRegion, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_RGN_SetAttr(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 0);
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstRegion);
	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_SET_ATTR,
				(CVI_VOID *)pstRegion, sizeof(RGN_ATTR_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_RGN_SetBitMap(RGN_HANDLE Handle, const BITMAP_S *pstBitmap)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 0), u32Len;
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;
	BITMAP_S stBitmapTemp;
	CVI_U64 u64PhyAddr;
	CVI_VOID *pVirAddr;
	RGN_ATTR_S stRegion;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstBitmap);

	stPrivData.as32PrivData[0] = Handle;

	switch (pstBitmap->enPixelFormat) {
	case PIXEL_FORMAT_ARGB_8888: {
		u32Len = pstBitmap->u32Width * pstBitmap->u32Height * 4;
		break;
	}

	case PIXEL_FORMAT_8BIT_MODE: {
		u32Len = pstBitmap->u32Width * pstBitmap->u32Height;
		break;
	}

	case PIXEL_FORMAT_ARGB_4444:
	case PIXEL_FORMAT_ARGB_1555:
	default: {
		u32Len = pstBitmap->u32Width * pstBitmap->u32Height * 2;
		break;
	}
	}

	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_GET_ATTR,
				&stRegion, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	memcpy(&stBitmapTemp, pstBitmap, sizeof(BITMAP_S));
	if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_SW) {
		u32Len = stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize;
	} else if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_HW) {
		u32Len = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
	}

	s32Ret = CVI_SYS_IonAlloc_Cached(&u64PhyAddr, &pVirAddr, "CVI_RGN_SetBitMap", u32Len);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "IonAlloc(%d) failed with s32Ret:%x\n", u32Len, s32Ret);
		return CVI_ERR_RGN_NOMEM;
	}

	memcpy(pVirAddr, pstBitmap->pData, u32Len);
	CVI_SYS_IonFlushCache(u64PhyAddr, pVirAddr, u32Len);
	//set bitmap.pData to physical addr before sending to c906l
	stBitmapTemp.pData = (CVI_VOID *)(unsigned long)u64PhyAddr;
	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_SET_BITMAP,
				&stBitmapTemp, sizeof(BITMAP_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
	}

	CVI_SYS_IonFree(u64PhyAddr, pVirAddr);

	return s32Ret;
}

CVI_S32 CVI_RGN_AttachToChn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 0), u32Len;
	MSG_PRIV_DATA_S stPrivData;
	CVI_VOID *pvMsg;
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChnAttr);

#if defined(__CV180X__)
	if (pstChn->enModId == CVI_ID_VO) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "No vo device, cannot attach to vo!\n");
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}
#endif

	u32Len = sizeof(MMF_CHN_S) + sizeof(RGN_CHN_ATTR_S);
	pvMsg = calloc(u32Len, 1);
	if (!pvMsg) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "calloc failed, size(%d)\n", u32Len);
		return CVI_ERR_RGN_NOMEM;
	}

	stPrivData.as32PrivData[0] = Handle;
	memcpy(pvMsg, (CVI_VOID *)pstChn, sizeof(MMF_CHN_S));
	memcpy(pvMsg +  sizeof(MMF_CHN_S), (CVI_VOID *)pstChnAttr, sizeof(RGN_CHN_ATTR_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_ATTACH_TO_CHN,
				pvMsg, u32Len, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
	}
	free(pvMsg);
	pvMsg = NULL;

	return s32Ret;
}

CVI_S32 CVI_RGN_DetachFromChn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 0);
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);

#if defined(__CV180X__)
	if (pstChn->enModId == CVI_ID_VO) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "No vo device, cannot detach from vo!\n");
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}
#endif

	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_DETACH_FROM_CHN,
				(CVI_VOID *)pstChn, sizeof(MMF_CHN_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_RGN_SetDisplayAttr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 0), u32Len;
	MSG_PRIV_DATA_S stPrivData;
	CVI_VOID *pvMsg;
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChnAttr);

	u32Len = sizeof(MMF_CHN_S) + sizeof(RGN_CHN_ATTR_S);
	pvMsg = calloc(u32Len, 1);
	if (!pvMsg) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "calloc failed, size(%d)\n", u32Len);
		return CVI_ERR_RGN_NOMEM;
	}

	stPrivData.as32PrivData[0] = Handle;
	memcpy(pvMsg, (CVI_VOID *)pstChn, sizeof(MMF_CHN_S));
	memcpy(pvMsg +  sizeof(MMF_CHN_S), (CVI_VOID *)pstChnAttr, sizeof(RGN_CHN_ATTR_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_SET_DISP_ATTR,
				pvMsg, u32Len, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
	}
	free(pvMsg);
	pvMsg = NULL;

	return s32Ret;
}

CVI_S32 CVI_RGN_GetDisplayAttr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 1);
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChnAttr);

	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync2(u32ModFd, MSG_CMD_RGN_GET_DISP_ATTR,
				(CVI_VOID *)pstChn, sizeof(MMF_CHN_S), pstChnAttr, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
	}

	return s32Ret;
}

static CVI_S32 _cvi_rgn_get_ctx_(RGN_HANDLE Handle, struct cvi_rgn_ctx *pCtx)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 1), i;
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pCtx);

	stPrivData.as32PrivData[0] = Handle;
	for (i = 0; i < RGN_MAX_MSG_SEND; ++i) {
		s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_GET_CTX,
					(CVI_VOID *)pCtx, 0, &stPrivData);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_INFO, "CVI_MSG_SendSync failed with s32Ret:%x\n", s32Ret);
		} else {
			break;
		}
	}

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 CVI_RGN_GetCanvasInfo(RGN_HANDLE Handle, RGN_CANVAS_INFO_S *pstCanvasInfo)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 1), i;
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;
	struct rgn_canvas *canvas;
	struct cvi_rgn_ctx ctx;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstCanvasInfo);

	s32Ret = _cvi_rgn_get_ctx_(Handle, &ctx);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get context failed!\n");
		return CVI_FAILURE;
	}

	stPrivData.as32PrivData[0] = Handle;
	for (i = 0; i < RGN_MAX_MSG_SEND; ++i) {
		s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_GET_CANVAS_INFO,
					pstCanvasInfo, 0, &stPrivData);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "CVI_MSG_SendSync failed with s32Ret:%x\n", s32Ret);
		} else {
			break;
		}
	}

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	canvas = calloc(1, sizeof(struct rgn_canvas));
	if (!canvas) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "malloc failed.\n");
		return CVI_ERR_RGN_NOMEM;
	}
	pthread_mutex_lock(&canvas_q_lock);
	canvas->u64PhyAddr = pstCanvasInfo->u64PhyAddr;
	canvas->u32Size = ctx.ion_len;
	pstCanvasInfo->pu8VirtAddr = canvas->pu8VirtAddr =
			CVI_SYS_MmapCache(pstCanvasInfo->u64PhyAddr, canvas->u32Size);
	if (pstCanvasInfo->pu8VirtAddr == NULL) {
		free(canvas);
		CVI_TRACE_RGN(CVI_DBG_ERR, "CVI_SYS_Mmap NG.\n");
		return CVI_FAILURE;
	}
	canvas->Handle = Handle;
	pstCanvasInfo->pstCanvasCmprAttr = (RGN_CANVAS_CMPR_ATTR_S *)pstCanvasInfo->pu8VirtAddr;
	pstCanvasInfo->pstObjAttr = (RGN_CMPR_OBJ_ATTR_S *)(pstCanvasInfo->pu8VirtAddr +
			sizeof(RGN_CANVAS_CMPR_ATTR_S));
	STAILQ_INSERT_TAIL(&canvas_q, canvas, stailq);
	pthread_mutex_unlock(&canvas_q_lock);

	return s32Ret;
}

CVI_S32 CVI_RGN_UpdateCanvas(RGN_HANDLE Handle)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 0), i, j = 0;
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;
	struct rgn_canvas *rgnCanvasMap = NULL;
	struct cvi_rgn_ctx ctx;
	CVI_U32 u32Bpp;
	struct cvi_rgn_bitmap *pstBitmaps;

	s32Ret = _cvi_rgn_get_ctx_(Handle, &ctx);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get context failed!\n");
		return CVI_FAILURE;
	}

	if (!STAILQ_EMPTY(&canvas_q)) {
		STAILQ_FOREACH(rgnCanvasMap, &canvas_q, stailq) {
			if (rgnCanvasMap->Handle == Handle) {
				break;
			}
		}
	}

	if (ctx.stCanvasInfo[ctx.canvas_idx].bCompressed &&
		ctx.stCanvasInfo[ctx.canvas_idx].enOSDCompressMode == OSD_COMPRESS_MODE_HW) {
		RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr = NULL;
		RGN_CMPR_OBJ_ATTR_S *pstObjAttr = NULL;
		CVI_S32 status;
		OSDC_Canvas_Attr_S osdc_canvas;
		OSDC_DRAW_OBJ_S *obj_vec;
		CVI_U32 bs_size;

		//cmpr canvas info is stored in ion which needs to be flush before reading
		CVI_SYS_IonFlushCache(rgnCanvasMap->u64PhyAddr,
			rgnCanvasMap->pu8VirtAddr, rgnCanvasMap->u32Size);
		pstCanvasCmprAttr = (RGN_CANVAS_CMPR_ATTR_S *)rgnCanvasMap->pu8VirtAddr;
		pstObjAttr = (RGN_CMPR_OBJ_ATTR_S *)(rgnCanvasMap->pu8VirtAddr +
			sizeof(RGN_CANVAS_CMPR_ATTR_S));

		for (i = 0; i < pstCanvasCmprAttr->u32ObjNum; ++i) {
			if (pstObjAttr[i].enObjType == RGN_CMPR_LINE) {
				CVI_TRACE_RGN(CVI_DBG_DEBUG, "start(%d %d) end(%d %d) Thick(%d) Color(0x%x)\n",
					pstObjAttr[i].stLine.stPointStart.s32X,
					pstObjAttr[i].stLine.stPointStart.s32Y,
					pstObjAttr[i].stLine.stPointEnd.s32X,
					pstObjAttr[i].stLine.stPointEnd.s32Y,
					pstObjAttr[i].stLine.u32Thick,
					pstObjAttr[i].stLine.u32Color);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_RECT) {
				CVI_TRACE_RGN(CVI_DBG_DEBUG,
					"xywh(%d %d %d %d) Thick(%d) Color(0x%x) is_fill(%d)\n",
					pstObjAttr[i].stRgnRect.stRect.s32X,
					pstObjAttr[i].stRgnRect.stRect.s32Y,
					pstObjAttr[i].stRgnRect.stRect.u32Width,
					pstObjAttr[i].stRgnRect.stRect.u32Height,
					pstObjAttr[i].stRgnRect.u32Thick,
					pstObjAttr[i].stRgnRect.u32Color,
					pstObjAttr[i].stRgnRect.u32IsFill);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_BIT_MAP) {
				CVI_TRACE_RGN(CVI_DBG_DEBUG, "xywh(%d %d %d %d) u32BitmapPAddr(%x)\n",
					pstObjAttr[i].stBitmap.stRect.s32X,
					pstObjAttr[i].stBitmap.stRect.s32Y,
					pstObjAttr[i].stBitmap.stRect.u32Width,
					pstObjAttr[i].stBitmap.stRect.u32Height,
					pstObjAttr[i].stBitmap.u32BitmapPAddr);
			}
		}

		osdc_canvas.width = ctx.stRegion.unAttr.stOverlay.stSize.u32Width;
		osdc_canvas.height = ctx.stRegion.unAttr.stOverlay.stSize.u32Height;
		osdc_canvas.bg_color_code = ctx.stRegion.unAttr.stOverlay.u32BgColor;
		switch (ctx.stRegion.unAttr.stOverlay.enPixelFormat) {
		case PIXEL_FORMAT_ARGB_8888:
			osdc_canvas.format = OSD_ARGB8888;
			u32Bpp = 4;
			break;

		case PIXEL_FORMAT_ARGB_4444:
			osdc_canvas.format = OSD_ARGB4444;
			u32Bpp = 2;
			break;

		case PIXEL_FORMAT_ARGB_1555:
			osdc_canvas.format = OSD_ARGB1555;
			u32Bpp = 2;
			break;

		case PIXEL_FORMAT_8BIT_MODE:
			osdc_canvas.format = OSD_LUT8;
			u32Bpp = 1;
			break;

		default:
			osdc_canvas.format = OSD_ARGB1555;
			u32Bpp = 2;
			break;
		}

		obj_vec = calloc(sizeof(OSDC_DRAW_OBJ_S) * pstCanvasCmprAttr->u32ObjNum, 1);
		if (!obj_vec) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "calloc size (%zu) failed!\n",
							sizeof(OSDC_DRAW_OBJ_S) * pstCanvasCmprAttr->u32ObjNum);
			return CVI_ERR_RGN_NOBUF;
		}
		pstBitmaps = (struct cvi_rgn_bitmap *)calloc(pstCanvasCmprAttr->u32ObjNum,
						sizeof(struct cvi_rgn_bitmap));
		if (!pstBitmaps) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "calloc size (%zu) failed!\n",
							pstCanvasCmprAttr->u32ObjNum * sizeof(struct cvi_rgn_bitmap));
			free(obj_vec);
			return CVI_ERR_RGN_NOBUF;
		}

		for (i = 0; i < pstCanvasCmprAttr->u32ObjNum; ++i) {
			if (pstObjAttr[i].enObjType == RGN_CMPR_LINE) {
				OSDC_SetLineObjAttr(&osdc_canvas, &obj_vec[i],
				pstObjAttr[i].stLine.u32Color,
					pstObjAttr[i].stLine.stPointStart.s32X,
					pstObjAttr[i].stLine.stPointStart.s32Y,
					pstObjAttr[i].stLine.stPointEnd.s32X,
					pstObjAttr[i].stLine.stPointEnd.s32Y,
					pstObjAttr[i].stLine.u32Thick);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_RECT) {
				OSDC_SetRectObjAttr(&osdc_canvas, &obj_vec[i],
					pstObjAttr[i].stRgnRect.u32Color,
					pstObjAttr[i].stRgnRect.stRect.s32X,
					pstObjAttr[i].stRgnRect.stRect.s32Y,
					pstObjAttr[i].stRgnRect.stRect.u32Width,
					pstObjAttr[i].stRgnRect.stRect.u32Height,
					pstObjAttr[i].stRgnRect.u32IsFill,
					pstObjAttr[i].stRgnRect.u32Thick);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_BIT_MAP) {
				pstBitmaps[j].u32BitmapSize = pstObjAttr[i].stBitmap.stRect.u32Width *
								pstObjAttr[i].stBitmap.stRect.u32Height * u32Bpp;
				pstBitmaps[j].pBitmapVAddr = CVI_SYS_MmapCache(pstObjAttr[i].stBitmap.u32BitmapPAddr,
								pstBitmaps[j].u32BitmapSize);

				OSDC_SetBitmapObjAttr(&osdc_canvas, &obj_vec[i],
					pstBitmaps[j++].pBitmapVAddr,
					pstObjAttr[i].stBitmap.stRect.s32X,
					pstObjAttr[i].stBitmap.stRect.s32Y,
					pstObjAttr[i].stBitmap.stRect.u32Width,
					pstObjAttr[i].stBitmap.stRect.u32Height,
					false);
			}
		}

		status = OSDC_DrawCmprCanvas(&osdc_canvas, &obj_vec[0], pstCanvasCmprAttr->u32ObjNum,
			rgnCanvasMap->pu8VirtAddr, ctx.ion_len,  &bs_size);
		if (status != 1) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Region(%d) needs ion size(%d), current size(%d)!\n",
				Handle, bs_size, ctx.ion_len);
			OSDC_DrawCmprCanvas(&osdc_canvas, &obj_vec[0], 0,
				rgnCanvasMap->pu8VirtAddr, ctx.ion_len,  &bs_size);
		}
		if (j) {
			for (i = 0; i < j; i++) {
				CVI_SYS_Munmap(pstBitmaps[i].pBitmapVAddr, pstBitmaps[i].u32BitmapSize);
			}
		}
		free(pstBitmaps);
		// store bitstream size in bit[32:63], after C906L gets it,
		// it should be restored to image width and height
		*((unsigned int *)rgnCanvasMap->pu8VirtAddr + 1) = bs_size;
		free(obj_vec);
		stPrivData.as32PrivData[1] = osdc_canvas.width;
		stPrivData.as32PrivData[2] = osdc_canvas.height;
	}

	CVI_SYS_IonFlushCache(rgnCanvasMap->u64PhyAddr,
		rgnCanvasMap->pu8VirtAddr, rgnCanvasMap->u32Size);

	stPrivData.as32PrivData[0] = Handle;
	for (i = 0; i < RGN_MAX_MSG_SEND; ++i) {
		s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_UPDATE_CANVAS,
					NULL, 0, &stPrivData);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "CVI_MSG_SendSync failed with s32Ret:%x\n", s32Ret);
		} else {
			break;
		}
	}
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	pthread_mutex_lock(&canvas_q_lock);
	if (!STAILQ_EMPTY(&canvas_q)) {
		STAILQ_FOREACH(rgnCanvasMap, &canvas_q, stailq) {
			if (rgnCanvasMap->Handle == Handle) {
				STAILQ_REMOVE(&canvas_q, rgnCanvasMap, rgn_canvas, stailq);
				CVI_SYS_Munmap(rgnCanvasMap->pu8VirtAddr, rgnCanvasMap->u32Size);
				free(rgnCanvasMap);
				break;
			}
		}
	}
	pthread_mutex_unlock(&canvas_q_lock);

	return s32Ret;
}

CVI_S32 CVI_RGN_SetChnPalette(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_PALETTE_S *pstPalette)
{
	CVI_U32 u32ModFd = MODFD2(CVI_ID_RGN, 0, 0, 0), u32Len;
	MSG_PRIV_DATA_S stPrivData;
	CVI_VOID *pvMsg;
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstPalette);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstPalette->pstPaletteTable);

	u32Len = sizeof(MMF_CHN_S) + sizeof(RGN_PALETTE_S) + sizeof(RGN_RGBQUARD_S) * pstPalette->lut_length;
	pvMsg = calloc(u32Len, 1);
	if (!pvMsg) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "calloc failed, size(%d)\n", u32Len);
		return CVI_ERR_RGN_NOMEM;
	}
	stPrivData.as32PrivData[0] = Handle;
	memcpy(pvMsg, (CVI_VOID *)pstChn, sizeof(MMF_CHN_S));
	memcpy(pvMsg +  sizeof(MMF_CHN_S), (CVI_VOID *)pstPalette, sizeof(RGN_PALETTE_S));
	memcpy(pvMsg +  sizeof(MMF_CHN_S) + sizeof(RGN_PALETTE_S), (CVI_VOID *)pstPalette->pstPaletteTable,
			sizeof(RGN_RGBQUARD_S) * pstPalette->lut_length);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_RGN_SET_CHN_PALETTE,
				pvMsg, u32Len, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
	}
	free(pvMsg);
	pvMsg = NULL;

	return s32Ret;
}
