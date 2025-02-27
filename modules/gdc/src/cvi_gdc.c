#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/mman.h>
//#include <cvi_vi_ctx.h>
#include <ldc_uapi.h>

#include "cvi_buffer.h"
#include "cvi_sys_base.h"
#include "cvi_sys.h"
#include "cvi_vb.h"

#include "cvi_vi.h"
#include "cvi_vpss.h"

#include "cvi_gdc.h"
#include "gdc_mesh.h"
//#include "ldc_ioctl.h"
//#include "vi_ioctl.h"
#include "gdc_ctx.h"
#include "cvi_msg.h"
#include "msg_client.h"

// #define LDC_YUV_BLACK 0x808000
// #define LDC_RGB_BLACK 0x0

#define CHECK_GDC_FORMAT(imgIn, imgOut)                                                                                \
	do {                                                                                                           \
		if (imgIn.stVFrame.enPixelFormat != imgOut.stVFrame.enPixelFormat) {                                   \
			CVI_TRACE_GDC(CVI_DBG_ERR, "in/out pixelformat(%d-%d) mismatch\n",                             \
				      imgIn.stVFrame.enPixelFormat, imgOut.stVFrame.enPixelFormat);                    \
			return CVI_ERR_GDC_ILLEGAL_PARAM;                                                              \
		}                                                                                                      \
		if (!GDC_SUPPORT_FMT(imgIn.stVFrame.enPixelFormat)) {                                                  \
			CVI_TRACE_GDC(CVI_DBG_ERR, "pixelformat(%d) unsupported\n", imgIn.stVFrame.enPixelFormat);     \
			return CVI_ERR_GDC_ILLEGAL_PARAM;                                                              \
		}                                                                                                      \
	} while (0)

static CVI_S32 gdc_rotation_check_size(ROTATION_E enRotation, const GDC_TASK_ATTR_S *pstTask)
{
	if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "invalid rotation(%d).\n", enRotation);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	if (enRotation == ROTATION_90 || enRotation == ROTATION_270 || enRotation == ROTATION_XY_FLIP) {
		if (pstTask->stImgOut.stVFrame.u32Width < pstTask->stImgIn.stVFrame.u32Height) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output width(%d) < input height(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Width,
				      pstTask->stImgIn.stVFrame.u32Height);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
		if (pstTask->stImgOut.stVFrame.u32Height < pstTask->stImgIn.stVFrame.u32Width) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output height(%d) < input width(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Height,
				      pstTask->stImgIn.stVFrame.u32Width);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	} else {
		if (pstTask->stImgOut.stVFrame.u32Width < pstTask->stImgIn.stVFrame.u32Width) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output width(%d) < input width(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Width,
				      pstTask->stImgIn.stVFrame.u32Width);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
		if (pstTask->stImgOut.stVFrame.u32Height < pstTask->stImgIn.stVFrame.u32Height) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output height(%d) < input height(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Height,
				      pstTask->stImgIn.stVFrame.u32Height);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	}

	return CVI_SUCCESS;
}

void gdcq_init(void)
{
}

CVI_BOOL bSuspend;

CVI_S32 CVI_GDC_Suspend(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_GDC, 0, 0);
	bSuspend = CVI_TRUE;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_GDC_SUSPEND, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_GDC_Suspend fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_Resume(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_GDC, 0, 0);
	bSuspend = CVI_FALSE;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_GDC_RESUME, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_GDC_Resume fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 CVI_GDC_BeginJob(GDC_HANDLE *phHandle)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, phHandle);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_GDC, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_GDC_BEGAIN_JOB, (CVI_VOID *)phHandle, sizeof(GDC_HANDLE), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_GDC_BeginJob fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_EndJob(GDC_HANDLE hHandle)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, hHandle);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_GDC, 0, 0);
	if (bSuspend == CVI_TRUE)
		return CVI_ERR_GDC_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_GDC_END_JOB, (CVI_VOID *)&hHandle, sizeof(GDC_HANDLE), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_GDC_EndJob fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_CancelJob(GDC_HANDLE hHandle)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, hHandle);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_GDC, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_GDC_CANCEL_JOB, (CVI_VOID *)&hHandle, sizeof(GDC_HANDLE), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_GDC_CancelJob fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_AddRotationTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, ROTATION_E enRotation)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstTask);
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, hHandle);

	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_GDC, 0, 0);

	if (enRotation == ROTATION_180) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "do not support rotation 180\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	if (gdc_rotation_check_size(enRotation, pstTask) != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_rotation_check_size fail\n");
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	struct gdc_task_attr attr;
	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	memcpy(attr.au64privateData, pstTask->au64privateData, sizeof(attr.au64privateData));
	attr.reserved = pstTask->reserved;
	attr.enRotation = enRotation;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_GDC_ADD_ROT_TASK, (CVI_VOID *)&attr, sizeof(attr), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_GDC_AddRotationTask fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_AddLDCTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask
	, const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstTask);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, hHandle);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstLDCAttr);
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_GDC, 0, 0);

	if (enRotation == ROTATION_180) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "do not support rotation 180\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	if (enRotation != ROTATION_0) {
		if (gdc_rotation_check_size(enRotation, pstTask) != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_rotation_check_size fail\n");
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	}

	struct gdc_task_attr attr;

	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	memcpy(attr.au64privateData, pstTask->au64privateData, sizeof(attr.au64privateData));
	attr.reserved = pstTask->reserved;
	attr.enRotation = enRotation;
	memcpy(&attr.stLDCAttr, pstLDCAttr, sizeof(attr.stLDCAttr));

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_GDC_ADD_LDC_TASK, (CVI_VOID *)&attr, sizeof(attr), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_GDC_AddLDCTask fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_GenLDCMesh(CVI_U32 u32Width, CVI_U32 u32Height, const LDC_ATTR_S *pstLDCAttr,
		const char *name, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr)
{
	CVI_U64 paddr;
	CVI_VOID *vaddr;
	SIZE_S in_size, out_size;
	CVI_U32 mesh_1st_size = 0, mesh_2nd_size = 0, mesh_size = 0;
	ROTATION_E enRotation = ROTATION_0;

	if (pstLDCAttr->bAspect) {
		if (pstLDCAttr->s32XYRatio < 0 || pstLDCAttr->s32XYRatio > 100) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "Invalid LDC s32XYRatio(%d).\n"
				      , pstLDCAttr->s32XYRatio);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	} else {
		if (pstLDCAttr->s32XRatio < 0 || pstLDCAttr->s32XRatio > 100) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "Invalid LDC s32XRatio(%d).\n"
				      , pstLDCAttr->s32XRatio);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
		if (pstLDCAttr->s32YRatio < 0 || pstLDCAttr->s32YRatio > 100) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "Invalid LDC s32YRatio(%d).\n"
				      , pstLDCAttr->s32YRatio);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	}
	if (pstLDCAttr->s32CenterXOffset < -511 || pstLDCAttr->s32CenterXOffset > 511) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "Invalid LDC s32CenterXOffset(%d).\n"
			      , pstLDCAttr->s32CenterXOffset);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}
	if (pstLDCAttr->s32CenterYOffset < -511 || pstLDCAttr->s32CenterYOffset > 511) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "Invalid LDC s32CenterYOffset(%d).\n"
			      , pstLDCAttr->s32CenterYOffset);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}
	if (pstLDCAttr->s32DistortionRatio < -300 || pstLDCAttr->s32DistortionRatio > 500) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "Invalid LDC s32DistortionRatio(%d).\n"
			      , pstLDCAttr->s32DistortionRatio);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}
	if (!name) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "Please asign name for LDC mesh\n");
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
	in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
	out_size.u32Width = in_size.u32Width;
	out_size.u32Height = in_size.u32Height;

	mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
	mesh_size = mesh_1st_size + mesh_2nd_size;

	CVI_TRACE_GDC(CVI_DBG_DEBUG, "W=%d, H=%d, mesh_size=%d\n",
			in_size.u32Width, in_size.u32Height,
			mesh_size);

	if (CVI_SYS_IonAlloc_Cached(&paddr, &vaddr, name, mesh_size) != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, " Can't acquire memory for LDC mesh.\n");
		return CVI_ERR_GDC_NOMEM;
	}

	if (mesh_gen_ldc(in_size, out_size, pstLDCAttr, paddr, vaddr, enRotation) != CVI_SUCCESS) {
		CVI_SYS_IonFree(paddr, vaddr);
		CVI_TRACE_GDC(CVI_DBG_ERR, "Can't generate ldc mesh.\n");
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}
	CVI_SYS_IonFlushCache(paddr, vaddr, mesh_size);

	*pu64PhyAddr = paddr;
	*ppVirAddr = vaddr;

	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_LoadLDCMesh(CVI_U32 u32Width, CVI_U32 u32Height, const char *fileNname
	, const char *tskName, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr)
{
	CVI_U64 paddr;
	CVI_VOID *vaddr;
	SIZE_S in_size, out_size;
	CVI_U32 mesh_1st_size = 0, mesh_2nd_size = 0, mesh_size = 0;

	if (!fileNname) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "Please asign name for LDC mesh\n");
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	if (!tskName) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "Please asign task name for LDC\n");
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
	in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
	out_size.u32Width = in_size.u32Width;
	out_size.u32Height = in_size.u32Height;

	mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
	mesh_size = mesh_1st_size + mesh_2nd_size;

	CVI_TRACE_GDC(CVI_DBG_DEBUG, "W=%d, H=%d, mesh_size=%d\n"
		, in_size.u32Width, in_size.u32Height, mesh_size);

	// acquire memory space for mesh.
	if (CVI_SYS_IonAlloc_Cached(&paddr, &vaddr, tskName, mesh_size) != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "Can't acquire memory for mesh.\n");
		return CVI_ERR_GDC_NOMEM;
	}
	memset(vaddr, 0 , mesh_size);

	FILE *fp = fopen(fileNname, "rb");
	if (!fp) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "open file:%s failed.\n", fileNname);
		CVI_SYS_IonFree(paddr, vaddr);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);

	if (mesh_size != (CVI_U32)fileSize) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "loadmesh file:(%s) size is not match.\n", fileNname);
		CVI_SYS_IonFree(paddr, vaddr);
		fclose(fp);
		return CVI_FAILURE;
	}
	rewind(fp);

	CVI_TRACE_GDC(CVI_DBG_DEBUG, "load mesh size:%d, mesh phy addr:%#"PRIx64", vir addr:%p.\n",
		mesh_size, paddr, vaddr);

	fread(vaddr, mesh_size, 1, fp);
	fclose(fp);
	CVI_SYS_IonFlushCache(paddr, vaddr, mesh_size);

	*pu64PhyAddr = paddr;
	*ppVirAddr = vaddr;

	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_SetBufWrapAttr(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, const LDC_BUF_WRAP_S *pstBufWrap)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstTask);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, hHandle);
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_GDC, 0, 0);
	struct ldc_buf_wrap_cfg *cfg;

	cfg = malloc(sizeof(*cfg));
	if (!cfg) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc malloc fails.\n");
		return CVI_FAILURE;
	}

	memset(cfg, 0, sizeof(*cfg));
	cfg->handle = hHandle;
	memcpy(&cfg->stTask.stImgIn, &pstTask->stImgIn, sizeof(cfg->stTask.stImgIn));
	memcpy(&cfg->stTask.stImgOut, &pstTask->stImgOut, sizeof(cfg->stTask.stImgOut));
	memcpy(&cfg->stBufWrap, pstBufWrap, sizeof(cfg->stBufWrap));

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_GDC_SET_BUF_WRAP, (CVI_VOID *)&cfg, sizeof(cfg), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_GDC_SetBufWrapAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	free(cfg);

	return s32Ret;
}

CVI_S32 CVI_GDC_GetBufWrapAttr(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, LDC_BUF_WRAP_S *pstBufWrap)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstTask);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, hHandle);
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);
	struct ldc_buf_wrap_cfg *cfg;
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_GDC, 0, 0);

	cfg = malloc(sizeof(*cfg));
	if (!cfg) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc malloc fails.\n");
		return CVI_FAILURE;
	}

	memset(cfg, 0, sizeof(*cfg));
	cfg->handle = hHandle;
	memcpy(&cfg->stTask, pstTask, sizeof(cfg->stTask));

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_GDC_GET_BUF_WRAP, (CVI_VOID *)cfg, sizeof(*cfg), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_GDC_GetBufWrapAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	if (s32Ret == CVI_SUCCESS)
		memcpy(pstBufWrap, &cfg->stBufWrap, sizeof(*pstBufWrap));

	free(cfg);

	return s32Ret;
}

CVI_S32 CVI_GDC_DumpMesh(MESH_DUMP_ATTR_S *pMeshDumpAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pMeshDumpAttr);

	CVI_U64 phyMesh;
	UNUSED(phyMesh);
	CVI_VOID *virMesh;
	CVI_U32 u32Width, u32Height, VpssGrp, VpssChn, ViChn;
	SIZE_S in_size, out_size;
	CVI_U32 mesh_1st_size, mesh_2nd_size, meshSize;
	CVI_S32 s32Ret;
	VPSS_CHN_ATTR_S stVPSSChnAttr;
	VI_CHN_ATTR_S stVIChnAttr;

	FILE *fp;
	MOD_ID_E mod = pMeshDumpAttr->enModId;
	CVI_CHAR *filePath = pMeshDumpAttr->binFileName;

	switch (mod) {
	case CVI_ID_VI:
		ViChn = pMeshDumpAttr->viMeshAttr.chn;
		phyMesh = g_vi_mesh[ViChn].paddr;
		virMesh = g_vi_mesh[ViChn].vaddr;
		s32Ret = CVI_VI_GetChnAttr(0, ViChn, &stVIChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_VI_GetChnAttr fail\n");
			return s32Ret;
		}
		u32Width = stVIChnAttr.stSize.u32Width;
		u32Height = stVIChnAttr.stSize.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		out_size.u32Width = in_size.u32Width;
		out_size.u32Height = in_size.u32Height;
		mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
		meshSize = mesh_1st_size + mesh_2nd_size;
		break;
	case CVI_ID_VPSS:
		VpssGrp = pMeshDumpAttr->vpssMeshAttr.grp;
		VpssChn = pMeshDumpAttr->vpssMeshAttr.chn;
		phyMesh = mesh[VpssGrp][VpssChn].paddr;
		virMesh = mesh[VpssGrp][VpssChn].vaddr;

		s32Ret = CVI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stVPSSChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "Grp(%d) Chn(%d) Get chn attr fail\n",
				      VpssGrp, VpssChn);
			return s32Ret;
		}
		u32Width = stVPSSChnAttr.u32Width;
		u32Height = stVPSSChnAttr.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		out_size.u32Width = in_size.u32Width;
		out_size.u32Height = in_size.u32Height;
		mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
		meshSize = mesh_1st_size + mesh_2nd_size;
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	CVI_TRACE_GDC(CVI_DBG_DEBUG, "dump mesh size:%d, mesh phy addr:%#"PRIx64", vir addr:%p.\n",
		meshSize, phyMesh, virMesh);

	fp = fopen(filePath, "wb");
	if (!fp) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "open file:%s failed.\n", filePath);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}
	fwrite(virMesh, meshSize, 1, fp);
	fflush(fp);
	fclose(fp);
	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_LoadMesh(MESH_DUMP_ATTR_S *pMeshDumpAttr, const LDC_ATTR_S *pstLDCAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pMeshDumpAttr);

	CVI_U64 phyMesh = 0;
	CVI_U64 phyMesh_old = 0;
	CVI_VOID *virMesh = NULL;
	CVI_VOID *virMesh_old = NULL;
	CVI_U32 VpssGrp = 0, VpssChn = 0, ViChn = 0;
	SIZE_S in_size, out_size;
	CVI_U32 mesh_1st_size, mesh_2nd_size, mesh_size;
	CVI_U32 u32Width, u32Height;
	struct cvi_gdc_mesh *pmesh;
	FILE *fp;
	CVI_U32 u32ModFd;
	MOD_ID_E mod = pMeshDumpAttr->enModId;
	CVI_CHAR *filePath = pMeshDumpAttr->binFileName;
	CVI_S32 s32Ret;
	VPSS_CHN_ATTR_S stVPSSChnAttr;
	VI_CHN_ATTR_S stVIChnAttr;
	MSG_PRIV_DATA_S stPrivData;
	VI_LDC_ATTR_S stVILDCAttr;
	VPSS_LDC_ATTR_S stVPSSLDCAttr;

	switch (mod) {
	case CVI_ID_VI:
		ViChn = pMeshDumpAttr->viMeshAttr.chn;
		s32Ret = CVI_VI_GetChnAttr(0, ViChn, &stVIChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "Get chn attr fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", 0, ViChn, s32Ret);
			return s32Ret;
		}
		pmesh = &g_vi_mesh[ViChn];
		u32Width = stVIChnAttr.stSize.u32Width;
		u32Height = stVIChnAttr.stSize.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		break;
	case CVI_ID_VPSS:
		VpssGrp = pMeshDumpAttr->vpssMeshAttr.grp;
		VpssChn = pMeshDumpAttr->vpssMeshAttr.chn;
		pmesh = &mesh[VpssGrp][VpssChn];

		s32Ret = CVI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stVPSSChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "Grp(%d) Chn(%d) Get chn attr fail\n",
				VpssGrp, VpssChn);
			return s32Ret;
		}

		u32Width = stVPSSChnAttr.u32Width;
		u32Height = stVPSSChnAttr.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	out_size.u32Width = in_size.u32Width;
	out_size.u32Height = in_size.u32Height;

	mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
	mesh_size = mesh_1st_size + mesh_2nd_size;

	fp = fopen(filePath, "rb");
	if (!fp) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "open file:%s failed.\n", filePath);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);

	if (mesh_size != (CVI_U32)fileSize) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "loadmesh file:(%s) size is not match.\n", filePath);
		fclose(fp);
		return CVI_FAILURE;
	}
	rewind(fp);

	// acquire memory space for mesh.
	if (CVI_SYS_IonAlloc_Cached(&phyMesh, &virMesh, "gdc_mesh", mesh_size) != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "Can't acquire memory for gdc mesh.\n");
		fclose(fp);
		return CVI_ERR_VPSS_NOMEM;
	}

	CVI_TRACE_GDC(CVI_DBG_DEBUG, "load mesh size:%d, mesh phy addr:%#"PRIx64", vir addr:%p.\n",
		      mesh_size, phyMesh, virMesh);

	fread(virMesh, mesh_size, 1, fp);
	CVI_SYS_IonFlushCache(phyMesh, virMesh, mesh_size);

	pthread_mutex_lock(&pmesh->lock);
	if (pmesh->paddr) {
		phyMesh_old = pmesh->paddr;
		virMesh_old = pmesh->vaddr;
	} else {
		phyMesh_old = 0;
		virMesh_old = NULL;
	}
	pmesh->paddr = phyMesh;
	pmesh->vaddr = virMesh;
	pthread_mutex_unlock(&pmesh->lock);

	stPrivData.as32PrivData[0] = (CVI_S32)(phyMesh & 0xFFFFFFF);
	stPrivData.as32PrivData[1] = (CVI_S32)((phyMesh >> 28) & 0xFFFFFFF);
	stPrivData.as32PrivData[2] = (CVI_S32)((phyMesh >> 56) & 0xFF);

	switch (mod) {
	case CVI_ID_VI:
		g_vi_mesh[ViChn].meshSize = mesh_size;
		stVILDCAttr.bEnable = CVI_TRUE;
		memcpy(&stVILDCAttr.stAttr, pstLDCAttr, sizeof(*pstLDCAttr));
		u32ModFd = MODFD(CVI_ID_VI, 0, ViChn);
		s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VI_SET_CHN_LDC_ATTR, (CVI_VOID *)&stVILDCAttr,
					   sizeof(VI_LDC_ATTR_S), &stPrivData, -1);

		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "VI Set Chn(%d) LDC fail\n", ViChn);
			fclose(fp);
			return CVI_FAILURE;
		}
		break;
	case CVI_ID_VPSS:
		mesh[VpssGrp][VpssChn].meshSize = mesh_size;
		stVPSSLDCAttr.bEnable = CVI_TRUE;
		memcpy(&stVPSSLDCAttr.stAttr, pstLDCAttr, sizeof(*pstLDCAttr));
		u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);
		s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VPSS_SET_CHN_LDCATTR, (CVI_VOID *)&stVPSSLDCAttr,
					   sizeof(VPSS_LDC_ATTR_S), &stPrivData, 30000);

		if (s32Ret != CVI_SUCCESS) {
			if (phyMesh_old != phyMesh && virMesh_old != virMesh && (phyMesh != DEFAULT_MESH_PADDR))
				CVI_SYS_IonFree(phyMesh, virMesh);
			pthread_mutex_lock(&pmesh->lock);
			pmesh->paddr = phyMesh_old;
			pmesh->vaddr = virMesh_old;
			pthread_mutex_unlock(&pmesh->lock);
			CVI_TRACE_GDC(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d Ldc_attr fail ret:%d\n",
				      VpssGrp, VpssChn, s32Ret);
			fclose(fp);
			return CVI_FAILURE;
		}

		if (phyMesh_old != phyMesh && virMesh_old != virMesh && (phyMesh_old != DEFAULT_MESH_PADDR))
			CVI_SYS_IonFree(phyMesh_old, virMesh_old);
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		fclose(fp);
		return CVI_ERR_GDC_NOT_SUPPORT;
	}
	fclose(fp);
	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_LoadMeshWithBuf(MESH_DUMP_ATTR_S *pMeshDumpAttr, const LDC_ATTR_S *pstLDCAttr, CVI_VOID *pBuf, CVI_U32 Len)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pMeshDumpAttr);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstLDCAttr);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pBuf);

	CVI_U64 phyMesh = 0;
	CVI_U64 phyMesh_old = 0;
	CVI_VOID *virMesh = NULL;
	CVI_VOID *virMesh_old = NULL;
	CVI_U32 VpssGrp = 0, VpssChn = 0, ViChn = 0;
	SIZE_S in_size, out_size;
	CVI_U32 mesh_1st_size, mesh_2nd_size, mesh_size;
	CVI_U32 u32Width, u32Height;
	struct cvi_gdc_mesh *pmesh = NULL;
	CVI_U32 u32ModFd;
	MOD_ID_E mod = pMeshDumpAttr->enModId;
	CVI_S32 s32Ret;
	VPSS_CHN_ATTR_S stVPSSChnAttr;
	VI_CHN_ATTR_S stVIChnAttr;
	MSG_PRIV_DATA_S stPrivData;
	VI_LDC_ATTR_S stVILDCAttr;
	VPSS_LDC_ATTR_S stVPSSLDCAttr;

	switch (mod) {
	case CVI_ID_VI:
		ViChn = pMeshDumpAttr->viMeshAttr.chn;
		pmesh = &g_vi_mesh[ViChn];
		s32Ret = CVI_VI_GetChnAttr(0, ViChn, &stVIChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "Get chn attr fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", 0, ViChn, s32Ret);
			return s32Ret;
		}

		u32Width = stVIChnAttr.stSize.u32Width;
		u32Height = stVIChnAttr.stSize.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		break;
	case CVI_ID_VPSS:
		VpssGrp = pMeshDumpAttr->vpssMeshAttr.grp;
		VpssChn = pMeshDumpAttr->vpssMeshAttr.chn;
		pmesh = &mesh[VpssGrp][VpssChn];

		s32Ret = CVI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stVPSSChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "Grp(%d) Chn(%d) Get chn attr fail\n",
				VpssGrp, VpssChn);
			return s32Ret;
		}

		u32Width = stVPSSChnAttr.u32Width;
		u32Height = stVPSSChnAttr.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	out_size.u32Width = in_size.u32Width;
	out_size.u32Height = in_size.u32Height;

	mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
	mesh_size = mesh_1st_size + mesh_2nd_size;
	if (mesh_size != Len) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "invalid param, Len[%d] not match with MOD[%d] meshsize[%d]\n",
			      Len, mod, mesh_size);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	// acquire memory space for mesh.
	if (CVI_SYS_IonAlloc_Cached(&phyMesh, &virMesh, "gdc_mesh", mesh_size) != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "Can't acquire memory for gdc mesh.\n");
		return CVI_ERR_VPSS_NOMEM;
	}

	CVI_TRACE_GDC(CVI_DBG_DEBUG, "load mesh size:%d, mesh phy addr:%#"PRIx64", vir addr:%p.\n",
		      mesh_size, phyMesh, virMesh);

	pthread_mutex_lock(&pmesh->lock);
	if (pmesh->paddr) {
		phyMesh_old = pmesh->paddr;
		virMesh_old = pmesh->vaddr;
	} else {
		phyMesh_old = 0;
		virMesh_old = NULL;
	}
	pmesh->paddr = phyMesh;
	pmesh->vaddr = virMesh;
	pthread_mutex_unlock(&pmesh->lock);

	memcpy(virMesh, pBuf, Len);
	CVI_SYS_IonFlushCache(phyMesh, virMesh, mesh_size);

	stPrivData.as32PrivData[0] = (CVI_S32)(phyMesh & 0xFFFFFFF);
	stPrivData.as32PrivData[1] = (CVI_S32)((phyMesh >> 28) & 0xFFFFFFF);
	stPrivData.as32PrivData[2] = (CVI_S32)((phyMesh >> 56) & 0xFF);

	switch (mod) {
	case CVI_ID_VI:
		g_vi_mesh[ViChn].meshSize = mesh_size;
		stVILDCAttr.bEnable = CVI_TRUE;
		memcpy(&stVILDCAttr.stAttr, pstLDCAttr, sizeof(*pstLDCAttr));
		u32ModFd = MODFD(CVI_ID_VI, 0, ViChn);
		s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VI_SET_CHN_LDC_ATTR, (CVI_VOID *)&stVILDCAttr,
					   sizeof(VI_LDC_ATTR_S), &stPrivData, -1);

		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "VI Set Chn(%d) LDC fail\n", ViChn);
			return CVI_FAILURE;
		}
		break;
	case CVI_ID_VPSS:
		mesh[VpssGrp][VpssChn].meshSize = mesh_size;
		stVPSSLDCAttr.bEnable = CVI_TRUE;
		memcpy(&stVPSSLDCAttr.stAttr, pstLDCAttr, sizeof(*pstLDCAttr));
		u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);
		s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VPSS_SET_CHN_LDCATTR, (CVI_VOID *)&stVPSSLDCAttr,
					   sizeof(VPSS_LDC_ATTR_S), &stPrivData, 30000);

		if (s32Ret != CVI_SUCCESS) {
			if (phyMesh_old != phyMesh && virMesh_old != virMesh && (phyMesh != DEFAULT_MESH_PADDR))
				CVI_SYS_IonFree(phyMesh, virMesh);
			pthread_mutex_lock(&pmesh->lock);
			pmesh->paddr = phyMesh_old;
			pmesh->vaddr = virMesh_old;
			pthread_mutex_unlock(&pmesh->lock);
			CVI_TRACE_GDC(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d Ldc_attr fail ret:%d\n", VpssGrp,
				      VpssChn, s32Ret);
			return CVI_FAILURE;
		}

		if (phyMesh_old != phyMesh && virMesh_old != virMesh && (phyMesh_old != DEFAULT_MESH_PADDR))
			CVI_SYS_IonFree(phyMesh_old, virMesh_old);
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	return CVI_SUCCESS;
}
