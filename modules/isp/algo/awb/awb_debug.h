/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: awb_debug.h
 * Description:
 *
 */

#ifndef _AWB_DEBUG_H_
#define _AWB_DEBUG_H_

#include "stddef.h"
#include "stdint.h"
#include <sys/time.h>

#include <cvi_common.h>
#include <cvi_comm_vi.h>
#include <cvi_comm_video.h>
#include <cvi_defines.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

void AWB_SetDebugMode(CVI_U8 mode);
CVI_U8 AWB_GetDebugMode(void);
void AWB_Debug(void);
void AWB_ShowInfoList(void);
void CVI_AWB_AutoTest(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif // _AWB_DEBUG_H_
