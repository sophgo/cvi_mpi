/* Include files */
#ifndef __CVIAUDIO_ALGO_INTERFACE2__
#define __CVIAUDIO_ALGO_INTERFACE2__
#include <stdio.h>

#ifndef AEC_PRO_DATA_LEN
#define AEC_PRO_DATA_LEN (160)
#endif

void *CviAud_Algo_Init2(int s32FunctMask, void *param_info);

int CviAud_Algo_Process2(void *pHandle,  short *mic_in,
			short *ref_in, short *out, int iLength);

void CviAud_Algo_DeInit2(void *pHandle);

void  CviAud_Algo_GetVersion2(char *pstrVersion);

#endif
