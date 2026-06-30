#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "cvi_system.h"
#include "cvi_uvc.h"
#include "cvi_uvc_gadget.h"
#include "sample_comm.h"


void sig_handler(int signo)
{
	if (SIGTERM == signo || SIGINT == signo)
		uvc_exit();

	exit(0);
}

int main(int argc, char const *argv[])
{
	int ret;
	sem_t exitsem;

	sem_init(&exitsem, 0, 0);

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);

	ret = uvc_init();
	if(ret != 0)
		printf("uvc_init failed\n");

	while ((0 != sem_wait(&exitsem)) && (errno == EINTR));

	return 0;
}
