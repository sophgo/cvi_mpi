
#include <errno.h>
#include "cvi_comm_ipcm.h"
#include "cvi_ipcm.h"
#include "ipcm_port_common.h"
#include "ipcm_anonymous.h"

// ANON PROC MaGic: A(0x41) N(0x4E) M(0xdD) G(0x47)
#define ANON_MSG_PROC_MAGIC 0x414E4D47
typedef struct _IPCM_ANON_MSGPROC_S {
    CVI_U8 u8PortID;
    CVI_VOID *pPrivData;
    IPCM_ANON_MSGPROC_FN pfnAnonProc;
    struct _IPCM_ANON_MSGPROC_S *next;
} IPCM_ANON_MSGPROC_S;

static IPCM_ANON_MSGPROC_S *s_anon_proc_head = NULL;

static s32 _ANON_MSGPROC_HANDLE(void *priv, ipcm_anon_msg_t *data)
{
    IPCM_ANON_MSGPROC_S *tmp = s_anon_proc_head;

    if (priv != (void *)ANON_MSG_PROC_MAGIC) {
        ipcm_err("unexcepted magic(%lx), excepted(%x)\n", (unsigned long)priv, ANON_MSG_PROC_MAGIC);
        return -EFAULT;
    }

    if (data) {
        IPCM_ANON_MSG_S stAnon = {};
        stAnon.u8PortID = data->port_id;
        stAnon.u8MsgID = data->msg_id;
        stAnon.u8DataType = data->data_type;
        if (MSG_TYPE_SHM == data->data_type) {
            stAnon.stData.pData = data->data;
            stAnon.stData.u32Size = data->size;
        } else {
            stAnon.u32Param = (unsigned int)(unsigned long)data->data;
        }
        while (tmp != NULL) {
            if (tmp->u8PortID == stAnon.u8PortID) {
                if (tmp->pfnAnonProc) {
                    return tmp->pfnAnonProc(tmp->pPrivData, &stAnon);
                }
                else {
                    ipcm_warning("port id(%d) been found, but handle is null.\n", tmp->u8PortID);
                    return -EFAULT;
                }
            }
            tmp = tmp->next;
        }
        ipcm_warning("port id(%d) is not registered.\n", stAnon.u8PortID);
        return 0;
    }

    ipcm_err("_ANON_MSGPROC_HANDLE data is null.\n");
    return -EFAULT;
}

CVI_S32 CVI_IPCM_Init(void)
{
#ifdef __ALIOS__
    // do nothing while alios
    return 0;
#else
    return ipcm_port_common_init();
#endif
}

CVI_S32 CVI_IPCM_Uninit(void)
{
#ifdef __ALIOS__
    // do nothing while alios
    return 0;
#else
    return ipcm_port_common_uninit();
#endif
}

CVI_S32 CVI_IPCM_InvData(CVI_VOID *pData, CVI_U32 u32Size)
{
    return ipcm_inv_data(pData, u32Size);
}

CVI_S32 CVI_IPCM_FlushData(CVI_VOID *pData, CVI_U32 u32Size)
{
    return ipcm_flush_data(pData, u32Size);
}

CVI_S32 CVI_IPCM_DataLock(CVI_U8 u8LockID)
{
    return ipcm_data_lock(u8LockID);
}

CVI_S32 CVI_IPCM_DataUnlock(CVI_U8 u8LockID)
{
    return ipcm_data_unlock(u8LockID);
}

CVI_VOID *CVI_IPCM_GetBuff(CVI_U32 u32Size)
{
    return ipcm_get_buff(u32Size);
}

CVI_S32 CVI_IPCM_ReleaseBuff(CVI_VOID *pData)
{
    return ipcm_release_buff(pData);
}

CVI_VOID *CVI_IPCM_GetUserAddr(CVI_U32 u32Paddr)
{
    return ipcm_get_user_addr(u32Paddr);
}

CVI_S32 CVI_IPCM_PoolReset(void)
{
    return ipcm_pool_reset();
}

CVI_U32 CVI_IPCM_GetParamBinAddr(void)
{
    return get_param_bin_addr();
}

CVI_U32 CVI_IPCM_GetParamBakBinAddr(void)
{
    return get_param_bak_bin_addr();
}

CVI_U32 CVI_IPCM_GetPQBinQddr(void)
{
    return get_pq_bin_addr();
}

CVI_S32 CVI_IPCM_AnonInit(void)
{
    ipcm_anon_register_handle(_ANON_MSGPROC_HANDLE, (void *)ANON_MSG_PROC_MAGIC);
    return ipcm_anon_init();
}

CVI_S32 CVI_IPCM_AnonUninit(void)
{
    ipcm_anon_deregister_handle();
    return ipcm_anon_uninit();
}

CVI_S32 CVI_IPCM_RegisterAnonHandle(CVI_U8 u8PortID, IPCM_ANON_MSGPROC_FN pfnHandler, CVI_VOID *pData)
{
    IPCM_ANON_MSGPROC_S *proc = NULL;
    IPCM_ANON_MSGPROC_S *tmp;
    IPCM_ANON_MSGPROC_S *tmp_next;

    if (u8PortID >= IPCM_ANON_PORT_MAX) {
        ipcm_err("port id(%d) is invalid.\n", u8PortID);
        return -EINVAL;
    }

    if (NULL == pfnHandler) {
        ipcm_warning("pfnHandler is null, port id(%d) will be deregistered.\n", u8PortID);
        CVI_IPCM_DeregisterAnonHandle(u8PortID);
        return 0;
    }

    proc = ipcm_alloc(sizeof(IPCM_ANON_MSGPROC_S));
    if (NULL == proc) {
        ipcm_err("proc malloc fail.\n");
        return -ENOMEM;
    }
    proc->u8PortID = u8PortID;
    proc->pPrivData = pData;
    proc->pfnAnonProc = pfnHandler;
    proc->next = NULL;

    tmp = s_anon_proc_head;
    tmp_next = s_anon_proc_head;
    while(tmp_next != NULL) {
        tmp = tmp_next;
        if (tmp->u8PortID == u8PortID) { // u8PortID has been registered
            ipcm_warning("port id(%d) has been registered, update.\n", u8PortID);
            tmp->pPrivData = pData;
            tmp->pfnAnonProc = pfnHandler;
            ipcm_free(proc);
            return 0;
        }
        tmp_next = tmp_next->next;
    }

    if (NULL == tmp) { // first handler
        s_anon_proc_head = proc;
    } else {
        tmp->next = proc;
    }

    return 0;
}

CVI_S32 CVI_IPCM_DeregisterAnonHandle(CVI_U8 u8PortID)
{
    IPCM_ANON_MSGPROC_S *tmp;
    IPCM_ANON_MSGPROC_S *tmp_pre;

    if (u8PortID >= IPCM_ANON_PORT_MAX) {
        ipcm_err("port id(%d) is invalid.\n", u8PortID);
        return -EINVAL;
    }

    tmp = s_anon_proc_head;
    tmp_pre = s_anon_proc_head;
    while(tmp != NULL) {
        if (tmp->u8PortID == u8PortID) {
            if (tmp == s_anon_proc_head) {
                s_anon_proc_head = s_anon_proc_head->next;
            } else {
                tmp_pre->next = tmp->next;
            }
            ipcm_free(tmp);
            return 0;
        }
        tmp_pre = tmp;
        tmp = tmp->next;
    }

    ipcm_warning("port id(%d) not been registered.\n", u8PortID);
    return -EFAULT;
}

// send msg if msg len > 4; max msg length is limited by pool block (2048?)
CVI_S32 CVI_IPCM_AnonSendMsg(CVI_U8 u8PortID, CVI_U8 u8MsgID, CVI_VOID *pData, CVI_U32 u32Len)
{
    if (pData == NULL) {
        ipcm_err("pData is null.\n");
        return -EINVAL;
    }

    if (u8MsgID > IPCM_MSG_ID_MAX) {
        ipcm_err("u8MsgID(%d) out of range, max(%d).\n", u8MsgID, IPCM_MSG_ID_MAX);
        return -EINVAL;
    }

    return ipcm_anon_send_msg(u8PortID, u8MsgID, pData, u32Len);
}

// send param if msg len <= 4 or send 32 bits addr
CVI_S32 CVI_IPCM_AnonSendParam(CVI_U8 u8PortID, CVI_U8 u8MsgID, CVI_U32 u32Param)
{
    return ipcm_anon_send_param(u8PortID, u8MsgID, u32Param);
}

CVI_VOID *CVI_IPCM_AnonGetUserAddr(CVI_U32 u32Paddr)
{
    return ipcm_anon_get_user_addr(u32Paddr);
}

CVI_S32 CVI_IPCM_SetRtosSysBootStat(void)
{
#ifdef __ALIOS__
    return ipcm_set_rtos_boot_bit(RTOS_SYS_BOOT_STAT, 1);
#else
    ipcm_warning("do not support set rtos sys boot status by linux.\n");
    return -EOPNOTSUPP;
#endif
}

CVI_S32 CVI_IPCM_SetRtosBootLogoStat(void)
{
#ifdef __ALIOS__
    return ipcm_set_rtos_boot_bit(RTOS_BOOTLOGO_DONE, 1);
#else
    ipcm_warning("do not support set rtos sys boot status by linux.\n");
    return -EOPNOTSUPP;
#endif
}

CVI_S32 CVI_IPCM_ClrRtosSysBootStat(void)
{
#ifdef __ALIOS__
    return ipcm_set_rtos_boot_bit(RTOS_SYS_BOOT_STAT, 0);
#else
    ipcm_warning("do not support clear rtos sys boot status by linux.\n");
    return -EOPNOTSUPP;
#endif
}

CVI_S32 CVI_IPCM_GetRtosBootStatus(CVI_U32 *pBootStatus)
{
    s32 ret;
    u32 stat;

    ret = ipcm_get_rtos_boot_status(&stat);
    if(ret) {
        ipcm_err("ipcm_get_rtos_boot_status fail, ret:%d\n", ret);
        return ret;
    }

    *pBootStatus = 0;
    if (stat & (1 << RTOS_SYS_BOOT_STAT)) {
        *pBootStatus |= (1 << IPCM_RTOS_SYS_INIT_STAT);
    }
    if (stat & (1 << RTOS_PANIC)) {
        *pBootStatus |= (1 << IPCM_RTOS_PANIC);
    }
    if (stat & (1 << RTOS_IPCM_DONE)) {
        *pBootStatus |= (1 << IPCM_RTOS_IPCM_DONE);
    }
    if (stat & (1 << RTOS_IPCMSG_DONE)) {
        *pBootStatus |= (1 << IPCM_RTOS_IPCMSG_DONE);
    }
    if (stat & (1 << RTOS_VI_DONE)) {
        *pBootStatus |= (1 << IPCM_RTOS_VI_DONE);
    }
    if (stat & (1 << RTOS_VPSS_DONE)) {
        *pBootStatus |= (1 << IPCM_RTOS_VPSS_DONE);
    }
    if (stat & (1 << RTOS_VENC_DONE)) {
        *pBootStatus |= (1 << IPCM_RTOS_VENC_DONE);
    }
    if (stat & (1 << RTOS_BOOTLOGO_DONE)) {
        *pBootStatus |= (1 << IPCM_RTOS_BOOTLOGO_DONE);
    }
    return 0;
}
