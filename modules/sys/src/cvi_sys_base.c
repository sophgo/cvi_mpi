#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <sys/queue.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

#include <getopt.h>		/* getopt_long() */

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <sys/prctl.h>

#include "cvi_sys_base.h"
#include "cvi_buffer.h"


static CVI_S32 sys_fd = -1;

int open_device(const char *dev_name, CVI_S32 *fd)
{
	struct stat st;

	*fd = open(dev_name, O_RDWR /* required */  | O_NONBLOCK | O_CLOEXEC, 0);
	if (-1 == *fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno,
			strerror(errno));
		return -1;
	}

	if (-1 == fstat(*fd, &st)) {
		close(*fd);
		fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name,
			errno, strerror(errno));
		return -1;
	}

	if (!S_ISCHR(st.st_mode)) {
		close(*fd);
		fprintf(stderr, "%s is no device\n", dev_name);
		return -ENODEV;
	}
	return 0;
}

CVI_S32 close_device(CVI_S32 *fd)
{
	if (*fd == -1)
		return -1;

	if (-1 == close(*fd)) {
		fprintf(stderr, "%s: fd(%d) failure\n", __func__, *fd);
		return -1;
	}

	*fd = -1;

	return CVI_SUCCESS;
}

CVI_S32 sys_dev_open(CVI_VOID)
{
	if (sys_fd != -1) {
		CVI_TRACE_SYS(CVI_DBG_INFO, "sys dev has already opened\n");
		return CVI_SUCCESS;
	}

	if (open_device(SYS_DEV_NAME, &sys_fd) != 0) {
		perror("sys open failed");
		sys_fd = -1;
		return CVI_ERR_SYS_NOTREADY;
	}
	return CVI_SUCCESS;
}

CVI_S32 sys_dev_close(CVI_VOID)
{
	if (sys_fd == -1) {
		CVI_TRACE_SYS(CVI_DBG_INFO, "sys dev is not opened\n");
		return CVI_SUCCESS;
	}

	close_device(&sys_fd);
	sys_fd = -1;
	return CVI_SUCCESS;
}

CVI_S32 get_sys_fd(CVI_VOID)
{
	if (sys_fd == -1)
		sys_dev_open();
	return sys_fd;
}

long get_diff_in_us(struct timespec t1, struct timespec t2)
{
	struct timespec diff;

	if (t2.tv_nsec-t1.tv_nsec < 0) {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
	} else {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
	}
	return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}

