#include "cmd.h"
#include "page.h"

extern stm32_dev_t *dev;

/*
int read_page(int page, uint8_t *bf)
{
int n;
uint32_t addr;
int ret;
    //
    for(n=0; n<4; n++) {
        addr = 0x08000000 + (page * 0x400) + (0x100 * n);
        ret = cmd_ReadMem(addr, 256, bf);
        if(ret) break;
        //
        }
    //
    return(ret);
    //
}

int write_page(int page, uint8_t *bf)
{
int n;
uint32_t addr;
int ret;
    //
    for(n=0; n<4; n++) {
        addr = 0x08000000 + (page * 0x400) + (0x100 * n);
        ret = cmd_WriteMem(addr, 256, bf);
        if(ret) break;
        //
        }
    //
    return(ret);
    //
}
*/

void print_page(int fd, int page)
{
uint8_t bf[256];
int i, n;
uint32_t addr;
uint32_t p0x400 = dev->fl_ps[0];
    //
    printf("Page: %03d [0x%04x - 0x%04x]", page, p0x400*page, p0x400*page+p0x400-1);

    for(n=0; n<(p0x400/0x100); n++) {
        for(i=0; i<256; i++) bf[i] = 0;
        addr = 0x08000000 + (page * p0x400) + (0x100 * n);
        cmd_ReadMem(fd, addr, 256, bf);
        //
        addr = 0x08000000 + (page * p0x400) + 256*n;
        for(i=0; i<256; i++) {
            if((i%32) == 0) { printf("\n%08x: ", addr); addr += 32; }
            printf("%02x", bf[i]);
            }
        }
    //
    printf("\n");
    //
}

void print_buff(uint8_t *bf)
{
int i;
    //
    printf("\n");
    for(i=0; i<0x400; i++) {
        if((i%32) == 0) printf("\n");
        printf("%02x ", bf[i]);
        }
    printf("\n");
    //
}

