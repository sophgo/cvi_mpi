#include "cvi_sys_base.h"

#include "isp_control.h"
#include "isp_debug.h"
#include "isp_ioctl.h"
#include "isp_3a.h"

#include "isp_comm_inc.h"

CVI_S32 isp_control_get_scene_info(VI_PIPE ViPipe, enum ISP_SCENE_INFO *scene)
{
	ISP_CTX_S *pstIspCtx = NULL;

	ISP_GET_CTX(ViPipe, pstIspCtx);

	*scene = pstIspCtx->scene;

	return CVI_SUCCESS;
}
