#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "cvi_type.h"
#include "cvi_ipcmsg.h"
#include "cvi_msg.h"
#include "msg_client.h"
#include "cvi_common.h"
#include "cvi_sensor.h"
#include <inttypes.h>
#include "cvi_debug.h"
#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

/*
#define CVI_TRACE_MSG(CVI_DBG_ERR, fmt, ...)  \
		printf("%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
*/

extern CVI_S32 MSG_SENSOR_SetAHDEnable(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg);
static CVI_S32 g_mMediaMsgId = -1;

static void MEDIA_MSG_HandleMessage(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_U32 u32ModID;
	CVI_IPCMSG_MESSAGE_S *respMsg;

	u32ModID = GET_MOD_ID(pstMsg->u32Module);
	switch (u32ModID) {
		case CVI_ID_SENSOR:
			MSG_SENSOR_SetAHDEnable(s32Id, pstMsg);
			break;
		default:

			respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, 0, NULL, 0);
			if (respMsg == CVI_NULL) {
				CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
				break;
			}
			CVI_IPCMSG_SendOnly(s32Id, respMsg);
			CVI_IPCMSG_DestroyMessage(respMsg);
			break;
	}
	CVI_TRACE_MSG(CVI_DBG_INFO, "s32Id=%d msg_id=%llu, u32Module=%d cmd=%d len=%d\n",
		s32Id, pstMsg->u64Id,
		pstMsg->u32Module, pstMsg->u32CMD, pstMsg->u32BodyLen);
}

CVI_S32 CVI_MSG_Init(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_CONNECT_S stConnectAttr = { 1, CVI_IPCMSG_MEDIA_PORT, 1 };

	if (g_mMediaMsgId >= 0)
		return CVI_SUCCESS;

	s32Ret = CVI_IPCMSG_AddService("CVI_MMF_MSG", &stConnectAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "client ipcmsg add service fail\n");
		return s32Ret;
	}

	/* connect with block to prevent tryconnect fail */
	s32Ret = CVI_IPCMSG_Connect(&g_mMediaMsgId, "CVI_MMF_MSG", MEDIA_MSG_HandleMessage);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Connect fail\n");
		return s32Ret;
	}

	CVI_IPCMSG_Run(g_mMediaMsgId);

	return 0;
}

CVI_S32 CVI_MSG_Deinit(CVI_VOID)
{
	CVI_S32 s32Ret;

	if (g_mMediaMsgId < 0)
		return CVI_SUCCESS;

	s32Ret = CVI_IPCMSG_Disconnect(g_mMediaMsgId);
	CVI_TRACE_MSG(CVI_DBG_INFO, "CVI_IPCMSG_Disconnect\n");

	s32Ret |= CVI_IPCMSG_DelService("CVI_MMF_MSG");

	g_mMediaMsgId = -1;

	CVI_TRACE_MSG(CVI_DBG_INFO, "Media_MSG_DeInit\n");
	return s32Ret;
}

CVI_S32 CVI_MSG_SendSync(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
					 MSG_PRIV_DATA_S *pstPrivData)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;
	CVI_IPCMSG_MESSAGE_S *pResp = NULL;

	if (g_mMediaMsgId < 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Message not initialized.\n");
		return CVI_FAILURE;
	}

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}
	if (pstPrivData != NULL) {
		memcpy(pReq->as32PrivData, pstPrivData->as32PrivData, sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
	}
	s32Ret = CVI_IPCMSG_SendSync(g_mMediaMsgId, pReq, &pResp, CVI_IPCMSG_SEND_SYNC_TIMEOUT);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		CVI_IPCMSG_DestroyMessage(pResp);
		return s32Ret;
	}
	s32Ret = pResp->s32RetVal;
	if (s32Ret == CVI_SUCCESS && (pResp->u32BodyLen > 0)) {
		memcpy(pBody, pResp->pBody, pResp->u32BodyLen);

		if (pstPrivData != NULL) {
			memcpy(pstPrivData->as32PrivData, pResp->as32PrivData,
				sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
		}
	}
	CVI_IPCMSG_DestroyMessage(pReq);
	CVI_IPCMSG_DestroyMessage(pResp);

	return s32Ret;
}

CVI_S32 CVI_MSG_SendSync2(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
					CVI_VOID *pRespBody, MSG_PRIV_DATA_S *pstPrivData)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;
	CVI_IPCMSG_MESSAGE_S *pResp = NULL;

	if (g_mMediaMsgId < 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Message not initialized.\n");
		return CVI_FAILURE;
	}

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}
	if (pstPrivData != NULL) {
		memcpy(pReq->as32PrivData, pstPrivData->as32PrivData, sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
	}
	s32Ret = CVI_IPCMSG_SendSync(g_mMediaMsgId, pReq, &pResp, CVI_IPCMSG_SEND_SYNC_TIMEOUT);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		CVI_IPCMSG_DestroyMessage(pResp);
		return s32Ret;
	}

	s32Ret = pResp->s32RetVal;
	if (s32Ret == CVI_SUCCESS && (pResp->u32BodyLen > 0)) {
		memcpy(pRespBody, pResp->pBody, pResp->u32BodyLen);

		if (pstPrivData != NULL) {
			memcpy(pstPrivData->as32PrivData, pResp->as32PrivData,
				sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
		}
	}
	CVI_IPCMSG_DestroyMessage(pReq);
	CVI_IPCMSG_DestroyMessage(pResp);

	return s32Ret;
}

CVI_S32 CVI_MSG_SendSync3(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
					CVI_U32 *pu32Data)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;
	CVI_IPCMSG_MESSAGE_S *pResp = NULL;

	if (g_mMediaMsgId < 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Message not initialized.\n");
		return CVI_FAILURE;
	}

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}

	s32Ret = CVI_IPCMSG_SendSync(g_mMediaMsgId, pReq, &pResp, CVI_IPCMSG_SEND_SYNC_TIMEOUT);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		CVI_IPCMSG_DestroyMessage(pResp);
		return s32Ret;
	}
	s32Ret = pResp->s32RetVal;
	if ((s32Ret == CVI_SUCCESS) && (pu32Data != NULL)) {
		*pu32Data = pResp->as32PrivData[0];
	}
	CVI_IPCMSG_DestroyMessage(pReq);
	CVI_IPCMSG_DestroyMessage(pResp);

	return s32Ret;
}

CVI_S32 CVI_MSG_SendSync4(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
			MSG_PRIV_DATA_S *pstPrivData, CVI_S32 s32TimeoutMs)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;
	CVI_IPCMSG_MESSAGE_S *pResp = NULL;

	if (g_mMediaMsgId < 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Message not initialized.\n");
		return CVI_FAILURE;
	}

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}
	if (pstPrivData != NULL) {
		memcpy(pReq->as32PrivData, pstPrivData->as32PrivData, sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
	}
	s32Ret = CVI_IPCMSG_SendSync(g_mMediaMsgId, pReq, &pResp, s32TimeoutMs);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		CVI_IPCMSG_DestroyMessage(pResp);
		return s32Ret;
	}
	s32Ret = pResp->s32RetVal;
	if (s32Ret == CVI_SUCCESS && (pResp->u32BodyLen > 0)) {
		memcpy(pBody, pResp->pBody, pResp->u32BodyLen);

		if (pstPrivData != NULL) {
			memcpy(pstPrivData->as32PrivData, pResp->as32PrivData,
				sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
		}
	}
	CVI_IPCMSG_DestroyMessage(pReq);
	CVI_IPCMSG_DestroyMessage(pResp);

	return s32Ret;
}

CVI_S32 CVI_MSG_SendSync5(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
					 MSG_PRIV_DATA_S *pstPrivData)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;
	CVI_IPCMSG_MESSAGE_S *pResp = NULL;

	if (g_mMediaMsgId < 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Message not initialized.\n");
		return CVI_FAILURE;
	}

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}
	if (pstPrivData != NULL) {
		memcpy(pReq->as32PrivData, pstPrivData->as32PrivData, sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
	}
	s32Ret = CVI_IPCMSG_SendSync(g_mMediaMsgId, pReq, &pResp, CVI_IPCMSG_SEND_SYNC_TIMEOUT);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		CVI_IPCMSG_DestroyMessage(pResp);
		return s32Ret;
	}
	s32Ret = pResp->s32RetVal;
	if (s32Ret == CVI_SUCCESS) {
		if (pResp->u32BodyLen > 0)
			memcpy(pBody, pResp->pBody, pResp->u32BodyLen);

		if (pstPrivData != NULL) {
			memcpy(pstPrivData->as32PrivData, pResp->as32PrivData,
				sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
		}
	}
	CVI_IPCMSG_DestroyMessage(pReq);
	CVI_IPCMSG_DestroyMessage(pResp);

	return s32Ret;
}