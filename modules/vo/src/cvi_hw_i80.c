#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "cvi_hw_i80.h"
#include "cvi_sys_base.h"
#include "cvi_msg.h"
#include "msg_client.h"

CVI_S32 CVI_HWI80_Init(VO_DEV VoDev, const HW_I80_CFG_S *pstHwI80Attr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_HW_I80, pstHwI80Attr);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_HW_I80, VoDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_HWI80_INIT, (CVI_VOID *)pstHwI80Attr, sizeof(*pstHwI80Attr), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "CVI_HWI80_Init fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}
