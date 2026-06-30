#ifndef _RGB666_FRD280C50106A_H_
#define _RGB666_FRD280C50106A_H_

#include <cvi_comm_vo.h>
#include "../../../sample/sample_panel/panel_spi.h"

VO_RGB_ATTR_S stFRD280C50106A = {
	.pins = {
		.pin_num = 22,
		.d_pins = {
			{VO_MIPI_TXP4, VO_MUX_RGB_VS},
			{VO_MIPI_RXN2, VO_MUX_RGB_HS},
			{VO_MIPI_RXP1, VO_MUX_RGB_HDE},
			{VO_MIPI_RXN1, VO_MUX_RGB_DATA23},
			{VO_MIPI_RXP0, VO_MUX_RGB_DATA22},
			{VO_MIPI_RXN0, VO_MUX_RGB_DATA21},
			{VO_MIPI_TXM3, VO_MUX_RGB_DATA20},
			{VO_MIPI_TXP3, VO_MUX_RGB_DATA19},
			{VO_MIPI_TXM2, VO_MUX_RGB_DATA18},
			{VO_MIPI_TXP2, VO_MUX_RGB_CLK},
			{VO_MIPI_TXM1, VO_MUX_RGB_DATA15},
			{VO_MIPI_TXP1, VO_MUX_RGB_DATA14},
			{VO_MIPI_TXM0, VO_MUX_RGB_DATA13},
			{VO_MIPI_TXP0, VO_MUX_RGB_DATA12},
			{VO_VIVO_CLK, VO_MUX_RGB_DATA11},
			{VO_VIVO_D0, VO_MUX_RGB_DATA10},
			{VO_VIVO_D1, VO_MUX_RGB_DATA7},
			{VO_VIVO_D2, VO_MUX_RGB_DATA6},
			{VO_VIVO_D3, VO_MUX_RGB_DATA5},
			{VO_VIVO_D4, VO_MUX_RGB_DATA4},
			{VO_VIVO_D5, VO_MUX_RGB_DATA3},
			{VO_VIVO_D6, VO_MUX_RGB_DATA2}
		}
	},
};

//st7789v2 prgb initial commands
PANEL_INSTR_S prgb_st7789v2_init_cmds[] = {
	{.delay = 120, .data_type = PANEL_COMM,  .data = 0x11},
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0x36},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0x3A},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x06},//0x101 = 16 bit/pixel, 0x110 = 18bit/pixel
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xB0},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x11},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0xF0},//parallel rgb
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xB1},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0xC2},
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0x04},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x14},
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xB2},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x0C},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x0C},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x33},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x33},
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xB7},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x75},//VGH=14.97V, VGL=-10.43V
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xBB},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x2B},//Vcom 0x1F: 0.875, 0x31: 1.35V
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xC0},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x2C},
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xC2},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x01},
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xC3},//GVDD
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x0B},
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xC4},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x20},//VDV, 0x20:0v
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xC6},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x0F},//Frame Rate control 0F: 60hz
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xD0},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0xA4},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0xA1},
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xD6},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0xA1},
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xE0},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0xD0},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x01},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x04},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x09},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x0B},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x07},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x2E},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x44},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x43},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x0B},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x16},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x15},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x17},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x1D},
	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0xE1},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0xD0},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x01},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x05},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x0A},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x0B},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x08},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x2F},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x44},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x41},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x0A},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x15},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x14},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x19},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x1D},

	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0x2A},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0xEF},  //240

	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0x2B},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x01},
	{.delay = 0,   .data_type = PANEL_DATA,	 .data = 0x40},  //320

	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0x29},

	{.delay = 0,   .data_type = PANEL_COMM,	 .data = 0x2C},
};

#endif // _RGB666_FRD280C50106A_H_