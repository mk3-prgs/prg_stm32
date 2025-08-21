#ifndef	_CMD_H
#define	_CMD_H	1

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define ACK 0x79
#define NOACK 0x1f

typedef struct stm32_dev {
	uint16_t	id;
	const char	*name;
	uint32_t	ram_start, ram_end;
	uint32_t	fl_start, fl_end;
	uint16_t	fl_pps;  // pages per sector
	uint32_t	*fl_ps;  // page size
	uint32_t	opt_start, opt_end;
	uint32_t	mem_start, mem_end;
	uint32_t	flags;
} stm32_dev_t;

int wait_ack(int fd, char *str);

int cmd_Get(int fd);

int cmd_GetVers(int fd);

int cmd_GetID(int fd, uint32_t* sz_pg, uint32_t *len_pg);

int cmd_ReadMem(int fd, uint32_t addr, uint16_t len, uint8_t *bf);

int cmd_WriteMem(int fd, uint32_t addr, uint16_t len, const uint8_t *bf);

int cmd_GO(int fd, uint32_t addr);

int cmd_ErMem(int fd, uint8_t numbers_of_pages, uint8_t pages);

int cmd_ExErMem(int fd, uint16_t numb, uint16_t page);

int cmd_erase_pages(int fd, uint8_t numbers_of_pages, uint8_t *pages);

#endif
