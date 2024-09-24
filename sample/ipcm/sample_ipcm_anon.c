
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "cvi_comm_ipcm.h"
#include "cvi_ipcm.h"
#define ANON_PRIV_DATA_MAGIC 0x55aaccdd
#define ANON_TEST_PARAM_DATA 0x4a5a2230

static int SAMPLE_IPCM_ANON_Msg_Process(CVI_VOID *priv, IPCM_ANON_MSG_S *data)
{
	int ret = 0;

	if (priv != (void *)ANON_PRIV_DATA_MAGIC) {
		printf("======anon test fail, reg handle magic err.\n");
		return -1;
	}
	if (data == NULL) {
		printf("======anon test fail, handle data is null.\n");
		return -1;
	}

	if(data->u8DataType == 0) {
        printf("recv msg:\n\t port_id:%d msg_id:%d data_type:%d data_addr:%lx data_len:%d data:0x%llx\n",
            data->u8PortID, data->u8MsgID, data->u8DataType, (unsigned long)data->stData.pData, data->stData.u32Size, 
            *((unsigned long long *)data->stData.pData));
    } else {
        printf("recv msg:\n\t port_id:%d msg_id:%d data_type:%d param:%x\n",
            data->u8PortID, data->u8MsgID, data->u8DataType, data->u32Param);
    }

	return ret;
}

static int SAMPLE_IPCM_ANON_INIT(void)
{
	int ret = 0;
    CVI_U32 boot_stat = 0;

    ret = CVI_IPCM_Init();
    if (ret) {
        printf("======anon test CVI_IPCM_Init fail:%d.\n",ret);
        return ret;
    }

    ret = CVI_IPCM_GetRtosBootStatus(&boot_stat);
    if (ret) {
        printf("======anon test CVI_IPCM_GetRtosBootStatus fail:%d.\n",ret);
    }

    printf("boot_stat:%d\n", boot_stat);

	ret = CVI_IPCM_RegisterAnonHandle(0, SAMPLE_IPCM_ANON_Msg_Process, (void *)(unsigned long)ANON_PRIV_DATA_MAGIC);
	if (ret) {
		printf("======anon test ipcm_anon_register_handle fail:%d.\n",ret);
		CVI_IPCM_AnonUninit();
		return ret;
	}

	ret = CVI_IPCM_AnonInit();
	if (ret) {
		printf("======anon test ipcm_anon_init fail:%d.\n",ret);
		return ret;
	}

	return ret;
}

static int SAMPLE_IPCM_ANON_UNINIT(void)
{
	int ret = 0;

    ret = CVI_IPCM_AnonUninit();
    if (ret) {
        printf("======anon test CVI_IPCM_AnonUninit fail:%d.\n",ret);
    }

    ret = CVI_IPCM_Uninit();
    if (ret) {
        printf("======anon test CVI_IPCM_Uninit fail:%d.\n",ret);
    }

    return ret;
}

static void SAMPLE_ANON_print_help(char *s)
{
    printf("%s -h/--help for help\n", s);
    printf("%s -t/--type DATA_TYPE\n", s);
    printf("\tDATA_TYPE 0:share memory msg 1:param msg\n");
    printf("%s -p/--param PARAM\n", s);
    printf("\t only effect for param msg\n");
}

static int SAMPLE_ANON_Send_Msg(CVI_U8 port_id, CVI_U8 msg_id)
{
    int ret = 0;
    CVI_U32 buf_size = 128;
    void *data;

    data = CVI_IPCM_GetBuff(buf_size);
    if (!data) {
        printf("======anon test fail, ipcm get buf fail.\n");
        return -1;
    }

    // write data to buff
    memset(data, 0x5a + msg_id, buf_size);

    ret = CVI_IPCM_AnonSendMsg(port_id, msg_id, data, buf_size);
    if (ret) {
        printf("======CVI_IPCM_AnonSendMsg send fail ret:%d\n", ret);
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int c = 0;
    int option_index = 0;
    unsigned char data_type = 0;
    CVI_U32 param = ANON_TEST_PARAM_DATA;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"type", required_argument, 0, 't'},
        {"param", required_argument, 0, 'p'},
        {0, 0, 0, 0}
    };

    while((c = getopt_long(argc, argv, "ht:p:", long_options, &option_index)) != -1) {
        switch(c) {
            case 'h':
                SAMPLE_ANON_print_help(argv[0]);
                return 0;
            case 't':
                data_type = atoi(optarg);
                break;
            case 'p':
                param = atoi(optarg);
                break;
            case '?':
                break;
            default:
                break;
        }
    }

    printf("data_type:%d param:0x%x\n", data_type, param);

    SAMPLE_IPCM_ANON_INIT();

    if (0 == data_type) {
        SAMPLE_ANON_Send_Msg(0, 0);
        SAMPLE_ANON_Send_Msg(CVI_IPCM_ANON_KER_PORT_ST, 0);
    } else if (1 == data_type) {
        CVI_IPCM_AnonSendParam(0, 0, param);
        CVI_IPCM_AnonSendParam(CVI_IPCM_ANON_KER_PORT_ST, 0, param);
    } else {
        SAMPLE_ANON_Send_Msg(0, 0);
        SAMPLE_ANON_Send_Msg(CVI_IPCM_ANON_KER_PORT_ST, 0);
        CVI_IPCM_AnonSendParam(0, 0, param);
        CVI_IPCM_AnonSendParam(CVI_IPCM_ANON_KER_PORT_ST, 0, param);
    }

    // wait for recv msg
    usleep(1000*1000);

    // uninit
    SAMPLE_IPCM_ANON_UNINIT();

    return 0;
}

