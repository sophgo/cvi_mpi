#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/queue.h>
#include <pthread.h>
#include <stdatomic.h>
#include <inttypes.h>

#include "devmem.h"
#include "cvi_sys_base.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_msg.h"
#include "msg_client.h"

#ifndef UNUSED
#define UNUSED(x) ((x) = (x))
#endif


VB_BLK CVI_VB_GetBlock(VB_POOL Pool, CVI_U32 u32BlkSize)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	MSG_PRIV_DATA_S stPrivData;
	VB_BLK blk = VB_INVALID_HANDLE;

	stPrivData.as32PrivData[0] = Pool;
	stPrivData.as32PrivData[1] = u32BlkSize;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_GET_BLOCK, &blk, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "get block fail,s32Ret:%x\n", s32Ret);
		return blk;
	}
	return blk;
}

/* CVI_VB_ReleaseBlock: release a vb_blk.
 *
 * @param Block: the vb_blk going to be released.
 * @return: CVI_SUCCESS if success; others if fail.
 */
CVI_S32 CVI_VB_ReleaseBlock(VB_BLK Block)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_RELEASE_BLOCK, &Block, sizeof(Block), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Release block fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

VB_BLK CVI_VB_PhysAddr2Handle(CVI_U64 u64PhyAddr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	VB_BLK blk = VB_INVALID_HANDLE;

	s32Ret = CVI_MSG_SendSync2(u32ModFd, MSG_CMD_VB_PHYS_ADDR2_HANDLE, &u64PhyAddr,
				sizeof(u64PhyAddr), &blk, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "PhysAddr2Handle fail,s32Ret:%x\n", s32Ret);
		return VB_INVALID_HANDLE;
	}
	return blk;
}

CVI_U64 CVI_VB_Handle2PhysAddr(VB_BLK Block)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	CVI_U64 u64PhyAddr = 0;

	s32Ret = CVI_MSG_SendSync2(u32ModFd, MSG_CMD_VB_HANDLE2_PHYS_ADDR, &Block, sizeof(Block),
				&u64PhyAddr, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Handle2PhysAddr fail,s32Ret:%x\n", s32Ret);
		return u64PhyAddr;
	}
	return u64PhyAddr;
}

VB_POOL CVI_VB_Handle2PoolId(VB_BLK Block)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	VB_POOL pool = VB_INVALID_POOLID;

	pool = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_HANDLE2_POOL_ID, &Block, sizeof(Block), NULL);
	return pool;
}

CVI_S32 CVI_VB_InquireUserCnt(VB_BLK Block, CVI_U32 *pCnt)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VB, pCnt);

	s32Ret = CVI_MSG_SendSync3(u32ModFd, MSG_CMD_VB_INQUIRE_USER_CNT, &Block, sizeof(Block),
				pCnt);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "InquireUserCnt fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

CVI_S32 CVI_VB_Init(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_INIT, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "vb init fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_VB_Exit(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_EXIT, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "vb exit fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

VB_POOL CVI_VB_CreatePool(VB_POOL_CONFIG_S *pstVbPoolCfg)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	VB_POOL pool = VB_INVALID_POOLID;

	MOD_CHECK_NULL_PTR(CVI_ID_VB, pstVbPoolCfg);

	pool = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_CREATE_POOL, pstVbPoolCfg,
				sizeof(VB_POOL_CONFIG_S), NULL);
	return pool;
}

VB_POOL CVI_VB_CreateExPool(VB_POOL_CONFIG_EX_S *pstVbPoolExCfg)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	VB_POOL pool = VB_INVALID_POOLID;

	MOD_CHECK_NULL_PTR(CVI_ID_VB, pstVbPoolExCfg);

	pool = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_CREATE_EX_POOL, pstVbPoolExCfg,
				sizeof(VB_POOL_CONFIG_EX_S), NULL);
	return pool;
}

CVI_S32 CVI_VB_DestroyPool(VB_POOL Pool)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	MSG_PRIV_DATA_S stPrivData;

	stPrivData.as32PrivData[0] = Pool;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_DESTROY_POOL, NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "DestroyPool fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_VB_SetConfig(const VB_CONFIG_S *pstVbConfig)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VB, pstVbConfig);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_SET_CONFIG, (CVI_VOID *)pstVbConfig,
		sizeof(VB_CONFIG_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Set config fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_VB_GetConfig(VB_CONFIG_S *pstVbConfig)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VB, pstVbConfig);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_GET_CONFIG, pstVbConfig, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Get config fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

/* CVI_VB_MmapPool - mmap the whole pool to get virtual-address
 *
 * @param Pool: pool id
 * @return CVI_SUCCESS if success; others if fail
 */
CVI_S32 CVI_VB_MmapPool(VB_POOL Pool)
{
	UNUSED(Pool);
	CVI_TRACE_VB(CVI_DBG_WARN, "mmap pool(%d) not supported yet.\n", Pool);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VB_MunmapPool(VB_POOL Pool)
{
	UNUSED(Pool);
	CVI_TRACE_VB(CVI_DBG_WARN, "munmap pool(%d) not supported yet.\n", Pool);
	return CVI_SUCCESS;
}

/* CVI_VB_GetBlockVirAddr - to get virtual-address of the Block
 *
 * @param Pool: pool id
 * @param Block: block id
 * @param ppVirAddr: virtual-address of the Block, cached if pool create with VB_REMAP_MODE_CACHED
 * @return CVI_SUCCESS if success; others if fail
 */
CVI_S32 CVI_VB_GetBlockVirAddr(VB_POOL Pool, VB_BLK Block, void **ppVirAddr)
{
	UNUSED(Pool);
	UNUSED(Block);
	UNUSED(ppVirAddr);
	CVI_TRACE_VB(CVI_DBG_WARN, "GetBlockVirAddr not supported yet.\n");

	return CVI_SUCCESS;
}

CVI_VOID CVI_VB_PrintPool(VB_POOL Pool)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	MSG_PRIV_DATA_S stPrivData;

	stPrivData.as32PrivData[0] = Pool;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_PRINT_POOL, NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "PrintPool fail,s32Ret:%x\n", s32Ret);
	}
}

