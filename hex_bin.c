
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "hex_bin.h"

//===========================================================================//
int  IsHex(char c)
{
	return(((c>='0')&&(c<='9')) ||
	(c=='a') || (c=='b') || (c=='c') || (c=='d') || (c=='e') || (c=='f') ||
	(c=='A') || (c=='B') || (c=='C') || (c=='D') || (c=='E') || (c=='F'));
}

//===========================================================================//
void  byte_copy(char* dst, char* src, int n)
{
char c;
int i;
	for(i=0;i<n;i++) {
		c = (char)src[i];
		dst[i] = c;
		}
	dst[i] = '\0';
}

static uint32_t start;
static uint32_t part;

uint32_t get_part(void)
{
    return(part);
}

uint32_t get_start(void)
{
    return(start);
}

//===========================================================================//
int HexToBin(uint8_t* buff, char* src, unsigned int lendst)
{
//:10 0000 00 020CB6120CC28F30E530220210130201 2E
unsigned int kol=0;
unsigned int adr=0;
unsigned int cod=0;
//
char str[8];
unsigned int d;
int ret = -1;
unsigned int i;
//
    if(src[0]==':') { // Проверяем формат
        //
        // Извлекаем количество байт в строке
        memset(str, 0, 8);
        byte_copy(str, &src[1], 2);
        sscanf(str,"%x",&kol);
        //
        // Извлекаем адрес
        memset(str, 0, 8);
        byte_copy(str,&src[3],4);
        sscanf(str,"%x",&adr);
        if(adr > lendst-kol) {
            //s.Format(_T("Адресс=0x%04x вне допустимого: 0x0000-0x%04x"),adr+kol,lendst);
            return(-2);
            }
        //
        // Извлекаем код строки
        memset(str, 0, 8);
        byte_copy(str, &src[7], 2);
        sscanf(str,"%x",&cod);
        //
        //
        if(cod == 0) { // строка кода
            //
            for(i=0; i<kol; i++) {
                byte_copy(str,&src[(2*i)+9],2);
                if((adr+i) < lendst) {
                    sscanf(str,"%x",&d);
                    buff[adr+i+(0x10000 * (0x07&part))] = (uint8_t)d;
                    //printf("0x%08x\n", adr+i+(0x10000 * (0x07&part)));
                    }
                else {
                    //s.Format(_T("Адресс=0x%04x за пределом буфера: 0x%04x"),adr+i,lendst);
                    //AfxMessageBox(s);
                    return(-3);
                    }
                //printf("%02x", (uint8_t)d);
                }
            ret = 1;
            }
        else if(cod == 1) {
            // Конец файла
            ret = 0;
            }
        else if(cod == 5) {
            // :04 0000 05 08000000 EF
            // Получаем стартовый адрес сегмента
            memset(str, 0, 8);
            byte_copy(str, &src[9], 8);
            sscanf(str, "%x", &start);
            //printf("Adres = %08x\n", start);
            ret = 5;
            }
        else if(cod == 4) {
            // :02 0000 04 0800 F2
            // Получаем адрес сегмента
            memset(str, 0, 8);
            byte_copy(str, &src[9], 4);
            sscanf(str, "%x", &part);
            //printf("\n  part: %s %04x\n", str, part);
            ret = 4;
            }
        else {
            //s.Format(_T("Не верный формат файла:\r\n"));
            //
            return(-4);
            }
        }
    else {
        return(-5);
        }
	//
	return(ret);
}

void BinToHex(uint8_t n, uint16_t addr, uint8_t *src, char *dst)
{
uint8_t  ks = 0;
int i;
    sprintf(dst, ":%02x%04x00 ", n, (uint16_t)addr); // 9-я┌я▄ п╠п╟п╧я┌ (0...8)
    //
    ks =  (uint8_t)n;
    ks += (uint8_t)(addr>>8);
    ks += (uint8_t)addr;
    //
    for(i=0; i<n; i++) {
        sprintf(&dst[9+2*i], "%02x ", src[i]);
        ks += src[i];
        }
    ks = 0x100 - ks;
    sprintf(&dst[9+2*n], "%02x\n", ks);
    //
    //printf("%s", dst);
    //
}
