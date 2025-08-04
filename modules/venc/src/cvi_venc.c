
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <inttypes.h>
#include "cvi_venc.h"
#include "cvi_type.h"
#include "cvi_sys_base.h"
#include "cvi_debug.h"
#include "cvi_msg.h"
#include "msg_client.h"
#include "cvi_sys.h"
#include "cvi_datafifo.h"


typedef struct _VENC_STREAM_PACK_S {
	VENC_PACK_S pstPack[8];
	VENC_STREAM_S stStream;
	VENC_CHN VeChn;
} VENC_STREAM_PACK_S;

#define UNUSED_VARIABLE(x) ((void)(x))

venc_dbg vencDbg;

static CVI_DATAFIFO_HANDLE hVencDataFifoHandle[VENC_MAX_CHN_NUM] = {0};

// #define FLOW_DEBUG 1

#ifdef FLOW_DEBUG

#define CVI_VENC_PRINT(msg, ...)		\
do { \
	printf("%s %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
} while (0)

#define CVI_VENC_API_IN  CVI_VENC_PRINT("Chn:%d In\n", VeChn);
#define CVI_VENC_API_OUT CVI_VENC_PRINT("Chn:%d Out\n", VeChn);


#define DATA_DUMP(data, len)  data_dump((unsigned char *)data, len, __FUNCTION__, __LINE__)

__attribute__((unused)) static void data_dump(unsigned char *pu8, int len, const char *func, int line)
{
	int i = 0;
	int sum = 0;

	printf("=====%s %d====,len:%d\n", func, line, len);
	for (i = 0; i < len; i++) {
		// printf("%d:0x%x\n", i, pu8[i]);
		sum += pu8[i];
	}
	printf("sum:%d\n", sum);

}
#else

#define CVI_VENC_API_IN
#define CVI_VENC_API_OUT
#define DATA_DUMP(data, len)
#endif

CVI_S32 CVI_VENC_Suspend(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModEnc = MODFD(CVI_ID_VENC, 0, 0);
	CVI_U32 u32ModDec = MODFD(CVI_ID_VDEC, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModDec, MSG_CMD_VDEC_SUSPEND, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VDEC_Suspend fail\n");
		return s32Ret;
	}

	s32Ret = CVI_MSG_SendSync(u32ModEnc, MSG_CMD_VENC_SUSPEND, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_Suspend fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_Resume(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModEnc = MODFD(CVI_ID_VENC, 0, 0);
	CVI_U32 u32ModDec = MODFD(CVI_ID_VDEC, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModDec, MSG_CMD_VDEC_RESUME, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VDEC_Resume fail\n");
		return s32Ret;
	}

	s32Ret = CVI_MSG_SendSync(u32ModEnc, MSG_CMD_VENC_RESUME, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_Resume fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_VOID cviGetMask(void)
{
	return;
}

CVI_S32 CVI_VENC_CreateChn(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstAttr);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_CREATE_CHN, (CVI_VOID *)pstAttr,
				sizeof(VENC_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CreateChn fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_DestroyChn(VENC_CHN VeChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VENC, 0, VeChn, 1);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_DESTROY_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("DestroyChn fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_ResetChn(VENC_CHN VeChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_RESET_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("ResetChn fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}


CVI_S32 CVI_VENC_StartRecvFrame(VENC_CHN VeChn, const VENC_RECV_PIC_PARAM_S *pstRecvParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);
	MSG_PRIV_DATA_S stPrivDate = {0};
	CVI_U64 u64PhyAddr = 0;
	CVI_U32 u32DataFifoLen = 0;
	CVI_DATAFIFO_PARAMS_S stDataFifoParams = {
		.u32EntriesNum = 0,
		.u32CacheLineSize = sizeof(VENC_STREAM_PACK_S),
		.bDataReleaseByWriter = CVI_FALSE,
		.enOpenMode = DATAFIFO_READER,
	};

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRecvParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_DATA_FIFO_LEN, (CVI_VOID *)&u32DataFifoLen,
				sizeof(CVI_U32), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("StartRecvFrame GetDataFifo fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	if (!u32DataFifoLen) {
		CVI_VENC_ERR("chn:%d u32DataFifoLen is 0\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_MSG_SendSync3(u32ModFd, MSG_CMD_VENC_START_RECV_FRAME, (CVI_VOID *)pstRecvParam,
				sizeof(VENC_RECV_PIC_PARAM_S), (CVI_U32 *)&stPrivDate.as32PrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("StartRecvFrame fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	DATA_DUMP(stPrivDate.as32PrivData, sizeof(MSG_PRIV_DATA_S));

	memcpy(&u64PhyAddr, stPrivDate.as32PrivData, sizeof(u64PhyAddr));

	stDataFifoParams.u32EntriesNum = u32DataFifoLen;
	s32Ret = CVI_DATAFIFO_OpenByAddr(&hVencDataFifoHandle[VeChn], &stDataFifoParams, u64PhyAddr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("DATAFIFO_OpenByAddr, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_StopRecvFrame(VENC_CHN VeChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_STOP_RECV_FRAME, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("StopRecvFrame fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	if (hVencDataFifoHandle[VeChn] != 0) {
		s32Ret = CVI_DATAFIFO_Close(hVencDataFifoHandle[VeChn]);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_DATAFIFO_Close fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
			return s32Ret;
		}
		hVencDataFifoHandle[VeChn] = 0;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_QueryStatus(VENC_CHN VeChn, VENC_CHN_STATUS_S *pstStatus)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStatus);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_QUERY_STATUS, (CVI_VOID *)pstStatus,
				sizeof(VENC_CHN_STATUS_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("QueryStatus fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetChnAttr(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstChnAttr);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_CHN_ATTR, (CVI_VOID *)pstChnAttr,
				sizeof(VENC_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetChnAttr fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetChnAttr(VENC_CHN VeChn, VENC_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstChnAttr);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_CHN_ATTR, (CVI_VOID *)pstChnAttr,
				sizeof(VENC_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetChnAttr fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream, CVI_S32 S32MilliSec)
{
#if 0
	CVI_S32 s32Ret;
	CVI_U32 i;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);
	MSG_PRIV_DATA_S stPrivDate;
	VENC_STREAM_PACK_S StreamPack;
	VENC_PACK_S *pPack = pstStream->pstPack;
	//stPrivDate.as32PrivData[0] = S32MilliSec;

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream);
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream->pstPack);

	CVI_VENC_API_IN;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_STREAM, (CVI_VOID *)&StreamPack,
				sizeof(VENC_STREAM_PACK_S), &stPrivDate);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("ReleaseStream fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	memcpy(pstStream->pstPack, StreamPack.pstPack, StreamPack.stStream.u32PackCount * sizeof(VENC_PACK_S));

	*pstStream = StreamPack.stStream;
	pstStream->pstPack = pPack;

	// in alios, no mmu, pyh addr is equal vir addr
	// stream form alios ion pyh addr, need map to vir addr
	for(i = 0; i < pstStream->u32PackCount; i++){
		if(pPack[i].u64PhyAddr){
			pPack[i].pu8Addr = CVI_SYS_Mmap(pPack[i].u64PhyAddr, pPack[i].u32Len);
			if(pPack[i].pu8Addr == NULL){
				CVI_VENC_ERR("CVI_SYS_Mmap fail, chn:%d, PhyAddr:0x%lx len:%d \n",
							VeChn, pPack[i].u64PhyAddr ,pPack[i].u32Len);
				return CVI_FAILURE;
			}
		}
	}
#else
	CVI_S32 s32Ret;
	CVI_U32 u32ReadLen;
	VENC_PACK_S *pPack = pstStream->pstPack;
	VENC_STREAM_PACK_S *pStreamPack;
	CVI_U32 i;
	CVI_U32 retrytimes;

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream);
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream->pstPack);
	UNUSED_VARIABLE(S32MilliSec);

	if(S32MilliSec < 0) {
		retrytimes = 1000;
	} else {
		retrytimes = S32MilliSec/5;
	}

	CVI_VENC_API_IN;
GET_STREAM_RETRY:
	s32Ret = CVI_DATAFIFO_CMD(hVencDataFifoHandle[VeChn], DATAFIFO_CMD_GET_AVAIL_READ_LEN, &u32ReadLen);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("DATAFIFO_CMD GET_AVAIL_READ_LEN fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	if (u32ReadLen > 0) {
		s32Ret = CVI_DATAFIFO_Read(hVencDataFifoHandle[VeChn], (CVI_VOID **)&pStreamPack);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_DATAFIFO_Read fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
			return s32Ret;
		}
		DATA_DUMP(pStreamPack, sizeof(VENC_STREAM_PACK_S));

		memcpy(pstStream->pstPack, pStreamPack->pstPack, pStreamPack->stStream.u32PackCount * sizeof(VENC_PACK_S));

		*pstStream = pStreamPack->stStream;
		pstStream->pstPack = pPack;

		s32Ret = CVI_DATAFIFO_CMD(hVencDataFifoHandle[VeChn], DATAFIFO_CMD_READ_DONE, NULL);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("DATAFIFO_CMD READ_DONE fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
			return s32Ret;
		}

		// in alios, no mmu, pyh addr is equal vir addr
		// stream form alios ion pyh addr, need map to vir addr
		for (i = 0; i < pstStream->u32PackCount; i++) {
			if (pPack[i].u64PhyAddr) {
				pPack[i].pu8Addr = CVI_SYS_MmapCache(pPack[i].u64PhyAddr, pPack[i].u32Len);
				if (pPack[i].pu8Addr == NULL) {
					CVI_VENC_ERR("CVI_SYS_Mmap fail, chn:%d, PhyAddr:0x%llu len:%d \n",
								VeChn, pPack[i].u64PhyAddr ,pPack[i].u32Len);
					return CVI_FAILURE;
				}
			}
		}

		CVI_VENC_API_OUT;
		return CVI_SUCCESS;
	}

#endif
	if(retrytimes--) {
		usleep(5 * 1000);
		goto GET_STREAM_RETRY;
	}

	CVI_VENC_API_OUT;

	return CVI_ERR_VENC_NOBUF;
}

CVI_S32 CVI_VENC_GetStreamEx(VENC_CHN VeChn, VENC_STREAM_S *pstStream, CVI_S32 S32MilliSec, CVI_U32 packsCnt)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ReadLen;
	VENC_PACK_S *pPack = pstStream->pstPack;
	VENC_STREAM_PACK_S *pStreamPack;
	CVI_U32 i;
	CVI_U32 retrytimes;
	CVI_U32 remainPacksCnt = 0;

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream);
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream->pstPack);
	UNUSED_VARIABLE(S32MilliSec);

	if(S32MilliSec < 0) {
		retrytimes = 1000;
	} else {
		retrytimes = S32MilliSec/5;
	}

	if (packsCnt == 0) {
		packsCnt = 1;
	}

	pstStream->u32PackCount = 0;
	pstStream->u32Seq = 0;

	CVI_VENC_API_IN;
GET_STREAM_RETRY:
	s32Ret = CVI_DATAFIFO_CMD(hVencDataFifoHandle[VeChn], DATAFIFO_CMD_GET_AVAIL_READ_LEN, &u32ReadLen);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("DATAFIFO_CMD GET_AVAIL_READ_LEN fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	if (u32ReadLen > 0) {
		do {
			s32Ret = CVI_DATAFIFO_Read(hVencDataFifoHandle[VeChn], (CVI_VOID **)&pStreamPack);
			if (s32Ret != CVI_SUCCESS) {
				CVI_VENC_ERR("CVI_DATAFIFO_Read fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
				return s32Ret;
			}
			DATA_DUMP(pStreamPack, sizeof(VENC_STREAM_PACK_S));

			memcpy(pstStream->pstPack + remainPacksCnt, pStreamPack->pstPack, pStreamPack->stStream.u32PackCount * sizeof(VENC_PACK_S));

			pstStream->u32PackCount += pStreamPack->stStream.u32PackCount;
			pstStream->u32Seq = pStreamPack->stStream.u32Seq;

			remainPacksCnt = pstStream->u32PackCount;
			u32ReadLen--;

			s32Ret = CVI_DATAFIFO_CMD(hVencDataFifoHandle[VeChn], DATAFIFO_CMD_READ_DONE, NULL);
			if (s32Ret != CVI_SUCCESS) {
				CVI_VENC_ERR("DATAFIFO_CMD READ_DONE fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
				return s32Ret;
			}
		} while (u32ReadLen > 0 && pstStream->u32PackCount < packsCnt);

		// in alios, no mmu, pyh addr is equal vir addr
		// stream form alios ion pyh addr, need map to vir addr
		for(i = 0; i < pstStream->u32PackCount; i++){
			if(pPack[i].u64PhyAddr){
				pPack[i].pu8Addr = CVI_SYS_MmapCache(pPack[i].u64PhyAddr, pPack[i].u32Len);
				if(pPack[i].pu8Addr == NULL){
					CVI_VENC_ERR("CVI_SYS_Mmap fail, chn:%d, PhyAddr:0x%llu len:%d \n",
								VeChn, pPack[i].u64PhyAddr ,pPack[i].u32Len);
					return CVI_FAILURE;
				}
			}
		}

		CVI_VENC_API_OUT;
		return CVI_SUCCESS;
	}

	if(retrytimes--) {
		usleep(5 * 1000);
		goto GET_STREAM_RETRY;
	}

	CVI_VENC_API_OUT;

	return CVI_ERR_VENC_NOBUF;
}

CVI_S32 CVI_VENC_ReleaseStreamEx(VENC_CHN VeChn, VENC_STREAM_S *pstStream)
{
	CVI_S32 s32Ret;
	CVI_U32 i, j;
	VENC_STREAM_PACK_S StreamPack;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream);
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream->pstPack);

	VENC_PACK_S *pPack = pstStream->pstPack;
	for(i = 0; i < pstStream->u32PackCount; i++){
		if(pPack[i].pu8Addr)
			CVI_SYS_Munmap(pPack[i].pu8Addr, pPack[i].u32Len);
	}

	memset(&StreamPack, 0, sizeof(VENC_STREAM_PACK_S));
	memcpy(&StreamPack.stStream, pstStream, sizeof(VENC_STREAM_S));

	// only copy max(8) sei packs to release
	for (i = 0, j = 0; i < pstStream->u32PackCount; i++) {
		if (pstStream->pstPack[i].DataType.enH264EType != H264E_NALU_SEI
			&& pstStream->pstPack[i].DataType.enH265EType != H265E_NALU_SEI) {
			continue;
		}

		if (j >= 8) {
			break;
		}

		memcpy(&StreamPack.pstPack[j], &pstStream->pstPack[i], sizeof(VENC_PACK_S));
		j++;
	}

	StreamPack.stStream.u32PackCount = j;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_RELEASE_STREAM, (CVI_VOID *)&StreamPack,
				sizeof(VENC_STREAM_PACK_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("ReleaseStream fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_ReleaseStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream)
{
#if 0
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);
	CVI_U32 i;
	VENC_PACK_S *pPack = pstStream->pstPack;

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream);
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream->pstPack);

	CVI_VENC_API_IN;

	for(i = 0; i < pstStream->u32PackCount; i++){
		if(pPack[i].pu8Addr)
			CVI_SYS_Munmap(pPack[i].pu8Addr, pPack[i].u32Len);
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_RELEASE_STREAM, (CVI_VOID *)pstStream,
				sizeof(VENC_STREAM_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("ReleaseStream fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
#else

	CVI_S32 s32Ret;
	CVI_U32 i;
	VENC_STREAM_PACK_S StreamPack;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream);
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream->pstPack);

	VENC_PACK_S *pPack = pstStream->pstPack;
	for(i = 0; i < pstStream->u32PackCount; i++){
		if(pPack[i].pu8Addr)
			CVI_SYS_Munmap(pPack[i].pu8Addr, pPack[i].u32Len);
	}

	memset(&StreamPack, 0, sizeof(VENC_STREAM_PACK_S));
	memcpy(&StreamPack.stStream, pstStream, sizeof(VENC_STREAM_S));
	memcpy(StreamPack.pstPack, pstStream->pstPack, pstStream->u32PackCount * sizeof(VENC_PACK_S));

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_RELEASE_STREAM, (CVI_VOID *)&StreamPack,
				sizeof(VENC_STREAM_PACK_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("ReleaseStream fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

#endif
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_InsertUserData(VENC_CHN VeChn, CVI_U8 *pu8Data, CVI_U32 u32Len)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VENC, 0, VeChn, 1);
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pu8Data);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_USER_DATA, pu8Data, u32Len, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("Insert User Data fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return s32Ret;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SendFrame(VENC_CHN VeChn, const VIDEO_FRAME_INFO_S *pstFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VENC, 0, VeChn, 1);
	MSG_PRIV_DATA_S stPrivDate;
	CVI_S32 i;
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstFrame);

	CVI_VENC_API_IN;

	stPrivDate.as32PrivData[0] = s32MilliSec;

	for (i = 0; i < 3; i++) {
		if (pstFrame->stVFrame.u32Length[i] && pstFrame->stVFrame.pu8VirAddr[i] != NULL) {
			CVI_SYS_IonFlushCache(
				pstFrame->stVFrame.u64PhyAddr[i],
				pstFrame->stVFrame.pu8VirAddr[i],
				pstFrame->stVFrame.u32Length[i]
			);
		}
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SEND_FRAME, (CVI_VOID *)pstFrame,
				sizeof(VIDEO_FRAME_INFO_S), &stPrivDate);
	if (s32Ret != CVI_SUCCESS && s32Ret != CVI_ERR_VENC_FRC_NO_ENC) {
		CVI_VENC_ERR("SendFrame fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return s32Ret;
}

CVI_S32 CVI_VENC_SendFrameEx(VENC_CHN VeChn, const USER_FRAME_INFO_S *pstFrame, CVI_S32 s32MilliSec)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstFrame);
	UNUSED_VARIABLE(s32MilliSec);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_RequestIDR(VENC_CHN VeChn, CVI_BOOL bInstant)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);
	MSG_PRIV_DATA_S stPrivDate;

	CVI_VENC_API_IN;

	stPrivDate.as32PrivData[0] = bInstant;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_REQUEST_IDR, NULL,
				0, &stPrivDate);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("RequestIDR fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetFd(VENC_CHN VeChn)
{
	UNUSED_VARIABLE(VeChn);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_CloseFd(VENC_CHN VeChn)
{
	UNUSED_VARIABLE(VeChn);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetRoiAttr(VENC_CHN VeChn, const VENC_ROI_ATTR_S *pstRoiAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRoiAttr);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_ROI_ATTR, (CVI_VOID *)pstRoiAttr,
				sizeof(VENC_ROI_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetRoiAttr fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetRoiAttr(VENC_CHN VeChn, CVI_U32 u32Index, VENC_ROI_ATTR_S *pstRoiAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);
	MSG_PRIV_DATA_S stPrivDate;

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRoiAttr);

	CVI_VENC_API_IN;

	stPrivDate.as32PrivData[0] = u32Index;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_ROI_ATTR, (CVI_VOID *)pstRoiAttr,
				sizeof(VENC_CHN_ATTR_S), &stPrivDate);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetRoiAttr fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetH264Trans(VENC_CHN VeChn, const VENC_H264_TRANS_S *pstH264Trans)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264Trans);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_H264_TRANS, (CVI_VOID *)pstH264Trans,
				sizeof(VENC_H264_TRANS_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetH264Trans fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetH264Trans(VENC_CHN VeChn, VENC_H264_TRANS_S *pstH264Trans)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264Trans);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_H264_TRANS, (CVI_VOID *)pstH264Trans,
				sizeof(VENC_H264_TRANS_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetH264Trans fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetH264Entropy(VENC_CHN VeChn, const VENC_H264_ENTROPY_S *pstH264EntropyEnc)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264EntropyEnc);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_H264_ENTROPY, (CVI_VOID *)pstH264EntropyEnc,
				sizeof(VENC_H264_ENTROPY_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetH264Entropy fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetH264Entropy(VENC_CHN VeChn, VENC_H264_ENTROPY_S *pstH264EntropyEnc)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264EntropyEnc);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_H264_ENTROPY, (CVI_VOID *)pstH264EntropyEnc,
				sizeof(VENC_H264_ENTROPY_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetH264Entropy fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetH264Vui(VENC_CHN VeChn, const VENC_H264_VUI_S *pstH264Vui)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264Vui);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_H264VUI, (CVI_VOID *)pstH264Vui,
				sizeof(VENC_H264_VUI_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_SetH264Vui fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetH264Vui(VENC_CHN VeChn, VENC_H264_VUI_S *pstH264Vui)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264Vui);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_H264VUI, (CVI_VOID *)pstH264Vui,
				sizeof(VENC_H264_VUI_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_GetH264Vui fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetH265Vui(VENC_CHN VeChn, const VENC_H265_VUI_S *pstH265Vui)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH265Vui);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_H265VUI, (CVI_VOID *)pstH265Vui,
				sizeof(VENC_H265_VUI_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_SetH265Vui fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetH265Vui(VENC_CHN VeChn, VENC_H265_VUI_S *pstH265Vui)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH265Vui);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_H265VUI, (CVI_VOID *)pstH265Vui,
				sizeof(VENC_H265_VUI_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_GetH265Vui fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetJpegParam(VENC_CHN VeChn, const VENC_JPEG_PARAM_S *pstJpegParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstJpegParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_JPEG_PARAM, (CVI_VOID *)pstJpegParam,
				sizeof(VENC_JPEG_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetJpegParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetJpegParam(VENC_CHN VeChn, VENC_JPEG_PARAM_S *pstJpegParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstJpegParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_JPEG_PARAM, (CVI_VOID *)pstJpegParam,
				sizeof(VENC_JPEG_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetJpegParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetRcParam(VENC_CHN VeChn, const VENC_RC_PARAM_S *pstRcParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRcParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_RC_PARAM, (CVI_VOID *)pstRcParam,
				sizeof(VENC_RC_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetRcParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetRcParam(VENC_CHN VeChn, VENC_RC_PARAM_S *pstRcParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRcParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_RC_PARAM, (CVI_VOID *)pstRcParam,
				sizeof(VENC_RC_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetRcParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetRefParam(VENC_CHN VeChn, const VENC_REF_PARAM_S *pstRefParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRefParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_REF_PARAM, (CVI_VOID *)pstRefParam,
				sizeof(VENC_REF_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetRefParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetRefParam(VENC_CHN VeChn, VENC_REF_PARAM_S *pstRefParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRefParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_REF_PARAM, (CVI_VOID *)pstRefParam,
				sizeof(VENC_REF_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetRefParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetH265Trans(VENC_CHN VeChn, const VENC_H265_TRANS_S *pstH265Trans)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstH265Trans);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetH265Trans(VENC_CHN VeChn, VENC_H265_TRANS_S *pstH265Trans)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstH265Trans);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetFrameLostStrategy(VENC_CHN VeChn, const VENC_FRAMELOST_S *pstFrmLostParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstFrmLostParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_FRAME_LOST, (CVI_VOID *)pstFrmLostParam,
				sizeof(VENC_FRAMELOST_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetFrameLostStrategy fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetFrameLostStrategy(VENC_CHN VeChn, VENC_FRAMELOST_S *pstFrmLostParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstFrmLostParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_FRAME_LOST, (CVI_VOID *)pstFrmLostParam,
				sizeof(VENC_FRAMELOST_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetFrameLostStrategy fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetSuperFrameStrategy(VENC_CHN VeChn, const VENC_SUPERFRAME_CFG_S *pstSuperFrmParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstSuperFrmParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_SUPERFRAME_STRATEGY, (CVI_VOID *)pstSuperFrmParam,
				sizeof(VENC_CHN_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetChnParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetSuperFrameStrategy(VENC_CHN VeChn, VENC_SUPERFRAME_CFG_S *pstSuperFrmParam)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstSuperFrmParam);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetChnParam(VENC_CHN VeChn, const VENC_CHN_PARAM_S *pstChnParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstChnParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_CHN_PARAM, (CVI_VOID *)pstChnParam,
				sizeof(VENC_CHN_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetChnParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetChnParam(VENC_CHN VeChn, VENC_CHN_PARAM_S *pstChnParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstChnParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_CHN_PARAM, (CVI_VOID *)pstChnParam,
				sizeof(VENC_CHN_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetChnParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetModParam(const VENC_PARAM_MOD_S *pstModParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, 0);
	VENC_CHN VeChn = 0;
	UNUSED_VARIABLE(VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstModParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_MOD_PARAM, (CVI_VOID *)pstModParam,
				sizeof(VENC_PARAM_MOD_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetModParam fail, ret:0x%x\n", s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetModParam(VENC_PARAM_MOD_S *pstModParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, 0);
	VENC_CHN VeChn = 0;
	UNUSED_VARIABLE(VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstModParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_MOD_PARAM, (CVI_VOID *)pstModParam,
				sizeof(VENC_PARAM_MOD_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetModParam fail, ret:0x%x\n", s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_AttachVbPool(VENC_CHN VeChn, const VENC_CHN_POOL_S *pstPool)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstPool);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_ATTACH_VBPOOL, (CVI_VOID *)pstPool,
				sizeof(VENC_CHN_POOL_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("AttachVbPool fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_DetachVbPool(VENC_CHN VeChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_DETACH_VBPOOL, NULL,
				0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("DetachVbPool fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetDataFifoLen(VENC_CHN VeChn, CVI_U32 u32Len)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_DATA_FIFO_LEN, (CVI_VOID *)&u32Len,
				sizeof(u32Len), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetDataFifoLen fail, ret:0x%x\n", s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetDataFifoLen(VENC_CHN VeChn, CVI_U32 *pU32Len)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_DATA_FIFO_LEN, (CVI_VOID *)pU32Len,
				sizeof(CVI_U32), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetDataFifoLen fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetCuPrediction(VENC_CHN VeChn,
		const VENC_CU_PREDICTION_S *pstCuPrediction)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstCuPrediction);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetCuPrediction(VENC_CHN VeChn, VENC_CU_PREDICTION_S *pstCuPrediction)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstCuPrediction);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_CalcFrameParam(VENC_CHN VeChn, VENC_FRAME_PARAM_S *pstFrameParam)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstFrameParam);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetFrameParam(VENC_CHN VeChn, const VENC_FRAME_PARAM_S *pstFrameParam)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstFrameParam);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetFrameParam(VENC_CHN VeChn, VENC_FRAME_PARAM_S *pstFrameParam)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstFrameParam);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetH264SliceSplit(VENC_CHN VeChn, const VENC_H264_SLICE_SPLIT_S *pstSliceSplit)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstSliceSplit);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetH264SliceSplit(VENC_CHN VeChn, VENC_H264_SLICE_SPLIT_S *pstSliceSplit)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstSliceSplit);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetH265SliceSplit(VENC_CHN VeChn, const VENC_H265_SLICE_SPLIT_S *pstSliceSplit)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstSliceSplit);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetH265SliceSplit(VENC_CHN VeChn, VENC_H265_SLICE_SPLIT_S *pstSliceSplit)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstSliceSplit);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetH264Dblk(VENC_CHN VeChn, const VENC_H264_DBLK_S *pstH264Dblk)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstH264Dblk);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetH264Dblk(VENC_CHN VeChn, VENC_H264_DBLK_S *pstH264Dblk)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstH264Dblk);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetH265Dblk(VENC_CHN VeChn, const VENC_H265_DBLK_S *pstH265Dblk)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstH265Dblk);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetH265Dblk(VENC_CHN VeChn, VENC_H265_DBLK_S *pstH265Dblk)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstH265Dblk);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetH264IntraPred(VENC_CHN VeChn, const VENC_H264_INTRA_PRED_S *pstH264IntraPred)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstH264IntraPred);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetH264IntraPred(VENC_CHN VeChn, VENC_H264_INTRA_PRED_S *pstH264IntraPred)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstH264IntraPred);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_EnableSVC(VENC_CHN VeChn, CVI_BOOL bEnable)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);
	MSG_PRIV_DATA_S stPrivDate;

	CVI_VENC_API_IN;

	stPrivDate.as32PrivData[0] = bEnable;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_ENABLE_SVC, NULL,
				0, &stPrivDate);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("EnableSVC fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetSvcParam(VENC_CHN VeChn, const VENC_SVC_PARAM_S *pstSvcParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstSvcParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_SVC_PARAM, (CVI_VOID *)pstSvcParam,
				sizeof(VENC_CHN_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetChnParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetSvcParam(VENC_CHN VeChn, VENC_SVC_PARAM_S *pstSvcParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstSvcParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_SVC_PARAM, (CVI_VOID *)pstSvcParam,
				sizeof(VENC_CHN_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetChnParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}
