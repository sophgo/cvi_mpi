#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/queue.h>
#include <pthread.h>
#include <stdatomic.h>
#include <inttypes.h>
#include <sys/mman.h>

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <sys/stat.h>
#include <linux/ion_cvitek.h>
#include <sys/ioctl.h>
#include "linux/sys_uapi.h"
#include <linux/cvi_tpu_ioctl.h>

#include "devmem.h"
#include "cvi_sys_base.h"
#include "cvi_sys.h"
#include "hashmap.h"
#include "cvi_msg.h"
#include "msg_client.h"

#define TPUDEVNAME "/dev/cvi-tpu0"
pthread_mutex_t tdma_pio_seq_lock = PTHREAD_MUTEX_INITIALIZER;
static uint32_t tdma_pio_seq;

static int devm_fd = -1, devm_cached_fd = -1;
static Hashmap *ionHashmap = NULL;
CVI_CHAR const *log_name[8] = {
	(CVI_CHAR *)"EMG", (CVI_CHAR *)"ALT", (CVI_CHAR *)"CRI", (CVI_CHAR *)"ERR",
	(CVI_CHAR *)"WRN", (CVI_CHAR *)"NOT", (CVI_CHAR *)"INF", (CVI_CHAR *)"DBG"
};


CVI_S32 *log_levels;


static int _ion_hash_key(void *key)
{
	return (((uintptr_t)key) >> 10);
}

static bool _ion_hash_equals(void *keyA, void *keyB)
{
	return (keyA == keyB);
}

CVI_S32 CVI_SYS_DevMem_Open(void)
{
	if (devm_fd < 0)
		devm_fd = devm_open();

	if (devm_cached_fd < 0)
		devm_cached_fd = devm_open_cached();

	if (devm_fd < 0 || devm_cached_fd < 0) {
		perror("devmem open failed\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_DevMem_Close(void)
{
	if (devm_fd < 0 || devm_cached_fd < 0)
		return CVI_SUCCESS;

	devm_close(devm_fd);
	devm_fd = -1;
	devm_close(devm_cached_fd);
	devm_cached_fd = -1;
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_Init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);
	FILE *file;

	CVI_TRACE_SYS(CVI_DBG_INFO, "+\n");

	if (CVI_SYS_DevMem_Open() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "devmem open failed.\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	file = fopen("/sys/module/cvi_fb/parameters/option", "r");
	if (file != NULL) {
		CVI_S32 option = 0;

		fscanf(file, "%d", &option);
		if (option & BIT(1)) {
			s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_FB_ON_SC, NULL, 0, NULL);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_SYS(CVI_DBG_ERR, "set fb on sc fail, s32Ret:%x\n", s32Ret);
				return s32Ret;
			}
			CVI_TRACE_SYS(CVI_DBG_INFO, "set fb on sc success.\n");
		}
		fclose(file);
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_ALIOS_INIT, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "alios sys init fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	CVI_TRACE_SYS(CVI_DBG_INFO, "-\n");

	return s32Ret;
}

CVI_S32 CVI_SYS_Exit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	CVI_TRACE_SYS(CVI_DBG_INFO, "+\n");

	s32Ret = CVI_SYS_DevMem_Close();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "devmem close failed\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_ALIOS_EXIT, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "alios sys exit fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	CVI_TRACE_SYS(CVI_DBG_INFO, "-\n");

	return s32Ret;
}

CVI_S32 CVI_SYS_Bind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);
	MMF_CHN_S stChn[2];

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstSrcChn);
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstDestChn);
	stChn[0] = *pstSrcChn;
	stChn[1] = *pstDestChn;

#ifdef __CV180X__
	if (pstDestChn->enModId == CVI_ID_VO) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "No vo device, vo cannot be bind!\n");
		return CVI_ERR_SYS_ILLEGAL_PARAM;
	}
#endif

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_BIND, (CVI_VOID *)stChn,
				sizeof(stChn), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Sys bind fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_UnBind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);
	MMF_CHN_S stChn[2];

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstSrcChn);
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstDestChn);
	stChn[0] = *pstSrcChn;
	stChn[1] = *pstDestChn;

#ifdef __CV180X__
	if (pstDestChn->enModId == CVI_ID_VO) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "No vo device, cannot unbind vo!\n");
		return CVI_ERR_SYS_ILLEGAL_PARAM;
	}
#endif

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_UNBIND, (CVI_VOID *)stChn,
				sizeof(stChn), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Sys unbind fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetBindbyDest(const MMF_CHN_S *pstDestChn, MMF_CHN_S *pstSrcChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstSrcChn);
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstDestChn);

	s32Ret = CVI_MSG_SendSync2(u32ModFd, MSG_CMD_SYS_GET_BIND_BY_DEST, (CVI_VOID *)pstDestChn,
				sizeof(MMF_CHN_S), (CVI_VOID *)pstSrcChn, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetBindbyDest fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetBindbySrc(const MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);
	MSG_PRIV_DATA_S stPrivData;

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstSrcChn);
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstBindDest);

	s32Ret = CVI_MSG_SendSync2(u32ModFd, MSG_CMD_SYS_GET_BIND_BY_SRC, (CVI_VOID *)pstSrcChn,
				sizeof(MMF_CHN_S), (CVI_VOID *)pstBindDest->astMmfChn, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetBindbySrc fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	pstBindDest->u32Num = stPrivData.as32PrivData[0];
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetVersion(MMF_VERSION_S *pstVersion)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVersion);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_GET_VERSION, pstVersion, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetVersion fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetChipId(CVI_U32 *pu32ChipId)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu32ChipId);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_GET_CHIP_ID, pu32ChipId, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetChipId fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetPowerOnReason(CVI_U32 *pu32PowerOnReason)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu32PowerOnReason);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_GET_POWER_REASON, pu32PowerOnReason, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetPowerOnReason fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetChipVersion(CVI_U32 *pu32ChipVersion)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu32ChipVersion);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_GET_CHIP_VERSION, pu32ChipVersion, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetChipVersion fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

void *CVI_SYS_Mmap(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
	CVI_SYS_DevMem_Open();

	return devm_map(devm_fd, u64PhyAddr, u32Size);
}

/* CVI_SYS_MmapCache - mmap the physical address to cached virtual-address
 *
 * @param pu64PhyAddr: the phy-address of the buffer
 * @param u32Size: the length of the buffer
 * @return virtual-address if success; 0 if fail.
 */
void *CVI_SYS_MmapCache(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
	CVI_SYS_DevMem_Open();

	void *addr = devm_map(devm_cached_fd, u64PhyAddr, u32Size);

	if (addr)
		CVI_SYS_IonInvalidateCache(u64PhyAddr, addr, u32Size);
	return addr;
}

CVI_S32 CVI_SYS_Munmap(void *pVirAddr, CVI_U32 u32Size)
{
	devm_unmap(pVirAddr, u32Size);
	return CVI_SUCCESS;
}

CVI_S32 queryHeapID(int devFd, enum ion_heap_type type)
{
	CVI_S32 ret;
	struct ion_heap_query heap_query;
	struct ion_heap_data *heap_data;
	CVI_S32 heap_id = -1;

	memset(&heap_query, 0, sizeof(heap_query));
	ret = ioctl(devFd, ION_IOC_HEAP_QUERY, &heap_query);
	if (ret < 0 || heap_query.cnt == 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl ION_IOC_HEAP_QUERY failed\n");
		return -1;
	}

	heap_data = (struct ion_heap_data *)calloc(heap_query.cnt, sizeof(struct ion_heap_data));
	if (heap_data == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "calloc failed\n");
		return -1;
	}

	heap_query.heaps = (unsigned long)heap_data;
	ret = ioctl(devFd, ION_IOC_HEAP_QUERY, &heap_query);
	if (ret < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl ION_IOC_HEAP_QUERY failed\n");
		free(heap_data);
		return -1;
	}

	for (CVI_U32 i = 0; i < heap_query.cnt; i++) {
		if (heap_data[i].type == type) {
			heap_id = (CVI_S32)heap_data[i].heap_id;
			break;
		}
	}

	free(heap_data);

	return heap_id;
}

int ionMalloc(int devFd, struct sys_ion_data *para, bool isCache)
{
	CVI_S32 ret;

	para->cached = isCache;
	ret = ioctl(devFd, SYS_ION_ALLOC, para);
	if (ret < 0) {
		printf("ioctl SYS_ION_ALLOC failed\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

int ionFree(struct sys_ion_data *para)
{
	CVI_S32 fd = -1;
	CVI_S32 ret = CVI_SUCCESS;

	if ((fd = get_sys_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	ret = ioctl(fd, SYS_ION_FREE, para);
	if (ret < 0) {
		printf("ioctl SYS_ION_ALLOC failed\n");
	}

	return CVI_SUCCESS;
}

static CVI_S32 _SYS_IonAlloc(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
			     CVI_U32 u32Len, CVI_BOOL cached, const CVI_CHAR *name)
{
	CVI_S32 fd = -1;
	struct sys_ion_data *ion_data;

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64PhyAddr);

	if ((fd = get_sys_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	if (!ionHashmap)
		ionHashmap = hashmapCreate(20, _ion_hash_key, _ion_hash_equals);

	ion_data = malloc(sizeof(*ion_data));
	ion_data->size = u32Len;
	// Set buffer as "anonymous" when user is passing null pointer.
	if (name)
		strncpy((char *)(ion_data->name), name, MAX_ION_BUFFER_NAME);
	else
		strncpy((char *)(ion_data->name), "anonymous", MAX_ION_BUFFER_NAME);

	if (ionMalloc(fd, ion_data, cached) != CVI_SUCCESS) {
		free(ion_data);
		CVI_TRACE_SYS(CVI_DBG_ERR, "alloc failed.\n");
		return CVI_ERR_SYS_NOMEM;
	}

	*pu64PhyAddr = ion_data->addr_p;
	hashmapPut(ionHashmap, (void *)(uintptr_t)*pu64PhyAddr, ion_data);

	if (ppVirAddr) {
		if (cached)
			*ppVirAddr = CVI_SYS_MmapCache(*pu64PhyAddr, u32Len);
		else
			*ppVirAddr = CVI_SYS_Mmap(*pu64PhyAddr, u32Len);
		if (*ppVirAddr == NULL) {
			hashmapGet(ionHashmap, (void *)(uintptr_t)*pu64PhyAddr);
			ionFree(ion_data);
			free(ion_data);
			CVI_TRACE_SYS(CVI_DBG_ERR, "mmap failed. (%s)\n", strerror(errno));
			return CVI_ERR_SYS_REMAPPING;
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_IonAlloc(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr, const CVI_CHAR *strName, CVI_U32 u32Len)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64PhyAddr);

	return _SYS_IonAlloc(pu64PhyAddr, ppVirAddr, u32Len, CVI_FALSE, strName);
}

/* CVI_SYS_IonAlloc_Cached - acquire buffer of u32Len from ion
 *
 * @param pu64PhyAddr: the phy-address of the buffer
 * @param ppVirAddr: the cached vir-address of the buffer
 * @param strName: the name of the buffer
 * @param u32Len: the length of the buffer acquire
 * @return CVI_SUCCES if ok
 */
CVI_S32 CVI_SYS_IonAlloc_Cached(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
				 const CVI_CHAR *strName, CVI_U32 u32Len)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64PhyAddr);
	return _SYS_IonAlloc(pu64PhyAddr, ppVirAddr, u32Len, CVI_TRUE, strName);
}

CVI_S32 CVI_SYS_IonFree(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr)
{
	struct sys_ion_data *ion_data;

	if (ionHashmap == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ion not alloc before.\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	ion_data = hashmapGet(ionHashmap, (void *)(uintptr_t)u64PhyAddr);
	if (ion_data == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "u64PhyAddr(0x%"PRIx64") not found in ion.\n", u64PhyAddr);
		return CVI_ERR_SYS_ILLEGAL_PARAM;
	}
	hashmapRemove(ionHashmap, (void *)(uintptr_t)u64PhyAddr);
	if (pVirAddr)
		devm_unmap(pVirAddr, ion_data->size);
	ionFree(ion_data);
	free(ion_data);

	if (hashmapSize(ionHashmap) == 0) {
		hashmapFree(ionHashmap);
		ionHashmap = NULL;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_IonGetMemoryState(ION_MEM_STATE_S *pstState)
{
	CVI_S32 fd = -1;
	CVI_S32 ret = CVI_SUCCESS;
	struct sys_ion_mem_state ion_state;

	if ((fd = get_sys_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	ret = ioctl(fd, SYS_ION_G_ION_MEM_STATE, &ion_state);
	if (ret < 0) {
		printf("ioctl SYS_ION_ALLOC failed\n");
	}

	pstState->total_size = ion_state.total_size;
	pstState->free_size = ion_state.free_size;
	pstState->max_avail_size = ion_state.max_avail_size;

	return ret;
}

CVI_S32 CVI_SYS_GetMemoryState(ION_MEM_STATE_S *pstState)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstState);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_GET_ALIOS_MEM_STATE,
				pstState, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Get alios memory statics fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_IonFlushCache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	CVI_S32 fd = -1;
	CVI_S32 ret = CVI_SUCCESS;
	struct sys_cache_op cache_cfg;

	if ((fd = get_sys_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	if (pVirAddr == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "pVirAddr Null.\n");
		return CVI_ERR_SYS_NULL_PTR;
	}

	cache_cfg.addr_p = u64PhyAddr;
	cache_cfg.addr_v = pVirAddr;
	cache_cfg.size = u32Len;

	ret = ioctl(fd, SYS_CACHE_FLUSH, &cache_cfg);
	if (ret < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ion flush err.\n");
		ret = CVI_ERR_SYS_NOTREADY;
	}
	return ret;
}

CVI_S32 CVI_SYS_IonInvalidateCache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	CVI_S32 fd = -1;
	CVI_S32 ret = CVI_SUCCESS;
	struct sys_cache_op cache_cfg;

	if ((fd = get_sys_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	if (pVirAddr == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "pVirAddr Null.\n");
		return CVI_ERR_SYS_NULL_PTR;
	}

	cache_cfg.addr_p = u64PhyAddr;
	cache_cfg.addr_v = pVirAddr;
	cache_cfg.size = u32Len;

	ret = ioctl(fd, SYS_CACHE_INVLD, &cache_cfg);
	if (ret < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ion flush err.\n");
		ret = CVI_ERR_SYS_NOTREADY;
	}
	return ret;
}

CVI_S32 CVI_SYS_SetVIVPSSMode(const VI_VPSS_MODE_S *pstVIVPSSMode)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVIVPSSMode);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_SET_VI_VPSS_MODE,
				(CVI_VOID *)pstVIVPSSMode, sizeof(VI_VPSS_MODE_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Set vi-vpss mode fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetVIVPSSMode(VI_VPSS_MODE_S *pstVIVPSSMode)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVIVPSSMode);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_GET_VI_VPSS_MODE,
				pstVIVPSSMode, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Get vi-vpss mode fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_SetVPSSMode(VPSS_MODE_E enVPSSMode)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);
	MSG_PRIV_DATA_S stPrivData;

	stPrivData.as32PrivData[0] = enVPSSMode;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_SET_VPSS_MODE, NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Set vpss mode fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

VPSS_MODE_E CVI_SYS_GetVPSSMode(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);
	VPSS_MODE_E enVpssMode;

	s32Ret = CVI_MSG_SendSync3(u32ModFd, MSG_CMD_SYS_GET_VPSS_MODE, NULL, 0, &enVpssMode);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Get vpss mode fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return enVpssMode;
}

CVI_S32 CVI_SYS_SetVPSSModeEx(const VPSS_MODE_S *pstVPSSMode)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVPSSMode);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_SET_VPSS_MODE_EX, (CVI_VOID *)pstVPSSMode,
				sizeof(VPSS_MODE_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "SetVPSSModeEx fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetVPSSModeEx(VPSS_MODE_S *pstVPSSMode)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVPSSMode);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_GET_VPSS_MODE_EX, pstVPSSMode, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetVPSSModeEx fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

const CVI_CHAR *CVI_SYS_GetModName(MOD_ID_E id)
{
	return CVI_GET_MOD_NAME(id);
}

CVI_S32 CVI_LOG_SetLevelConf(LOG_LEVEL_CONF_S *pstConf)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstConf);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_LOG_SET_LEVEL_CONF,
					(CVI_VOID *)pstConf, sizeof(LOG_LEVEL_CONF_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "SetLevelConf fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_LOG_GetLevelConf(LOG_LEVEL_CONF_S *pstConf)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstConf);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_LOG_GET_LEVEL_CONF,
					(CVI_VOID *)pstConf, sizeof(LOG_LEVEL_CONF_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetLevelConf fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetCurPTS(CVI_U64 *pu64CurPTS)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64CurPTS);

	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	*pu64CurPTS = ts.tv_sec*1000000 + ts.tv_nsec/1000;

	return CVI_SUCCESS;

}

CVI_S32 CVI_SYS_TDMACopy(CVI_U64 u64PhyDst, CVI_U64 u64PhySrc, CVI_U32 u32Len)
{
#define TDMA2D_LEN_LIMIT 0xFFFFFFFF

	static int tpu_fd = -1;
	struct cvi_tdma_copy_arg tdma_ioctl;
	struct cvi_tdma_wait_arg wait_ioctl;

	if (u32Len >= TDMA2D_LEN_LIMIT) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_TDMACopy() input param can't be supported\n");
		return CVI_ERR_SYS_NOT_SUPPORT;
	}

	tpu_fd = open(TPUDEVNAME, O_RDWR | O_DSYNC);
	if (tpu_fd < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "tpu fd open failed.\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	memset(&tdma_ioctl, 0, sizeof(struct cvi_tdma_copy_arg));

	pthread_mutex_lock(&tdma_pio_seq_lock);
	tdma_pio_seq++;
	tdma_ioctl.seq_no = tdma_pio_seq;
	pthread_mutex_unlock(&tdma_pio_seq_lock);

	tdma_ioctl.paddr_src = u64PhySrc;
	tdma_ioctl.paddr_dst = u64PhyDst;
	tdma_ioctl.leng_bytes = u32Len;
	ioctl(tpu_fd, CVITPU_SUBMIT_PIO, &tdma_ioctl);

	//wait finished
	wait_ioctl.seq_no = tdma_ioctl.seq_no;
	ioctl(tpu_fd, CVITPU_WAIT_PIO, &wait_ioctl);

	if (wait_ioctl.ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_TDMACopy wait failed\n");
	}

	close(tpu_fd);
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_TDMACopy2D(CVI_TDMA_2D_S *param)
{
#define TDMA2D_W_LIMIT 0x10000
#define TDMA2D_H_LIMIT 0x10000

	static int tpu_fd = -1;
	struct cvi_tdma_copy_arg tdma_ioctl;
	struct cvi_tdma_wait_arg wait_ioctl;

	if (param->stride_bytes_src < param->w_bytes ||
			param->stride_bytes_dst < param->w_bytes ||
			param->w_bytes >= TDMA2D_W_LIMIT ||
			param->h >= TDMA2D_H_LIMIT) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_TDMACopy2D() input param can't be supported\n");
		return CVI_ERR_SYS_NOT_SUPPORT;
	}

	tpu_fd = open(TPUDEVNAME, O_RDWR | O_DSYNC);
	if (tpu_fd < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "tpu fd open failed.\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	memset(&tdma_ioctl, 0, sizeof(struct cvi_tdma_copy_arg));

	pthread_mutex_lock(&tdma_pio_seq_lock);
	tdma_pio_seq++;
	tdma_ioctl.seq_no = tdma_pio_seq;
	pthread_mutex_unlock(&tdma_pio_seq_lock);

	tdma_ioctl.enable_2d = 1;
	tdma_ioctl.paddr_src = param->paddr_src;
	tdma_ioctl.paddr_dst = param->paddr_dst;
	tdma_ioctl.h = param->h;
	tdma_ioctl.w_bytes = param->w_bytes;
	tdma_ioctl.stride_bytes_src = param->stride_bytes_src;
	tdma_ioctl.stride_bytes_dst = param->stride_bytes_dst;
	ioctl(tpu_fd, CVITPU_SUBMIT_PIO, &tdma_ioctl);

	//wait finished
	wait_ioctl.seq_no = tdma_ioctl.seq_no;
	ioctl(tpu_fd, CVITPU_WAIT_PIO, &wait_ioctl);

	if (wait_ioctl.ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_TDMACopy2D wait failed\n");
	}

	close(tpu_fd);
	return CVI_SUCCESS;
}