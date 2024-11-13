#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include "sample_comm.h"

/*****************************************************************************
* function : for reader to get datafifo.
*****************************************************************************/
CVI_S32 SAMPLE_COMM_DATAFIFO_Access(CVI_DATAFIFO_PARAMS_S* params,
			CVI_U64 phyAddr, CVI_DATAFIFO_HANDLE *hDataFifo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_DATAFIFO_OpenByAddr(hDataFifo, params, phyAddr);
	if (CVI_SUCCESS != s32Ret) {
		SAMPLE_PRT("get datafifo error:%x\n", s32Ret);
		return -1;
	}

	SAMPLE_PRT("datafifo_init finish\n");

	return 0;
}

/*****************************************************************************
* function : for writer to init datafifo.
*****************************************************************************/
CVI_S32 SAMPLE_COMM_DATAFIFO_Init(CVI_DATAFIFO_PARAMS_S* params,
			CVI_U64 *phyAddr, CVI_DATAFIFO_HANDLE *hDataFifo, CVI_VOID *pArg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_DATAFIFO_Open(hDataFifo, params);
	if (CVI_SUCCESS != s32Ret) {
		SAMPLE_PRT("open datafifo error:%x\n", s32Ret);
		return -1;
	}

	s32Ret = CVI_DATAFIFO_CMD(*hDataFifo, DATAFIFO_CMD_GET_PHY_ADDR, phyAddr);
	if (CVI_SUCCESS != s32Ret) {
		SAMPLE_PRT("get datafifo phy addr error:%x\n", s32Ret);
		return -1;
	}

	SAMPLE_PRT("PhyAddr:%#llx\n", *phyAddr);

	s32Ret = CVI_DATAFIFO_CMD(*hDataFifo, DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, pArg);
	if (CVI_SUCCESS != s32Ret) {
		SAMPLE_PRT("set release func callback error:%x\n", s32Ret);
		return -1;
	}

	SAMPLE_PRT("datafifo_init finish\n");

	return 0;
}

/*****************************************************************************
* function : for reader to read datafifo.
*****************************************************************************/
CVI_S32 SAMPLE_COMM_DATAFIFO_Read(CVI_DATAFIFO_HANDLE hDataFifo, CVI_CHAR *pBuf)
{
	CVI_U32 readLen = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	readLen = 0;
	s32Ret = CVI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
	if (CVI_SUCCESS != s32Ret) {
		SAMPLE_PRT("datafifo get available read len failed with:%x\n", s32Ret);
		return s32Ret;
	}

	if (readLen > 0) {
		s32Ret = CVI_DATAFIFO_Read(hDataFifo, (CVI_VOID **)&pBuf);
		if (CVI_SUCCESS != s32Ret) {
			SAMPLE_PRT("datafifo read failed with:%x\n", s32Ret);
			return s32Ret;
		}

		SAMPLE_PRT("received: %s\n", pBuf);

		s32Ret = CVI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_READ_DONE, pBuf);
		if (CVI_SUCCESS != s32Ret) {
			SAMPLE_PRT("datafifo read done failed with:%x\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

/*****************************************************************************
* function : for writer to write datafifo.
*****************************************************************************/
CVI_S32 SAMPLE_COMM_DATAFIFO_Write(CVI_DATAFIFO_HANDLE hDataFifo, CVI_CHAR *pBuf)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 availWriteLen = 0;

	// call write NULL to flush
	s32Ret = CVI_DATAFIFO_Write(hDataFifo, NULL);
	if (CVI_SUCCESS != s32Ret) {
		SAMPLE_PRT("write error:%x\n", s32Ret);
	}

	s32Ret = CVI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
	if (CVI_SUCCESS != s32Ret) {
		SAMPLE_PRT("get available write len error:%x\n", s32Ret);
		return s32Ret;
	}

	if (availWriteLen > 0) {
		s32Ret = CVI_DATAFIFO_Write(hDataFifo, pBuf);
		if (CVI_SUCCESS != s32Ret) {
			SAMPLE_PRT("CVI_DATAFIFO_Write failed with:%x\n", s32Ret);
			return s32Ret;
		}

		SAMPLE_PRT("send: %s\n", pBuf);

		s32Ret = CVI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_WRITE_DONE, NULL);
		if (CVI_SUCCESS != s32Ret) {
			SAMPLE_PRT("write done failed with:%x\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

/*****************************************************************************
* function : for reader to exit datafifo.
*****************************************************************************/
CVI_S32 SAMPLE_COMM_DATAFIFO_Exit(CVI_DATAFIFO_HANDLE hDataFifo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_DATAFIFO_Close(hDataFifo);
	if (CVI_SUCCESS != s32Ret ) {
		SAMPLE_PRT("datafifo exit failed with %x\n", s32Ret);
		return s32Ret;
	}
	SAMPLE_PRT("datafifo exit finish\n");
	return s32Ret;
}
