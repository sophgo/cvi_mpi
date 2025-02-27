#include<stdio.h>
#include "cvi_type.h"
#include "cvi_ipcmsg.h"
#include "cvi_msg.h"
#include "msg_client.h"
#include "cvi_common.h"
#include "cvi_sensor.h"

static AHD_Callback User_Callback;

CVI_VOID CVI_SENSOR_AHDRegisterDetect(AHD_Callback CB)
{
    User_Callback = CB;
}

CVI_S32 MSG_SENSOR_SetAHDEnable(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_S32 mode = pstMsg->as32PrivData[0];
	CVI_S32 snsid = pstMsg->as32PrivData[1];

    if (User_Callback) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Sensor %d callback ahd_mode: %d\n", snsid, mode);
        User_Callback(snsid, mode);
    }

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetAHDEnable Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

CVI_S32 CVI_SENSOR_EnableDetect(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, 0, 0);

	stPrivData.as32PrivData[0] = ViPipe;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_AHD, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Enable AHD Thread fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_GPIO_Init(VI_PIPE ViPipe, SNS_I2C_GPIO_INFO_S *pstGpioCfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GPIO_INIT, (CVI_VOID *)pstGpioCfg,
			sizeof(SNS_I2C_GPIO_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "init GPIO fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetAHDMode(VI_PIPE ViPipe, CVI_U32 AhdMode)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = AhdMode;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_AHD_MODE, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set %d AhdMode %d fail!\n", ViPipe, AhdMode);
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_GetAhdStatus(SNS_STATUS_MSG_S *pstStatus)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((pstStatus->s32SnsId < 0) || (pstStatus->s32SnsId >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", pstStatus->s32SnsId);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, pstStatus->s32SnsId, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_STATUS, (CVI_VOID *)pstStatus,
			sizeof(SNS_STATUS_MSG_S), CVI_NULL);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get sensor status fail, ViPipe:%d, s32Ret:%x\n", pstStatus->s32SnsId, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetSnsType(VI_PIPE ViPipe, CVI_U32 SnsType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = SnsType;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_TYPE, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set SnsType fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_GetSnsTypeByName(const char *SnsName, CVI_U32 *SnsType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SNS_TYPE_S SnsType_S = {0, ""};

	if (SnsName == CVI_NULL || SnsType == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "sensor name or sensor id is NULL\n");
		return CVI_FAILURE;
	}
	if (strlen(SnsName) >= MAX_SENSOR_NAME_LENGTH) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "sensor name length_%d is over 100\n", strlen(SnsName));
		return CVI_FAILURE;
	}

	strncpy(SnsType_S.SnsName, SnsName, strlen(SnsName));

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, 0, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_TYPE_BY_NAME, (CVI_VOID *)(&SnsType_S),
			sizeof(SNS_TYPE_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get Sns_Type fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	*SnsType = SnsType_S.SnsType;

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetSnsRxAttr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_RX_ATTR, (CVI_VOID *)pstRxAttr,
			sizeof(RX_INIT_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set combo attr fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetSnsI2c(VI_PIPE ViPipe, CVI_S32 astI2cDev, CVI_S32 s32I2cAddr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = astI2cDev;
	stPrivData.as32PrivData[1] = s32I2cAddr;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_I2C, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set I2C fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetSnsIspAttr(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_ISP_INIT, (CVI_VOID *)pstInitAttr,
			sizeof(ISP_INIT_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set ISP init fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_RegCallback(VI_PIPE ViPipe, ISP_DEV IspDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = IspDev;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_ISP_REG_CB, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Isp RegisterCallback fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_UnRegCallback(VI_PIPE ViPipe, ISP_DEV IspDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = IspDev;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_ISP_UNREG_CB, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Isp RegisterCallback fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetSnsImgMode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *stSnsrMode)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_IMG_MODE, (CVI_VOID *)stSnsrMode,
			sizeof(ISP_CMOS_SENSOR_IMAGE_MODE_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set image mode fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetSnsWdrMode(VI_PIPE ViPipe, WDR_MODE_E wdrMode)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = wdrMode;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_WDR_MODE, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set wdr mode fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_GetSnsRxAttr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *stDevAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_RX_ATTR, (CVI_VOID *)stDevAttr,
			sizeof(SNS_COMBO_DEV_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get rx attr fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetSnsProbe(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_PROBE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set sensor probe fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetSnsGpioInit(CVI_U32 devNo, CVI_U32 u32Rst_port_idx, CVI_U32 u32Rst_pin,
	CVI_U32 u32Rst_pol)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, devNo, 0);

	stPrivData.as32PrivData[0] = u32Rst_port_idx;
	stPrivData.as32PrivData[1] = u32Rst_pin;
	stPrivData.as32PrivData[2] = u32Rst_pol;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_GPIO_INIT, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set gpio init fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_RstSnsGpio(CVI_U32 devNo, CVI_U32 rstEnable)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, devNo, 0);

	stPrivData.as32PrivData[0] = rstEnable;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_RESET_GPIO, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "reset sensor gpio fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_RstMipi(CVI_U32 devNo, CVI_U32 rstEnable)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, devNo, 0);

	stPrivData.as32PrivData[0] = rstEnable;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_RESET_MIPI, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "reset MIPI fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetMipiAttr(VI_PIPE ViPipe, CVI_U32 SnsType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = SnsType;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_MIPI_ATTR, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set Mipi attr fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_EnableSnsClk(CVI_U32 devNo, CVI_U32 clkEnable)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, devNo, 0);

	stPrivData.as32PrivData[0] = clkEnable;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_EN_SNS_CLK, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Enable sns clk fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetSnsStandby(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_STANDBY, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set Sensor standby fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetSnsRestart(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_RESTART, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set Sensor restart fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetSnsInit(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_INIT, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set Sensor Init fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetVIFlipMirrorCB(VI_PIPE ViPipe, VI_DEV ViDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = ViDev;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_FLIPMIRROR_CB, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set mirror & filp callback to vi fail!\n");
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_GetAeDefault(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *stAeDefault)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_AE_DEFAULT, (CVI_VOID *)stAeDefault,
			sizeof(AE_SENSOR_DEFAULT_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get AE default param fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_GetIspBlkLev(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *stBlc)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_BLK_LEVEL, (CVI_VOID *)stBlc,
			sizeof(ISP_CMOS_BLACK_LEVEL_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get AE BLACK level fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetSnsFps(VI_PIPE ViPipe, CVI_U8 fps, AE_SENSOR_DEFAULT_S *stSnsDft)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, fps);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_FPS, (CVI_VOID *)stSnsDft,
			sizeof(AE_SENSOR_DEFAULT_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set sensor fps fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_GetExpRatio(VI_PIPE ViPipe, SNS_EXP_MAX_S *stExpMax)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_EXP_RAT, (CVI_VOID *)stExpMax,
			sizeof(SNS_EXP_MAX_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get exp max and min fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetDgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stDgain)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_DGAIN_CALC, (CVI_VOID *)stDgain,
			sizeof(SNS_GAIN_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set sns dgian cal fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_SENSOR_SetAgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stAgain)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_AGAIN_CALC, (CVI_VOID *)stAgain,
			sizeof(SNS_GAIN_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set sns agian cal fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}