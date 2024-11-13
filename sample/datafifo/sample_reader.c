#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include "sample_comm.h"

static const CVI_S32 BLOCK_LEN = 1024;
static CVI_DATAFIFO_HANDLE hDataFifo = CVI_DATAFIFO_INVALID_HANDLE;
static CVI_BOOL s_bStop = CVI_FALSE;

void *read_more(void *arg)
{
	CVI_CHAR *pBuf = NULL;
	CVI_S32 s32Ret = CVI_SUCCESS;

	(void)arg;

	while (CVI_FALSE == s_bStop) {
		s32Ret = SAMPLE_COMM_DATAFIFO_Read(hDataFifo, pBuf);
		if (s32Ret != 0) {
			SAMPLE_PRT("SAMPLE_COMM_DATAFIFO_Read failed with:%d\n", s32Ret);
			break;
		}
		usleep(40000);
	}

	return NULL;
}

int main(int argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_CHAR cmd[64];
	pthread_t readThread;
	CVI_U64 phyAddr;
	CVI_DATAFIFO_PARAMS_S params = {10, BLOCK_LEN, CVI_TRUE, DATAFIFO_READER};

	if (argc != 2) {
		SAMPLE_PRT("Usage: %s PhyAddr\n", argv[0]);
		return -1;
	}

	sscanf(argv[1], "%llu", &phyAddr);
	SAMPLE_PRT("phyAddr:%#llx\n", phyAddr);

	s32Ret = SAMPLE_COMM_DATAFIFO_Access(&params, phyAddr, &hDataFifo);
	if (s32Ret != 0) {
		SAMPLE_PRT("SAMPLE_COMM_DATAFIFO_Access failed with:%d\n", s32Ret);
		return s32Ret;
	}

	s_bStop = CVI_FALSE;
	pthread_create(&readThread, NULL, read_more, NULL);

	do {
		SAMPLE_PRT("Input q to exit: \n");
	} while ( 0 != strncmp(fgets(cmd, 64, stdin), "q", 1) );

	s_bStop = CVI_TRUE;

	pthread_join(readThread, NULL);

	s32Ret = SAMPLE_COMM_DATAFIFO_Exit(hDataFifo);
	if (s32Ret != 0) {
		SAMPLE_PRT("SAMPLE_COMM_DATAFIFO_Exit failed with:%d\n", s32Ret);
		return s32Ret;
	}

	return 0;
}


