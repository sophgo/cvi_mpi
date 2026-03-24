#ifndef __PANEL_SPI_H__
#define __PANEL_SPI_H__

#include <stdint.h>

enum panel_op {
	PANEL_COMM,
	PANEL_DATA,
};

typedef struct _PANEL_SPI_INSTR_S {
	uint8_t	delay;
	uint8_t	data_type;
	uint8_t	data;
} PANEL_INSTR_S;

int panel_spi_init(void);
int panel_spi_sendData(PANEL_INSTR_S init_cmds[], int size);

#endif // __PANEL_SPI_H__