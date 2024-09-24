#ifndef _CVI_AUDIO_PCM_H_
#define _CVI_AUDIO_PCM_H_

CVI_S32 cvi_pcm_open(CVI_U8 card_id, CVI_U8 dev_id, AIO_ATTR_S *pstAioAttrs, CVI_U8 inoutflag);
CVI_S32 cvi_pcm_read(CVI_U8 card_id, CVI_VOID * addr, CVI_U32 bytesLen);
CVI_S32 cvi_pcm_write(CVI_U8 card_id, CVI_VOID * addr, CVI_U32 bytesLen);
CVI_S32 cvi_pcm_close(CVI_U8 card_id);
CVI_S32 cvi_aud_set_volume(CVI_U8 card_id, CVI_U8 dev_id, CVI_S32 volume);
CVI_S32 cvi_aud_get_volume(CVI_U8 card_id, CVI_U8 dev_id, CVI_S32* volume);
CVI_S32 cvi_aud_set_mute(CVI_U8 card_id, CVI_U8 dev_id, CVI_S32 enable);
CVI_S32 cvi_aud_get_mute(CVI_U8 card_id, CVI_U8 dev_id, CVI_S32* enable);

#endif