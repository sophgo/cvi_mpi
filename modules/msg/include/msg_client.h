#ifndef __CVI_MESSAGE_CLIENT_H__
#define __CVI_MESSAGE_CLIENT_H__
#if __cplusplus
extern "C" {
#endif

typedef struct tagMSG_PRIV_DATA_S {
    CVI_S32 as32PrivData[8];
} MSG_PRIV_DATA_S;

/* CVI_MSG_SendSync: Send sync messages, this api will block and wait
for the other end's message command to be processed before returning
 *
 * @param u32Module: module which recv this message
 * @param u32CMD: command module processed
 * @param pBody: pointer to information body message carried
 * @param u32BodyLen: body information length message carried
 * @param pstPrivData: pointer to private data message carried
 * @return: 0 if success
 */
CVI_S32 CVI_MSG_SendSync(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
	MSG_PRIV_DATA_S *pstPrivData);

/* CVI_MSG_SendSync2: Send sync messages, this api will block and wait
for the other end's message command to be processed before returning
 *
 * @param u32Module: module which recv this message
 * @param u32CMD: command module processed
 * @param pBody: pointer to information body message carried
 * @param pRespBody: pointer to response body alios returned
 * @param u32BodyLen: body information length message carried
 * @param pstPrivData: pointer to private data message carried
 * @return: 0 if success
 */
CVI_S32 CVI_MSG_SendSync2(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
	CVI_VOID *pRespBody, MSG_PRIV_DATA_S *pstPrivData);

/* CVI_MSG_SendSync3: Send sync messages, this api will block and wait
for the other end's message command to be processed before returning
 *
 * @param u32Module: module which recv this message
 * @param u32CMD: command module processed
 * @param pBody: pointer to information body message carried
 * @param u32BodyLen: body information length message carried
 * @param pu32Data: pointer to 32 bits additional data message carried
 * @return: 0 if success
 */
CVI_S32 CVI_MSG_SendSync3(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
	CVI_U32 *pu32Data);

/* CVI_MSG_SendSync4: Send sync messages, this api will block and wait
for the other end's message command to be processed before returning
 *
 * @param u32Module: module which recv this message
 * @param u32CMD: command module processed
 * @param pBody: pointer to information body message carried
 * @param u32BodyLen: body information length message carried
 * @param pstPrivData: pointer to private data message carried
 * @param s32TimeoutMs: wait for api return until this time ended
 * @return: 0 if success
 */
CVI_S32 CVI_MSG_SendSync4(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
	MSG_PRIV_DATA_S *pstPrivData, CVI_S32 s32TimeoutMs);

/* CVI_MSG_SendSync5: Send sync messages for audio, this api will block and wait
for the other end's message command to be processed before returning
 *
 * @param u32Module: module which recv this message
 * @param u32CMD: command module processed
 * @param pBody: pointer to information body message carried
 * @param u32BodyLen: body information length message carried
 * @param pstPrivData: pointer to private data message carried
 * @return: 0 if success
 */
CVI_S32 CVI_MSG_SendSync5(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
					 MSG_PRIV_DATA_S *pstPrivData);

#if __cplusplus
}
#endif

#endif
