// SPDX-License-Identifier: GPL-2.0-only
/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "panel_spi.h"
#include "sample_comm.h"

void send_gpio_spi_9bit(int dc, unsigned char data) {
	int bit_per_word = 9;
	int i;
	int bit;

	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOE_18, 0); //spi_cs
	for (i = 0; i < bit_per_word; i++) {
		if (i == 0) {
			bit = (dc & 0x1);
		} else {
			bit = (data >> (bit_per_word - 1 - i)) & 1;
		}
		SAMPLE_COMM_GPIO_SetValue(CVI_GPIOE_22, bit);  //spi_sdo

		SAMPLE_COMM_GPIO_SetValue(CVI_GPIOE_23, 1);  // spi_sck high
		SAMPLE_COMM_GPIO_SetValue(CVI_GPIOE_23, 0);  //spi_sck low
	}
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOE_18, 1); //spi_cs
}

int panel_spi_sendData(PANEL_INSTR_S init_cmds[], int size)
{
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOA_28, 1);//reset
	usleep(100 * 1000);
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOA_28, 0);
	usleep(100 * 1000);
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOA_28, 1);
	usleep(120 * 1000);

	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOE_18, 1); //spi_cs
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOE_23, 0); //spi_sck
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOE_22, 1);  //spi_sdo

	for (int i = 0; i < size; i++) {

		if(init_cmds[i].data_type == PANEL_DATA) {
			send_gpio_spi_9bit(1, init_cmds[i].data);
		} else {
			send_gpio_spi_9bit(0, init_cmds[i].data);
		}

		if (init_cmds[i].delay)
			usleep(init_cmds[i].delay * 1000);
	}

	return 0;
}

