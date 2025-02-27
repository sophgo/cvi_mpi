#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include "sample_comm.h"


CVI_S32 SAMPLE_COMM_IPCMSG_Init(CVI_CHAR *pszServiceName, CVI_IPCMSG_CONNECT_S *pstConnectAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_IPCMSG_AddService(pszServiceName, pstConnectAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("client ipcmsg add service failed with:%d\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_IPCMSG_Deinit(CVI_CHAR *pszServiceName, CVI_S32 s32Id)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_IPCMSG_Disconnect(s32Id);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_IPCMSG_Disconnect failed with:%d\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_IPCMSG_DelService(pszServiceName);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("client ipcmsg del service failed with:%d\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_IPCMSG_SendSync(CVI_S32 s32Id, CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;
	CVI_IPCMSG_MESSAGE_S *pResp = NULL;
	CVI_S32 as32PrivData[CVI_IPCMSG_PRIVDATA_NUM] = {0};

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		SAMPLE_PRT("CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}

	memcpy(pReq->as32PrivData, as32PrivData, sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);

	s32Ret = CVI_IPCMSG_SendSync(s32Id, pReq, &pResp, 3000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_IPCMSG_SendSync failed with:%d\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		CVI_IPCMSG_DestroyMessage(pResp);
		return s32Ret;
	}
	s32Ret = pResp->s32RetVal;
	if (s32Ret == CVI_SUCCESS && (pResp->u32BodyLen > 0)) {
		memcpy(pBody, pResp->pBody, pResp->u32BodyLen);

		memcpy(as32PrivData, pResp->as32PrivData, sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
	}
	CVI_IPCMSG_DestroyMessage(pReq);
	CVI_IPCMSG_DestroyMessage(pResp);

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_IPCMSG_SendAsync(CVI_S32 s32Id, CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen, void (*respHandler)(CVI_IPCMSG_MESSAGE_S*))
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		SAMPLE_PRT("CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}

	s32Ret = CVI_IPCMSG_SendAsync(s32Id, pReq, respHandler);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_IPCMSG_SendAsync failed with:%d\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(pReq);

	return s32Ret;

}

CVI_S32 SAMPLE_COMM_IPCMSG_SendOnly(CVI_S32 s32Id, CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		SAMPLE_PRT("CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}

	s32Ret = CVI_IPCMSG_SendOnly(s32Id, pReq);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_IPCMSG_SendOnly failed with:%d\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(pReq);

	return s32Ret;

}
