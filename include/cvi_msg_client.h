#ifndef __CVI_MSG_CLIENT_H__
#define __CVI_MSG_CLIENT_H__
#if __cplusplus
extern "C" {
#endif

/* CVI_MSG_Init: dual os message communication system initialization
 *
 * @return: 0 if success
 */
CVI_S32 CVI_MSG_Init(CVI_VOID);

/* CVI_MSG_Deinit: dual os message communication system deinitialization
 *
 * @return: 0 if success
 */
CVI_S32 CVI_MSG_Deinit(CVI_VOID);

#if __cplusplus
}
#endif

#endif
