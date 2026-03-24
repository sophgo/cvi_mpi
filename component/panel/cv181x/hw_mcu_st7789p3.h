#ifndef _MCU_PARAM_ST7789P3_H_
#define _MCU_PARAM_ST7789P3_H_

#include <linux/cvi_comm_vo.h>

#define COMMAND 0
#define DATA	1

const VO_HW_MCU_CFG_S st7789p3Cfg = {
	.pins = {
		.pin_num = 11,
		.d_pins = {
			{VO_VIVO_D4, VO_MUX_MCU_DATA0},
			{VO_VIVO_D5, VO_MUX_MCU_DATA1},
			{VO_VIVO_D6, VO_MUX_MCU_DATA2},
			{VO_VIVO_D7, VO_MUX_MCU_DATA3},
			{VO_VIVO_CLK, VO_MUX_MCU_DATA4},
			{VO_VIVO_D8, VO_MUX_MCU_DATA5},
			{VO_VIVO_D9, VO_MUX_MCU_DATA6},
			{VO_VIVO_D10, VO_MUX_MCU_DATA7},
			{VO_VIVO_D3, VO_MUX_MCU_RD},
			{VO_VIVO_D2, VO_MUX_MCU_WR},
			// {VO_VIVO_D1, VO_MUX_MCU_CS},//可不接
			{VO_VIVO_D0, VO_MUX_MCU_RS},
		}
	},
	.mode = VO_MCU_MODE_RGB565,
	.lcd_power_gpio_num = GPIOA_01,
	.lcd_power_avtive = GPIO_ACTIVE_HIGH,
	.backlight_gpio_num = GPIOA_04,
	.backlight_avtive = GPIO_ACTIVE_HIGH,
	.reset_gpio_num = GPIOE_21,
	.reset_avtive = GPIO_ACTIVE_LOW,
	.instrs = {
		.instr_num = 75,
		.instr_cmd = {
			{.delay = 0,   .data_type = COMMAND, .data = 0x11},
			{.delay = 0,   .data_type = COMMAND, .data = 0xB2},
			{.delay = 0,   .data_type = DATA,    .data = 0x0C},
			{.delay = 0,   .data_type = DATA,    .data = 0x0C},
			{.delay = 0,   .data_type = DATA,    .data = 0x00},
			{.delay = 0,   .data_type = DATA,    .data = 0x33},
			{.delay = 0,   .data_type = DATA,    .data = 0x33},

			{.delay = 0,   .data_type = COMMAND, .data = 0x35},
			{.delay = 0,   .data_type = DATA,    .data = 0x00},

			{.delay = 0,   .data_type = COMMAND, .data = 0x36},
			{.delay = 0,   .data_type = DATA,    .data = 0x40},

			{.delay = 0,   .data_type = COMMAND, .data = 0x3A},
			{.delay = 0,   .data_type = DATA,    .data = 0x05},

			{.delay = 0,   .data_type = COMMAND, .data = 0xB7},
			{.delay = 0,   .data_type = DATA,    .data = 0x76},

			{.delay = 0,   .data_type = COMMAND, .data = 0xBB},
			{.delay = 0,   .data_type = DATA,    .data = 0x1a},

			{.delay = 0,   .data_type = COMMAND, .data = 0xC0},
			{.delay = 0,   .data_type = DATA,    .data = 0x2C},

			{.delay = 0,   .data_type = COMMAND, .data = 0xC2},
			{.delay = 0,   .data_type = DATA,    .data = 0x01},

			{.delay = 0,   .data_type = COMMAND, .data = 0xC3},
			{.delay = 0,   .data_type = DATA,    .data = 0x13},

			{.delay = 0,   .data_type = COMMAND, .data = 0xC4},
			{.delay = 0,   .data_type = DATA,    .data = 0x20},

			{.delay = 0,   .data_type = COMMAND, .data = 0xC6},
			{.delay = 0,   .data_type = DATA,    .data = 0x0F},

			{.delay = 0,   .data_type = COMMAND, .data = 0xD0},
			{.delay = 0,   .data_type = DATA,    .data = 0xA4},
			{.delay = 0,   .data_type = DATA,    .data = 0xA1},

			{.delay = 0,   .data_type = COMMAND, .data = 0xD6},
			{.delay = 0,   .data_type = DATA,    .data = 0xA1},

			{.delay = 0,   .data_type = COMMAND, .data = 0xE0},
			{.delay = 0,   .data_type = DATA,    .data = 0xF0},
			{.delay = 0,   .data_type = DATA,    .data = 0x03},
			{.delay = 0,   .data_type = DATA,    .data = 0x09},
			{.delay = 0,   .data_type = DATA,    .data = 0x11},
			{.delay = 0,   .data_type = DATA,    .data = 0x13},
			{.delay = 0,   .data_type = DATA,    .data = 0x0D},
			{.delay = 0,   .data_type = DATA,    .data = 0x38},
			{.delay = 0,   .data_type = DATA,    .data = 0x44},
			{.delay = 0,   .data_type = DATA,    .data = 0x4A},
			{.delay = 0,   .data_type = DATA,    .data = 0x07},
			{.delay = 0,   .data_type = DATA,    .data = 0x10},
			{.delay = 0,   .data_type = DATA,    .data = 0x0F},
			{.delay = 0,   .data_type = DATA,    .data = 0x17},
			{.delay = 0,   .data_type = DATA,    .data = 0x1A},

			{.delay = 0,   .data_type = COMMAND, .data = 0xE1},
			{.delay = 0,   .data_type = DATA,    .data = 0xF0},
			{.delay = 0,   .data_type = DATA,    .data = 0x01},
			{.delay = 0,   .data_type = DATA,    .data = 0x06},
			{.delay = 0,   .data_type = DATA,    .data = 0x0C},
			{.delay = 0,   .data_type = DATA,    .data = 0x0D},
			{.delay = 0,   .data_type = DATA,    .data = 0x18},
			{.delay = 0,   .data_type = DATA,    .data = 0x39},
			{.delay = 0,   .data_type = DATA,    .data = 0x54},
			{.delay = 0,   .data_type = DATA,    .data = 0x4A},
			{.delay = 0,   .data_type = DATA,    .data = 0x3F},
			{.delay = 0,   .data_type = DATA,    .data = 0x1C},
			{.delay = 0,   .data_type = DATA,    .data = 0x1B},
			{.delay = 0,   .data_type = DATA,    .data = 0x21},
			{.delay = 0,   .data_type = DATA,    .data = 0x25},

			{.delay = 0,   .data_type = COMMAND, .data = 0x21},

			{.delay = 0,   .data_type = COMMAND, .data = 0x29},

			{.delay = 0,   .data_type = COMMAND, .data = 0x2A},    //Column Address Set
			{.delay = 0,   .data_type = DATA,    .data = 0x00},
			{.delay = 0,   .data_type = DATA,    .data = 0x00},  //0
			{.delay = 0,   .data_type = DATA,    .data = 0x00},
			{.delay = 0,   .data_type = DATA,    .data = 0xEF},  //239

			{.delay = 0,   .data_type = COMMAND, .data = 0x2B},    //Row Address Set
			{.delay = 0,   .data_type = DATA,    .data = 0x00},
			{.delay = 0,   .data_type = DATA,    .data = 0x00},  //0
			{.delay = 0,   .data_type = DATA,    .data = 0x01},
			{.delay = 0,   .data_type = DATA,    .data = 0x3F},  //319

			{.delay = 0,   .data_type = COMMAND, .data = 0x2C},
		}
	},
};

#endif // _MCU_PARAM_ST7789V_H_
