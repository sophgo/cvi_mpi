#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include "sample_comm.h"

static CVI_BOOL s_bStopSend = CVI_FALSE;

void handle_resp(CVI_IPCMSG_MESSAGE_S *pResp)
{
	SAMPLE_PRT("receive async resp: %s\n", (CVI_CHAR *)pResp->pBody);
}

void handle_message(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *msg)
{

	(void)s32Id;
	switch (msg->u32Module)
	{
	case 2:
		handle_resp(msg);
		break;
	default:
		break;
	}

	return;
}


void *receive_thread(void *arg)
{
	CVI_S32 *pId = (CVI_S32 *)arg;

	CVI_IPCMSG_Run(*pId);
	SAMPLE_PRT("Run...\n");
	return NULL;
}

void *sendasync_thread(void *arg)
{
	CVI_S32 *ps32Id = (CVI_S32 *)arg;
	static CVI_U32 s_u32Index;
	CVI_CHAR content[32];
	CVI_S32 s32Ret = CVI_SUCCESS;

	while (s_bStopSend == CVI_FALSE) {
		memset(content, 0, 32);
		snprintf(content, 32, "async %u", s_u32Index++);
		SAMPLE_PRT("========%s========\n", content);
		s32Ret = SAMPLE_COMM_IPCMSG_SendAsync(*ps32Id, 2, 5, content, 32, handle_resp);
		if (s32Ret != 0) {
			SAMPLE_PRT("SAMPLE_COMM_IPCMSG_SendAsync failed with:%d\n", s32Ret);
			break;
		}

		sleep(2);
	}

	return NULL;
}

void *sendsync_thread(void *arg)
{
	CVI_S32 *ps32Id = (CVI_S32 *)arg;
	static CVI_U32 s_u32Index;
	CVI_CHAR content[32];
	CVI_S32 s32Ret = CVI_SUCCESS;

	while (s_bStopSend == CVI_FALSE) {
		memset(content, 0, 32);
		snprintf(content, 32, "sync %u", s_u32Index++);
		SAMPLE_PRT("========%s========\n", content);
		s32Ret = SAMPLE_COMM_IPCMSG_SendSync(*ps32Id, 1, 1, content, 32);
		if (s32Ret != 0) {
			SAMPLE_PRT("SAMPLE_COMM_IPCMSG_SendSync failed with:%d\n", s32Ret);
			break;
		} else {
			SAMPLE_PRT("receive sync resp: %s\n", content);
		}

		sleep(2);
	}

	return NULL;
}

int main(int argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_CHAR cmd[64];
	pthread_t receivePid;
	pthread_t sendPid0;
	pthread_t sendPid1;
	struct sched_param param;
	pthread_attr_t attr;
	CVI_S32 s32Id = -1;

	(void)argc;
	(void)argv;

	CVI_IPCMSG_CONNECT_S stConnectAttr = { 1, 201, 0 };

	s32Ret = SAMPLE_COMM_IPCMSG_Init("Test", &stConnectAttr);
	if (s32Ret != 0) {
		SAMPLE_PRT("SAMPLE_COMM_IPCMSG_Init failed with:%d\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_IPCMSG_Connect(&s32Id, "Test", handle_message);
	if (s32Ret != 0) {
		SAMPLE_PRT("CVI_IPCMSG_Connect failed with:%d\n", s32Ret);
		return s32Ret;
	}

	param.sched_priority = 45;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	if (pthread_create(&receivePid, &attr, receive_thread, &s32Id) != 0) {
		SAMPLE_PRT("pthread_create receive_thread fail\n");
		return -1;
	}
	sleep(1);

	if (pthread_create(&sendPid0, &attr, sendsync_thread, &s32Id) != 0) {
		SAMPLE_PRT("pthread_create fun fail\n");
		return -1;
	}
	sleep(1);

	if (pthread_create(&sendPid1, &attr, sendasync_thread, &s32Id) != 0) {
		SAMPLE_PRT("pthread_create fun fail\n");
		return -1;
	}

	while (strncmp(fgets(cmd, 64, stdin), "q", 1) != 0) {
		SAMPLE_PRT("Enter q to exit\n");
	}

	s_bStopSend = CVI_TRUE;
	pthread_join(sendPid0, NULL);
	pthread_join(sendPid1, NULL);
	pthread_join(receivePid, NULL);

	s32Ret = SAMPLE_COMM_IPCMSG_Deinit("Test", s32Id);
	if (s32Ret != 0) {
		SAMPLE_PRT("SAMPLE_COMM_IPCMSG_Deinit failed with:%d\n", s32Ret);
		return s32Ret;
	}

	return 0;
}
