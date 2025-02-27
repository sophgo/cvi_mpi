/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_vpss.c
 * Description:
 *	 MMF Programe Interface for video processing moudle
 */

#include <stdio.h>
#include <stdlib.h>
#include "cvi_type.h"
#include "cvi_vpss.h"
#include "cvi_gdc.h"
#include "cvi_sys_base.h"
#include "cvi_debug.h"
#include "cvi_msg.h"
#include "msg_client.h"
#include "cvi_buffer.h"
#include "vpss_bin.h"
#include "cvi_sys.h"

#define UNUSED_VARIABLE(x) ((void)(x))

struct cvi_gdc_mesh mesh[VPSS_MAX_GRP_NUM][VPSS_MAX_CHN_NUM];

CVI_S32 CVI_VPSS_Suspend(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SUSPEND, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "CVI_VPSS_Suspend fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_Resume(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_RESUME, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "CVI_VPSS_Resume fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_CreateGrp(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstGrpAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_CREATE, (CVI_VOID *)pstGrpAttr,
				sizeof(VPSS_GRP_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Create group fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_DestroyGrp(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_DESTROY, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Destroy group fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

VPSS_GRP CVI_VPSS_GetAvailableGrp(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_AVAILABLE_GRP, CVI_NULL, 0, CVI_NULL);

	return s32Ret;
}

CVI_S32 CVI_VPSS_StartGrp(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_START, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Start group fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_StopGrp(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_STOP, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Stop group fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_ResetGrp(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_RESET, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Reset group fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetGrpAttr(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstGrpAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstGrpAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_GRP_ATTR, pstGrpAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get grp attr fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetGrpAttr(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstGrpAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_GRP_ATTR, (CVI_VOID *)pstGrpAttr,
				sizeof(VPSS_GRP_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set grp attr fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetGrpCrop(VPSS_GRP VpssGrp, const VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_GRP_CROP, (CVI_VOID *)pstCropInfo,
				sizeof(VPSS_CROP_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set grp crop fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetGrpCrop(VPSS_GRP VpssGrp, VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_GRP_CROP, pstCropInfo, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get grp crop fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SendFrame(VPSS_GRP VpssGrp, const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);

	stPrivData.as32PrivData[0] = s32MilliSec;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SEND_FRAME, (CVI_VOID *)pstVideoFrame,
				sizeof(VIDEO_FRAME_INFO_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Send chn frame fail,VpssGrp:%d, s32Ret:%x\n",
			VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetGrpProcAmpCtrl(VPSS_GRP VpssGrp, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	stPrivData.as32PrivData[0] = type;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_GRP_PROCAMPCTRL, (CVI_VOID *)ctrl, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get Grp ProcAmpCtrl fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetGrpProcAmp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 *value)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	stPrivData.as32PrivData[0] = type;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_GRP_PROCAMP, value, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get VpssGrp:%d ProcAmp fail ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetAllProcAmp(struct vpss_all_proc_amp_cfg *cfg)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_ALL_GRP_PROCAMP, cfg, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get AllGrpProcAmp fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetGrpProcAmp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 value)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	stPrivData.as32PrivData[0] = type;
	stPrivData.as32PrivData[1] = value;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_GRP_PROCAMP, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d ProcAmp fail ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetGrpParamfromBin(VPSS_GRP VpssGrp, CVI_U8 scene)
{
	CVI_S32 s32Ret;
	VPSS_BIN_DATA *pBinData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);
	struct vpss_proc_amp_cfg bin_cfg = {0};

	if (get_loadbin_state()) {
		pBinData = get_vpssbindata_addr();
		bin_cfg.VpssGrp = VpssGrp;
		bin_cfg.scene = scene;
		memcpy(bin_cfg.proc_amp, pBinData[scene].proc_amp, sizeof(bin_cfg.proc_amp));

		s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_GRP_PQBIN, (CVI_VOID *)&bin_cfg,
					sizeof(struct vpss_proc_amp_cfg), CVI_NULL);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VPSS(CVI_DBG_ERR, "Set GRP PQBin fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
			return s32Ret;
		}
		CVI_TRACE_VPSS(CVI_DBG_INFO, "PqBin is exist, vpss grp param use pqbin value in Linux !!\n");
	} else {
		CVI_TRACE_VPSS(CVI_DBG_INFO, "PqBin is not find, vpss grp param use default !!\n");
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetBinScene(VPSS_GRP VpssGrp, CVI_U8 *scene)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_GRP_SCENE, scene, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get BinScene fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}


CVI_S32 CVI_VPSS_SetChnAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstChnAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_ATTR, (CVI_VOID *)pstChnAttr,
				sizeof(VPSS_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d attr fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_ATTR, pstChnAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn attr fail,VpssGrp:%d Chn:%d,s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_EnableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_ENABLE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Enable Grp:%d Chn:%d Fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_DisableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);
	struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];

	if (pmesh->paddr && (pmesh->paddr != DEFAULT_MESH_PADDR)) {
		CVI_SYS_IonFree(pmesh->paddr, pmesh->vaddr);
		pmesh->paddr = 0;
		pmesh->vaddr = NULL;
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_DISABLE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Disable Grp:%d Chn:%d Fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnCrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_CROP, (CVI_VOID *)pstCropInfo,
				sizeof(VPSS_CROP_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d Crop fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnCrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_CROP, pstCropInfo, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get VpssGrp:%d Chn:%d Crop fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnRotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E enRotation)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];

	pmesh->paddr = DEFAULT_MESH_PADDR;
	stPrivData.as32PrivData[0] = enRotation;

	s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VPSS_SET_CHN_ROTATION, CVI_NULL, 0, &stPrivData, 30000);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d ChnRotation fail ret:%d\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnRotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E *penRotation)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_ROTATION, penRotation, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get VpssGrp:%d Chn:%d ChnRotation fail ret:%d\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnLDCAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);
	VPSS_CHN_ATTR_S stChnAttr;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U64 paddr = 0;
	CVI_U64 paddr_old = 0;
	CVI_VOID *vaddr = NULL;
	CVI_VOID *vaddr_old = NULL;
	struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstLDCAttr);

	s32Ret = CVI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) Get chn attr fail\n",
			       VpssGrp, VpssChn);
		return s32Ret;
	}

	if (pstLDCAttr->bEnable) {
		s32Ret = CVI_GDC_GenLDCMesh(stChnAttr.u32Width, stChnAttr.u32Height, &pstLDCAttr->stAttr,
					    "vpss_mesh", &paddr, &vaddr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) gen mesh fail\n",
				       VpssGrp, VpssChn);
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

	s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VPSS_SET_CHN_LDCATTR, (CVI_VOID *)pstLDCAttr,
				   sizeof(VPSS_LDC_ATTR_S), &stPrivData, 30000);

	if (s32Ret != CVI_SUCCESS) {
		if (paddr_old != paddr && vaddr_old != vaddr && (paddr != DEFAULT_MESH_PADDR))
			CVI_SYS_IonFree(paddr, vaddr);
		pthread_mutex_lock(&pmesh->lock);
		pmesh->paddr = paddr_old;
		pmesh->vaddr = vaddr_old;
		pthread_mutex_unlock(&pmesh->lock);
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d Ldc_attr fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	if (paddr_old != paddr && vaddr_old != vaddr && (paddr_old != DEFAULT_MESH_PADDR))
		CVI_SYS_IonFree(paddr_old, vaddr_old);

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnLDCAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_LDCATTR, pstLDCAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get grp attr fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SendChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);

	stPrivData.as32PrivData[0] = s32MilliSec;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SEND_CHN_FRAME, (CVI_VOID *)pstVideoFrame,
				sizeof(VIDEO_FRAME_INFO_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Send chn frame fail,VpssGrp:%d, VpssChn:%d, s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S *pstVideoFrame,
				 CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VPSS, VpssGrp, VpssChn, 1);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);

	stPrivData.as32PrivData[0] = s32MilliSec;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_FRAME, (CVI_VOID *)pstVideoFrame,
				0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn frame fail,VpssGrp:%d, VpssChn:%d, s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_ReleaseChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_RELEASE_CHN_FRAME, (CVI_VOID *)pstVideoFrame,
				sizeof(VIDEO_FRAME_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Release chn frame fail,VpssGrp:%d, VpssChn:%d, s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_AttachVbPool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VB_POOL hVbPool)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	stPrivData.as32PrivData[0] = hVbPool;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_ATTACH_VBPOOL, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssAttachVbPool Grp:%d Chn:%d fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_DetachVbPool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_DETACH_VBPOOL, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssDetachVbPool Grp:%d Chn:%d fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnAlign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32Align)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	stPrivData.as32PrivData[0] = u32Align;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_ALIGN, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssSetChnAlign Grp:%d Chn:%d fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}
CVI_S32 CVI_VPSS_GetChnAlign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 *pu32Align)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_ALIGN, pu32Align, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn Align fail,VpssGrp:%d chn:%d s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnYRatio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT YRatio)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_YRATIO, (CVI_VOID *)&YRatio,
		sizeof(CVI_FLOAT), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssSetChnYRatio Grp:%d Chn:%d fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}
CVI_S32 CVI_VPSS_GetChnYRatio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT *pYRatio)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_YRATIO, pYRatio, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn YRatio fail,VpssGrp:%d chn:%d s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}


CVI_S32 CVI_VPSS_SetChnScaleCoefLevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E enCoef)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_SCALECOEF, (CVI_VOID *)&enCoef,
		sizeof(VPSS_SCALE_COEF_E), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssSetChnScaleCoefLevel Grp:%d Chn:%d fail ret:%d\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnScaleCoefLevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E *penCoef)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_SCALECOEF, penCoef, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn ScaleCoefLevel fail,VpssGrp:%d chn:%d s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_ShowChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SHOW_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssShowChn Grp:%d Chn:%d fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_HideChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_HIDE_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssHideChn Grp:%d Chn:%d fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnFd(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	UNUSED_VARIABLE(VpssGrp);
	UNUSED_VARIABLE(VpssChn);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_CloseFd(void)
{
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetRegionLuma(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_REGION_INFO_S *pstRegionInfo,
								CVI_U64 *pu64LumaData, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	CVI_U32 msg_len;
	CVI_VOID *sMsg;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstRegionInfo);

	if (pstRegionInfo->u32RegionNum == 0) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "region num(%d) invalid.\n"
					, pstRegionInfo->u32RegionNum);
		return CVI_FAILURE;
	}

	msg_len = sizeof(VIDEO_REGION_INFO_S) +
			pstRegionInfo->u32RegionNum * sizeof(RECT_S) +
			pstRegionInfo->u32RegionNum * sizeof(CVI_U64);

	sMsg = calloc(msg_len, 1);
	if (!sMsg) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Region luma msg calloc failed, size(%d)\n", msg_len);
		return CVI_FAILURE;
	}

	stPrivData.as32PrivData[0] = s32MilliSec;
	memcpy(sMsg, (CVI_VOID *)pstRegionInfo, sizeof(VIDEO_REGION_INFO_S));
	memcpy(sMsg + sizeof(VIDEO_REGION_INFO_S), pstRegionInfo->pstRegion,
			pstRegionInfo->u32RegionNum * sizeof(RECT_S));

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_REGIONLUMA, sMsg, msg_len, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get RegionLuma fail,VpssGrp:%d Chn:%d,s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	memcpy(pu64LumaData,
		(CVI_U64 *)(sMsg + sizeof(VIDEO_REGION_INFO_S) + pstRegionInfo->u32RegionNum * sizeof(RECT_S)),
		sizeof(CVI_U64) * pstRegionInfo->u32RegionNum);

	free(sMsg);
	sMsg = NULL;

	return CVI_SUCCESS;
}
CVI_S32 CVI_VPSS_SetChnBufWrapAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVpssChnBufWrap);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_BUFWRAPATTR, (CVI_VOID *)pstVpssChnBufWrap,
				sizeof(VPSS_CHN_BUF_WRAP_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d BufWrapAttr fail ret:%d\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnBufWrapAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_BUFWRAPATTR, pstVpssChnBufWrap, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn BufWrapAttr fail,VpssGrp:%d Chn:%d,s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_U32 CVI_VPSS_GetWrapBufferSize(CVI_U32 u32Width, CVI_U32 u32Height, PIXEL_FORMAT_E enPixelFormat,
	CVI_U32 u32BufLine, CVI_U32 u32BufDepth)
{
	CVI_U32 u32BufSize;
	VB_CAL_CONFIG_S stCalConfig;

	if (u32Width < 64 || u32Height < 64) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "width(%d) or height(%d) too small\n", u32Width, u32Height);
		return 0;
	}
	if (u32BufLine != 64 && u32BufLine != 128) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "u32BufLine(%d) invalid, only 64 or 128 lines\n",
				u32BufLine);
		return 0;
	}
	if (u32BufDepth < 2 || u32BufDepth > 32) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "u32BufDepth(%d) invalid, 2 ~ 32\n",
				u32BufDepth);
		return 0;
	}

	COMMON_GetPicBufferConfig(u32Width, u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stCalConfig);

	u32BufSize = stCalConfig.u32VBSize / u32Height;
	u32BufSize *= u32BufLine * u32BufDepth;
	CVI_TRACE_VPSS(CVI_DBG_INFO, "width(%d), height(%d), u32BufSize=%d\n",
		   u32Width, u32Height, u32BufSize);

	return u32BufSize;
}

CVI_S32 CVI_VPSS_CreateStitch(VPSS_GRP VpssGrp, const CVI_STITCH_ATTR_S *pstStitchAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstStitchAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_CREATE_STITCH, (CVI_VOID *)pstStitchAttr,
		sizeof(CVI_STITCH_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "CreateStitch Grp:%d ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_DestroyStitch(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_DESTROY_STITCH, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "DestroyStitch Grp:%d ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetStitchAttr(VPSS_GRP VpssGrp, const CVI_STITCH_ATTR_S *pstStitchAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstStitchAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_STITCH, (CVI_VOID *)pstStitchAttr,
		sizeof(CVI_STITCH_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "SetStitchAttr Grp:%d ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetStitchAttr(VPSS_GRP VpssGrp, CVI_STITCH_ATTR_S *pstStitchAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstStitchAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_STITCH, pstStitchAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "GetStitchAttr Grp:%d ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_StartStitch(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_START_STITCH, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "StartStitch Grp:%d ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_StopStitch(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_STOP_STITCH, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "StopStitch Grp:%d ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_BindFb(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_BIND_FB, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "CVI_VPSS_BindFb Grp:%d Chn:%d fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_UnbindFb(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_UNBIND_FB, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "CVI_VPSS_UnbindFb Grp:%d Chn:%d fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_EnableTileMode()
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_ENABLE_TILEMODE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "EnableTileMode ret:%d\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_DisableTileMode()
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_DISABLE_TILEMODE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "DisableTileMode ret:%d\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}