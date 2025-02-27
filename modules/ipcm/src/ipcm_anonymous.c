
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "ipcm_port_common.h"
#include "ipcm_anonymous.h"
#include "linux/ipcm_linux.h"

#define DEV_NAME "/dev/ipcm_anon"

typedef struct _ipcm_anon_ctx {
    int anon_fd;
    pthread_t t_thread_id;
	unsigned int b_stop;
	unsigned int timeout;
    ANON_MSGPROC_FN handler;
    void *handler_data;
} ipcm_anon_ctx;

static ipcm_anon_ctx _anon_ctx = {};

static void *_msg_anon_proc(void *args)
{
	int ret;
	fd_set rfds;
	struct timeval timeout;
    MsgData tmsg;
    ipcm_anon_ctx *ctx = NULL;
	u8 port_type = 0;
	u8 port_id = 0;

    if (args == NULL) {
        ipcm_err("%s args is null.\n", __func__);
        return NULL;
    }

    ctx = (ipcm_anon_ctx *)args;

    while(!ctx->b_stop) {
		timeout.tv_sec  = ctx->timeout / 1000;
		timeout.tv_usec = (ctx->timeout % 1000) * 1000;
		FD_ZERO(&rfds);
		FD_SET(ctx->anon_fd, &rfds);
		ret = select(ctx->anon_fd + 1, &rfds, NULL, NULL, &timeout);
		if (ret == -1) {
			ipcm_err("SELECT error\n");
			break;
		} else if (!ret) {
			ipcm_debug("SELECT timeout\n");
			continue;
		}
		if (FD_ISSET(ctx->anon_fd, &rfds)) {
            ret = read(ctx->anon_fd, &tmsg, sizeof(MsgData));
            if (ret != sizeof(MsgData)) {
                ipcm_err("anon read err ret:%d\n", ret);
                continue;
            }
            ret = ipcm_get_port_id(tmsg.grp_id, &port_type, &port_id);
            if (ret) {
                ipcm_err("ipcm get port type and id fail ret:%d.\n", ret);
                goto rls_pool;
            }
            if (ctx->handler && (port_id < IPCM_ANON_PORT_MAX)) {
                ipcm_anon_msg_t anon_msg;
                anon_msg.port_id = port_id;
                anon_msg.msg_id = tmsg.msg_id;
                anon_msg.data_type = tmsg.func_type;
                if (tmsg.func_type == MSG_TYPE_SHM) {
                    anon_msg.data = ipcm_get_data_by_pos(tmsg.msg_param.msg_ptr.data_pos);
                    anon_msg.size = tmsg.msg_param.msg_ptr.remaining_rd_len;
                    ipcm_inv_data(anon_msg.data, anon_msg.size);
                } else {
                    anon_msg.data = (void *)(unsigned long)tmsg.msg_param.param;
                    anon_msg.size = 4;
                }
                ctx->handler(ctx->handler_data, &anon_msg);
            } else {
                ipcm_warning("anon recv param err, handle(%p), port id(%u) msg id(%u).\n",
                    ctx->handler, port_id, tmsg.msg_id);
            }
rls_pool:
            // release pool buff
            if (tmsg.func_type == MSG_TYPE_SHM) {
                ipcm_release_buff_by_pos(tmsg.msg_param.msg_ptr.data_pos);
            }
		}
    }

    return NULL;
}

s32 ipcm_anon_init(void)
{
    int fd = -1;

	fd = open(DEV_NAME, O_RDWR);
	if (fd < 0) {
		ipcm_err("open %s fail. return:%d!\n", DEV_NAME, fd);
		return fd;
	}
    _anon_ctx.anon_fd = fd;

    _anon_ctx.b_stop = 0;
    _anon_ctx.timeout = 3000;
    pthread_create(&_anon_ctx.t_thread_id, NULL, _msg_anon_proc, &_anon_ctx);
    return 0;
}

s32 ipcm_anon_uninit(void)
{
    _anon_ctx.b_stop = 1;
    pthread_join(_anon_ctx.t_thread_id, NULL);

	if (_anon_ctx.anon_fd)
		close(_anon_ctx.anon_fd);

    return 0;
}

s32 ipcm_anon_send_msg(u8 port_id, u8 msg_id, void *data, u32 len)
{
	MsgData stMsg = {};
	u8 grp_id = 0;
	s32 ret = 0;

	if (data == NULL) {
		ipcm_err("data is null.\n");
		return -EFAULT;
	}
	if (port_id >= IPCM_ANON_PORT_MAX) {
		ipcm_err("port_id(%d max:%d) is invalid.\n", port_id, IPCM_ANON_PORT_MAX-1);
		return -EINVAL;
	}

	ret = ipcm_get_grp_id(PORT_ANON, port_id, &grp_id);
	if (ret) {
		ipcm_err("ipcm get grp id fail ret:%d.\n", ret);
		return ret;
	}

	stMsg.grp_id = grp_id;
	stMsg.msg_id = msg_id;
	stMsg.func_type = MSG_TYPE_SHM;
	ret = ipcm_data_packed(data, len, &stMsg);
	if (ret) {
		ipcm_err("ipcm_data_packed failed ret:%d.\n", ret);
		return ret;
	}

	return write(_anon_ctx.anon_fd, &stMsg, sizeof(MsgData));
}

s32 ipcm_anon_send_param(u8 port_id, u8 msg_id, u32 param)
{
	MsgData tmsg = {};
	u8 grp_id = 0;
	s32 ret = 0;

	if (port_id >= IPCM_ANON_PORT_MAX) {
		ipcm_err("port_id(%u) out of range, max(%u).\n", port_id,
			IPCM_ANON_PORT_MAX - 1);
		return -EINVAL;
	}
	ret = ipcm_get_grp_id(PORT_ANON, port_id, &grp_id);
	if (ret) {
		ipcm_err("ipcm get grp id fail ret:%d.\n", ret);
		return ret;
	}

	tmsg.grp_id = grp_id;
	tmsg.msg_id = msg_id;
	tmsg.func_type = MSG_TYPE_RAW_PARAM;
	tmsg.msg_param.param = param;

    return write(_anon_ctx.anon_fd, &tmsg, sizeof(MsgData));
}

s32 ipcm_anon_register_handle(ANON_MSGPROC_FN handler, void *data)
{
    if (handler) {
        _anon_ctx.handler = handler;
        _anon_ctx.handler_data = data;
        return 0;
    }
    ipcm_err("ipcm_anon_register_handle handler is null.\n");
    return -EINVAL;
}

s32 ipcm_anon_deregister_handle(void)
{
    _anon_ctx.handler = NULL;
    _anon_ctx.handler_data = NULL;
    return 0;
}

void *ipcm_anon_get_user_addr(u32 paddr)
{
    return ipcm_get_user_addr(paddr);
}
