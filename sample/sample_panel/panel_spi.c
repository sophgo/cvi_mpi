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
#include "cvi_common.h"

void send_gpio_spi(unsigned char data) {
	int bit_per_word = 8;
	int i;
	int bit;

	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOC_03, 0); //spi_cs
	for (i = 0; i < bit_per_word; i++) {
		bit = (data >> (bit_per_word - 1 - i)) & 1;
		if (bit == 0) {
			SAMPLE_COMM_GPIO_SetValue(CVI_GPIOC_05, 0);  //spi_mosi
		} else {
			SAMPLE_COMM_GPIO_SetValue(CVI_GPIOC_05, 1);  //spi_mosi
		}
		SAMPLE_COMM_GPIO_SetValue(CVI_GPIOC_02, 1);  // spi_sck high
		SAMPLE_COMM_GPIO_SetValue(CVI_GPIOC_02, 0);  //spi_sck low
	}
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOC_03, 1); //spi_cs
}

int panel_spi_sendData(PANEL_INSTR_S prgb_st7789v2_init_cmds[], int size)
{
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOC_18, 1);//reset
	usleep(100 * 1000);
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOC_18, 0);
	usleep(100 * 1000);
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOC_18, 1);
	usleep(120 * 1000);

	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOC_03, 1); //spi_cs
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOC_02, 1); //spi_sck
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOC_05, 1);  //spi_mosi

	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOA_20, 1); //rd
	SAMPLE_COMM_GPIO_SetValue(CVI_GPIOA_01, 1); //wrx

	for (int i = 0; i < size; i++) {
		if (prgb_st7789v2_init_cmds[i].data_type == PANEL_COMM) {
			SAMPLE_COMM_GPIO_SetValue(CVI_GPIOA_01, 0); //wrx
			send_gpio_spi(prgb_st7789v2_init_cmds[i].data);
		} else {
			SAMPLE_COMM_GPIO_SetValue(CVI_GPIOA_01, 1); //wrx
			send_gpio_spi(prgb_st7789v2_init_cmds[i].data);
		}

		if (prgb_st7789v2_init_cmds[i].delay)
			usleep(prgb_st7789v2_init_cmds[i].delay * 1000);
	}
		SAMPLE_COMM_GPIO_SetValue(CVI_GPIOA_01, 1); //wrxX

	return 0;
}