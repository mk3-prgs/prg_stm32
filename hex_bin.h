#ifndef	_HEX_BIN_H
#define	_HEX_BIN_H

#include <stdint.h>

int  IsHex(char c);
void  byte_copy(char* dst, char* src, int n);
void BinToHex(uint8_t n, uint16_t addr, uint8_t *src, char *dst);
int HexToBin(uint8_t* dst, char* src, unsigned int lendst);
uint32_t get_part(void);
uint32_t get_start(void);

#endif

