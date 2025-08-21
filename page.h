#ifndef	_PAGE_H
#define	_PAGE_H	1

#include <stdint.h>

int  read_page(int page, uint8_t *bf);
int  write_page(int page, uint8_t *bf);
void print_page(int fd, int page);
void print_buff(uint8_t *bf);

#endif
