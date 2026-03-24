#ifndef _3SRGB_GC9307_H_
#define _3SRGB_GC9307_H_

#include <linux/cvi_comm_vo.h>
#include "../../../sample/sample_panel/panel_spi.h"

const VO_SRGB_ATTR_S stGC9307Cfg = {
	.pins = {
		.pin_num = 10,
		.d_pins = {
			{VO_VIVO_D3, VO_MUX_SRGB_DATA0},
			{VO_VIVO_D4, VO_MUX_SRGB_DATA1},
			{VO_VIVO_D5, VO_MUX_SRGB_DATA2},
			{VO_VIVO_D6, VO_MUX_SRGB_DATA3},
			{VO_VIVO_D7, VO_MUX_SRGB_DATA4},
			{VO_VIVO_D8, VO_MUX_SRGB_DATA5},
			{VO_VIVO_D2, VO_MUX_SRGB_VS},
			{VO_VIVO_D1, VO_MUX_SRGB_HS},
			{VO_VIVO_D0, VO_MUX_SRGB_HDE},
			{VO_VIVO_CLK, VO_MUX_SRGB_CLK}
		}
	},
};

//3srgb initial commands
PANEL_INSTR_S srgb_gc9307_init_cmds[] = {
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xfe},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xef},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x36},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x48},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x3a},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x66},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x85},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0xc0},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x86},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x98},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x87},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x28},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x89},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x33},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x8b},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x84},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x8d},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x3b},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x8e},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x0f},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x8f},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x70},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xe8},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x13},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x17},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xec},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x57},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x07},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0xff},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xed},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x18},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x09},

	{.delay = 0, .data_type = PANEL_COMM, .data = 0xc3},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x29},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xc4},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x45},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xc9},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x10},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xff},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x61},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x99},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x3e},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x9d},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x4b},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x98},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x3e},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x9c},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x4b},
	// SYNC MODE 配置
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x3a},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x66},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x84},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x61},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x8a},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x40},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xf6},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0xc7},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xb0},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x40}, // 0x40: DE MODE，0x60: SYNC MODE
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xb5},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x02}, // vfp
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x05}, // vbp
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x12}, // hbp

	// Gamma/电压相关
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xF0},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x10},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x15},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x09},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x09},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x07},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x2f},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xF1},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x42},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x73},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x6F},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x36},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x35},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x2F},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xF2},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x12},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x10},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x0A},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x09},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x06},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x2f},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xF3},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x42},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x73},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x6F},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x36},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x35},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x2F},

	{.delay = 0, .data_type = PANEL_COMM, .data = 0xfa},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x80},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x0f},

	{.delay = 0, .data_type = PANEL_COMM, .data = 0xbe},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x11},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0xcb},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x02},

	{.delay = 0, .data_type = PANEL_COMM, .data = 0xcd},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x22},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x9b},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0xff},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x2a},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x00},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x22},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x00},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0xcd},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x2b},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x00},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x00},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x01},
	{.delay = 0, .data_type = PANEL_DATA, .data = 0x3f},

//	{.delay = 0, .data_type = PANEL_COMM, .data = 0x8a},//Bist mode
//	{.delay = 0, .data_type = PANEL_DATA, .data = 0x80},
//	{.delay = 0, .data_type = PANEL_COMM, .data = 0xf7},
//	{.delay = 0, .data_type = PANEL_DATA, .data = 0x20},
//	{.delay = 0, .data_type = PANEL_DATA, .data = 0x3f},
//  {.delay = 0, .data_type = PANEL_DATA, .data = 0x00},
//	{.delay = 0, .data_type = PANEL_DATA, .data = 0x00},

	{.delay = 120, .data_type = PANEL_COMM, .data = 0x11},
	{.delay = 20, .data_type = PANEL_COMM, .data = 0x29},
	{.delay = 0, .data_type = PANEL_COMM, .data = 0x2c},
};

#endif // _3SRGB_GC9307_H_
