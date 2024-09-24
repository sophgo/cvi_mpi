/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 *
 * File Name: cvi_ispd2_callback_funcs_vpss.c
 * Description:
 */

#include "cvi_ispd2_callback_funcs_local.h"

#include "cvi_vpss.h"

// -----------------------------------------------------------------------------
CVI_S32 CVI_ISPD2_CBFunc_VPSS_SetChnLDCAttr(TJSONRpcContentIn *ptContentIn,
	TJSONRpcContentOut *ptContentOut, JSONObject *pJsonResponse)
{
	TISPDeviceInfo	*ptDevInfo = ptContentIn->ptDeviceInfo;

	VPSS_LDC_ATTR_S	stAttr;
	CVI_S32			s32Ret;
	JSONObject		*pstJsonObject = NULL;

	ptContentOut->s32StatusCode = JSONRPC_CODE_OK;

	if (CVI_VPSS_GetChnLDCAttr(ptDevInfo->s32VpssGrp, ptDevInfo->s32VpssChn, &stAttr) != CVI_SUCCESS) {
		CVI_ISPD2_Utils_ComposeAPIErrorMessage(ptContentOut);
		return CVI_FAILURE;
	}

	s32Ret = CVI_SUCCESS;
	GET_INT_FROM_JSON(ptContentIn->pParams, "/bEnable", stAttr.bEnable, s32Ret);
	ISPD2_json_pointer_get(ptContentIn->pParams, "/stAttr", &pstJsonObject);
	GET_INT_FROM_JSON(pstJsonObject, "/bAspect", stAttr.stAttr.bAspect, s32Ret);
	GET_INT_FROM_JSON(pstJsonObject, "/s32XRatio", stAttr.stAttr.s32XRatio, s32Ret);
	GET_INT_FROM_JSON(pstJsonObject, "/s32YRatio", stAttr.stAttr.s32YRatio, s32Ret);
	GET_INT_FROM_JSON(pstJsonObject, "/s32XYRatio", stAttr.stAttr.s32XYRatio, s32Ret);
	GET_INT_FROM_JSON(pstJsonObject, "/s32CenterXOffset", stAttr.stAttr.s32CenterXOffset, s32Ret);
	GET_INT_FROM_JSON(pstJsonObject, "/s32CenterYOffset", stAttr.stAttr.s32CenterYOffset, s32Ret);
	GET_INT_FROM_JSON(pstJsonObject, "/s32DistortionRatio", stAttr.stAttr.s32DistortionRatio, s32Ret);

	if (CVI_VPSS_SetChnLDCAttr(ptDevInfo->s32VpssGrp, ptDevInfo->s32VpssChn, &stAttr) != CVI_SUCCESS) {
		CVI_ISPD2_Utils_ComposeAPIErrorMessage(ptContentOut);
		return CVI_FAILURE;
	}

	UNUSED(pJsonResponse);
	UNUSED(s32Ret);

	return CVI_SUCCESS;
}

// -----------------------------------------------------------------------------
CVI_S32 CVI_ISPD2_CBFunc_VPSS_GetChnLDCAttr(TJSONRpcContentIn *ptContentIn,
	TJSONRpcContentOut *ptContentOut, JSONObject *pJsonResponse)
{
	TISPDeviceInfo	*ptDevInfo = ptContentIn->ptDeviceInfo;
	VPSS_LDC_ATTR_S	stAttr;

	if (CVI_VPSS_GetChnLDCAttr(ptDevInfo->s32VpssGrp, ptDevInfo->s32VpssChn, &stAttr) != CVI_SUCCESS) {
		CVI_ISPD2_Utils_ComposeAPIErrorMessage(ptContentOut);
		return CVI_FAILURE;
	}

	JSONObject *pDataOut = ISPD2_json_object_new_object();
	JSONObject *pLdcAttrObject = ISPD2_json_object_new_object();

	SET_INT_TO_JSON("bEnable", stAttr.bEnable, pDataOut);
	SET_INT_TO_JSON("bAspect", stAttr.stAttr.bAspect, pLdcAttrObject);
	SET_INT_TO_JSON("s32XRatio", stAttr.stAttr.s32XRatio, pLdcAttrObject);
	SET_INT_TO_JSON("s32YRatio", stAttr.stAttr.s32YRatio, pLdcAttrObject);
	SET_INT_TO_JSON("s32XYRatio", stAttr.stAttr.s32XYRatio, pLdcAttrObject);
	SET_INT_TO_JSON("s32CenterXOffset", stAttr.stAttr.s32CenterXOffset, pLdcAttrObject);
	SET_INT_TO_JSON("s32CenterYOffset", stAttr.stAttr.s32CenterYOffset, pLdcAttrObject);
	SET_INT_TO_JSON("s32DistortionRatio", stAttr.stAttr.s32DistortionRatio, pLdcAttrObject);

	ISPD2_json_object_object_add(pDataOut, "stAttr", pLdcAttrObject);
	ISPD2_json_object_object_add(pJsonResponse, GET_RESPONSE_KEY_NAME, pDataOut);

	ptContentOut->s32StatusCode = JSONRPC_CODE_OK;

	UNUSED(ptContentIn);

	return CVI_SUCCESS;
}

// -----------------------------------------------------------------------------
CVI_S32 CVI_ISPD2_CBFunc_VPSS_SetGrpProcAmp(TJSONRpcContentIn *ptContentIn,
	TJSONRpcContentOut *ptContentOut, JSONObject *pJsonResponse)
{
	CVI_S32 s32Ret;
	CVI_S32 as32Brightness[VPSS_MAX_GRP_NUM] = {0};
	CVI_S32 as32Contrast[VPSS_MAX_GRP_NUM] = {0};
	CVI_S32 as32Saturation[VPSS_MAX_GRP_NUM] = {0};
	CVI_S32 as32Hue[VPSS_MAX_GRP_NUM] = {0};
	CVI_S32 VpssGrp = ptContentIn->ptDeviceInfo->s32VpssGrp;

	ptContentOut->s32StatusCode = JSONRPC_CODE_OK;

	CVI_VPSS_GetGrpProcAmp(VpssGrp, PROC_AMP_BRIGHTNESS, &(as32Brightness[VpssGrp]));
	CVI_VPSS_GetGrpProcAmp(VpssGrp, PROC_AMP_CONTRAST, &(as32Contrast[VpssGrp]));
	CVI_VPSS_GetGrpProcAmp(VpssGrp, PROC_AMP_SATURATION, &(as32Saturation[VpssGrp]));
	CVI_VPSS_GetGrpProcAmp(VpssGrp, PROC_AMP_HUE, &(as32Hue[VpssGrp]));

	s32Ret = CVI_SUCCESS;
	GET_INT_ARRAY_FROM_JSON(ptContentIn->pParams, "/brightness", VPSS_MAX_GRP_NUM, CVI_S32, as32Brightness, s32Ret);
	GET_INT_ARRAY_FROM_JSON(ptContentIn->pParams, "/contrast", VPSS_MAX_GRP_NUM, CVI_S32, as32Contrast, s32Ret);
	GET_INT_ARRAY_FROM_JSON(ptContentIn->pParams, "/saturation", VPSS_MAX_GRP_NUM, CVI_S32, as32Saturation, s32Ret);
	GET_INT_ARRAY_FROM_JSON(ptContentIn->pParams, "/hue", VPSS_MAX_GRP_NUM, CVI_S32, as32Hue, s32Ret);

	CVI_VPSS_SetGrpProcAmp(VpssGrp, PROC_AMP_BRIGHTNESS, as32Brightness[VpssGrp]);
	CVI_VPSS_SetGrpProcAmp(VpssGrp, PROC_AMP_CONTRAST, as32Contrast[VpssGrp]);
	CVI_VPSS_SetGrpProcAmp(VpssGrp, PROC_AMP_SATURATION, as32Saturation[VpssGrp]);
	CVI_VPSS_SetGrpProcAmp(VpssGrp, PROC_AMP_HUE, as32Hue[VpssGrp]);


	UNUSED(pJsonResponse);
	UNUSED(s32Ret);

	return CVI_SUCCESS;
}

// -----------------------------------------------------------------------------
CVI_S32 CVI_ISPD2_CBFunc_VPSS_GetGrpProcAmp(TJSONRpcContentIn *ptContentIn,
	TJSONRpcContentOut *ptContentOut, JSONObject *pJsonResponse)
{
	CVI_S32 as32Brightness[VPSS_MAX_GRP_NUM] = {0};
	CVI_S32 as32Contrast[VPSS_MAX_GRP_NUM] = {0};
	CVI_S32 as32Saturation[VPSS_MAX_GRP_NUM] = {0};
	CVI_S32 as32Hue[VPSS_MAX_GRP_NUM] = {0};
	CVI_S32 VpssGrp = ptContentIn->ptDeviceInfo->s32VpssGrp;

	CVI_VPSS_GetGrpProcAmp(VpssGrp, PROC_AMP_BRIGHTNESS, &(as32Brightness[VpssGrp]));
	CVI_VPSS_GetGrpProcAmp(VpssGrp, PROC_AMP_CONTRAST, &(as32Contrast[VpssGrp]));
	CVI_VPSS_GetGrpProcAmp(VpssGrp, PROC_AMP_SATURATION, &(as32Saturation[VpssGrp]));
	CVI_VPSS_GetGrpProcAmp(VpssGrp, PROC_AMP_HUE, &(as32Hue[VpssGrp]));

	JSONObject *pDataOut = ISPD2_json_object_new_object();

	SET_INT_ARRAY_TO_JSON("brightness", VPSS_MAX_GRP_NUM, as32Brightness, pDataOut);
	SET_INT_ARRAY_TO_JSON("contrast", VPSS_MAX_GRP_NUM, as32Contrast, pDataOut);
	SET_INT_ARRAY_TO_JSON("saturation", VPSS_MAX_GRP_NUM, as32Saturation, pDataOut);
	SET_INT_ARRAY_TO_JSON("hue", VPSS_MAX_GRP_NUM, as32Hue, pDataOut);

	ISPD2_json_object_object_add(pJsonResponse, GET_RESPONSE_KEY_NAME, pDataOut);

	ptContentOut->s32StatusCode = JSONRPC_CODE_OK;

	UNUSED(ptContentIn);

	return CVI_SUCCESS;
}

// -----------------------------------------------------------------------------
