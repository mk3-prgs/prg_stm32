//modified
#include <sys/poll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>

#include "io.h"
#include "hex_bin.h"
#include "cmd.h"
#include "page.h"

uint32_t IOBF_LEN=(0x80000);

int m_info=1;
extern stm32_dev_t *dev;

extern stm32_dev_t *dev;
extern uint32_t m_RESET;
extern uint32_t m_BOOT;

extern port_t s_ctx;

void hex_dump(uint8_t bf[], int len)
{
int i;
    for(i=0; i<len; i++) {
        if((i%16)==0) printf("\n:%08x ", i);
        printf("%02x", bf[i]);
        }
    printf("\n");

}

int test_ff(uint32_t len, uint8_t *bf)
{
int ret;
uint32_t i;
    //
    ret = 0;
    //
    for(i=0; i<len; i++) {
        if(bf[i] != 0xff) {
            ret = -1;
            //printf("\ni=%04x bf[i]=%02x ", i ,bf[i]);
            break;
            }
        }
    //
    return(ret);
}

static uint8_t tmp_bf[8192];

int main(int argc, char *argv[])
{
uint8_t *io_bf;
uint8_t *pt;
char c;
char str[256];
char filename[256];
char device[256];
char command[512];
//
FILE *file;
int f_bin=0;
int cmd_wr=0;
int erase_all=0;
int erase_ex=0;
int print_pages=0;
int n_page[256];
int m_verify=0;
int m_read=0;
    //===================
    //
int i;
int ret=0;
int vrf=0;
uint32_t part = 0x00000000;
uint32_t addr = 0;
//uint16_t len = 0;
port_t* ctx = &s_ctx;
int run=0;
//
uint32_t page=0;
uint32_t n_pages=0;
    //
    ctx->s = -1;
    //
    //strcpy(device, "/dev/ttymxc1");
    strcpy(device, "/dev/ttyUSB0");
    sprintf(command, "/usr/bin/picocom --imap lfcrlf -b 115200 -d8 -p1 -fn -l %s", device);
    //
    for(i=0; i<256; i++) { filename[i] = 0; n_page[i] = -1;}

    erase_all = 0;

    if(argc < 2) {
        printf("Ключи программы:\n");
        printf("    -p Порт. [%s]\n", device);
        printf("    -e Стереть все. [Стирать только под запись]\n");
        printf("    -w Запись.\n");
        printf("    -r Чтение.\n");
        printf("    -x Вариант стирания.\n");
        printf("    -l [страница] Дамп flash.\n");
        printf("    -f Файл для записи [.hex]\n");
        printf("    -b Файл для записи [.bin]\n");
        printf("    -t Выполнить команду [picocom -b 115200 /dev/ttyXXX]\n");
        printf("    -R Инвертировать RESET\n");
        printf("    -B Инвертировать BOOT\n");
        printf("    -f Имя файла.\n");
        printf("    -i Вывод доп. информации\n");
        //
        exit(0);
        }
    //
    //printf("\n");
    //printf("argc = %d\n", argc);
    //
    for(i=1; i < argc; i++) {
        //printf("%d  %s\n", i, argv[i]);
        //
        if(strcmp("-f", argv[i]) == 0) { f_bin=0; strcpy(filename, argv[i+1]); i++;}
        else if(strcmp("-b", argv[i]) == 0) { f_bin=1; strcpy(filename, argv[i+1]); i++;}
        else if(strcmp("-p", argv[i]) == 0) {
            strcpy(device, argv[i+1]); i++;
            sprintf(command, "/usr/bin/picocom --imap lfcrlf -b 115200 -d8 -p1 -fn -l %s", device);
            }
        else if(strcmp("-r", argv[i]) == 0) m_read  = 1;
        else if(strcmp("-v", argv[i]) == 0) m_verify  = 1;
        else if(strcmp("-w", argv[i]) == 0) cmd_wr = 1;
        else if(strcmp("-e", argv[i]) == 0) erase_all = 1;
        else if(strcmp("-x", argv[i]) == 0) erase_ex = 1;
        else if(strcmp("-R", argv[i]) == 0) m_RESET = 1;
        else if(strcmp("-B", argv[i]) == 0) m_BOOT  = 0;
        else if(strcmp("-t", argv[i]) == 0) {
            if((i+1) < argc) {
                if(*argv[i+1] != '-') {strcpy(command, argv[i+1]); i++;}
                }
            vrf = 1;
            }
        else if(strcmp("-l", argv[i]) == 0) {
            print_pages = 1;

            while((i+1) < argc) {
                if(isdigit(*argv[i+1])) {sscanf(argv[i+1], "%d", &n_page[i-1]); i++;}
                else { n_page[i-1]=-1; break; }
                }
            }
        }
    //for(i=0;i<256;i++) {
    //    if(n_page[i] != -1) printf("n_page[%d] = %d\n", i, n_page[i]);
    //    }
    /*
    printf("\n");
    printf("filename = %s\n", filename);
    printf("device = %s\n", device);
    printf("ease_all = %d\n", erase_all);
    printf("vrf = %d\n", vrf);
    printf("erase_ex = %d\n", erase_ex);
    printf("print_page = %d n_page = %d\n", print_pages, n_page);
    printf("command = %s\n", command);
    printf("\n");
    exit(0);
    */
    //
    //en_prg(1);
    //
    ctx->debug = 1;
    //
    strcpy(ctx->device, device);
    if(L475) {
        ctx->baud = 115200;
        }
    else {
        ctx->baud = 115200;
        }
    ctx->data_bit = 8;
    ctx->stop_bit = 1;
    ctx->parity = 'E';
    //
    if(p_connect(ctx) != 0) {
        ret = 1;
        printf("Ошибка подключения через %s.\n", device);
        return(0);
        }
    //

    //
    ret = p_reset(ctx);
    if(ret < 0) {
        ret = 2;
        if(ctx->s > 0) p_close(ctx);
        printf("Ошибка сброса.\n");
        return(ret);
        }
    //
    //p_close(ctx);
    //return(ret);
    //
    ret = cmd_Get(ctx->s);
    if(ret) return(-1);
    //
    //ret = cmd_GetVers();
    //
    ret = cmd_GetID(ctx->s, &page, &n_pages);
    if(ret) return(-1);
    IOBF_LEN = page * n_pages;
    printf("page=%d n_pages=%d io_bf: %d bytes\n", page, n_pages, IOBF_LEN);
    //
    page = 0x100;
    //return(0);
    /*
    uint8_t opt_bf[16];
    ret = cmd_ReadMem(0x1FFFF800, 16, opt_bf);
    if(ret > 0) {
        printf("Option bytes[%d]:\n", ret);
        for(i=0; i<ret; i++) printf(" 0x%02x", opt_bf[i]);
        printf("\n");
        //
        uint64_t* pt=(uint64_t*)opt_bf;
        printf(" 0x%08x 0x%08x", pt[0], pt[1]);
        printf("\n");
        }
    */
    io_bf = NULL;
    //
    if(print_pages) {
        //
        for(i=0; i<256; i++) {
            if(n_page[i] != -1) print_page(ctx->s, n_page[i]);
            }
        }
    else { //if(strlen(filename)) {
        //===========================================================
        // Выделение памяти под буфер данных
        io_bf = (uint8_t*)malloc((size_t)IOBF_LEN);
        if(io_bf == NULL) {
            printf("Error malloc().\n");
            goto end;
            }
        //
        memset(io_bf, 0xff, IOBF_LEN);
        if(strlen(filename)) {
            //===========================================================
            //              READ FILE
            if(f_bin == 0) {
                file=fopen(filename,"r+");
                if(file == NULL) {
                    printf("File %s not found.\n", filename);
                    //
                    goto end;
                    }
                else {
                    //
                    printf("File: %s ",filename);
                    while(1) {
                        if(fgets(str, 256, file) == NULL) {
                            printf("Error: в файле не найдена завершающая запись.\n");
                            break;
                            }
                        ret = HexToBin(io_bf, str, IOBF_LEN);
                        //
                        if(ret < 0) {
                            printf("Error: %d.\n", ret);
                            break;
                            }
                        else if(ret == 0) { // End file
                            printf(" Ok.\n");
                            break;
                            }
                        else if(ret == 4) {
                            part = 0x10000 * get_part();
                            printf(" Part: 0x%08x", part);
                            }
                        else if(ret == 5) {
                            printf(" Start:0x%08x", get_start());
                            }
                        //
                        }
                    //
                    fclose(file);
                    //
                    //hex_dump(io_bf, 0x13340);
                    }
                }
            else { // f_bin==1
                int fd = open(filename, O_RDONLY);
                if(fd < 0) {
                    printf("File %s not found.\n", filename);
                    goto end;
                    }
                else {
                    int res = read(fd, io_bf, IOBF_LEN);
                    printf("Read bin: res=%d\n", res);
                    if(res<=0) goto end;
                    else close(fd);
                    }
                }
            }
        //
        //===========================================================
        //                 ERASE
        if(erase_all) {
            printf("Erase ... ");
            if(erase_ex) ret = cmd_ExErMem(ctx->s, (uint16_t)0xffff, (uint16_t)0x00);
            else         ret = cmd_ErMem(ctx->s, (uint8_t)0xff, (uint8_t)0x00);
            //
            if(ret) {
                printf("failed!\n");
                }
            else {
                printf("Ok.\n");
                }
            }
        else if(erase_ex) {
            //
            printf("Page size: 0x%04x (%d)\n", dev->fl_ps[0], dev->fl_ps[0]);
            printf("Numb pg: %d\n", IOBF_LEN/dev->fl_ps[0]);
            //
            uint8_t n_p[IOBF_LEN/0x80];
            int k=0;
            for(i=0; i<(IOBF_LEN/dev->fl_ps[0]); i++) {
                //
                if(test_ff(dev->fl_ps[0], &io_bf[i*dev->fl_ps[0]])) {
                    n_p[k] = i;
                    k++;
                    //
                    //ret = cmd_erase_pages(1, &n_p[k]);
                    /*
                    ret=0;
                    //
                    if((k%16)==0) printf("\n");
                    //
                    if(ret) {
                        printf("XX ", i);
                        }
                    else {
                        addr = n_p[k] * dev->fl_ps[0];
                        printf("Erase page %02d [0x%08x - 0x%08x].\n", n_p[k], addr, addr+0x3ff);
                        //printf("%02x ", k);
                        }
                    */
                    //
                    }
                }
            printf("Erase ... ");
            ret = cmd_erase_pages(ctx->s, k, n_p);
            printf(" %d pages. res=%d\n", k, ret);
            usleep(100000);
            }
        //===========================================================
        if(cmd_wr) {
            //                 WRITE
            printf("Write\n");
            //
            for(addr=0x0000; addr<IOBF_LEN; addr+=page) {
                //
                if(test_ff(page, &io_bf[addr]) == 0) continue;

                ret = cmd_WriteMem(ctx->s, 0x8000000+addr, page, &io_bf[addr]);

                //if(ret == 0x100) printf("=");
                //if((addr%0x1000) == 0x00) {
                    if(ret == page) printf("=");
                    else printf("x");
                    fflush(stdout);
                //    }
                }
            printf("\n");
            }
        //===========================================================
        if(m_verify) {
            //                 COMPARE
            printf("Verification\n");
            for(addr=0x0000; addr<IOBF_LEN; addr+=page) {
                if(test_ff(page, &io_bf[addr]) == 0) continue;

                cmd_ReadMem(ctx->s, 0x8000000+addr, page, tmp_bf);
                pt = &io_bf[addr];
                c = '=';
                for(i=0; i<page; i++) {
                    if(tmp_bf[i] == pt[i]) continue;
                    //printf("a=%04x d=%02x f=%02x\n", adr+i, tmp_bf[i], pt[i]);
                    c = 'x';
                    break;
                    }
                printf("%c",c);
                fflush(stdout);
                }
            printf("\n");
            }
        //
        //===========================================================
        //cmd_ReadMem(int fd, uint32_t addr, uint16_t len, uint8_t *bf)
        if(m_read) {
            //                 Read
            int fd_r;
            fd_r=open("flash.bin", O_WRONLY | O_CREAT, 0666);
            if(fd_r>=0) {
                for(addr=0; addr<IOBF_LEN; addr += page) {
                    //
                    int res;
                    res = cmd_ReadMem(ctx->s, 0x8000000+addr, page, io_bf);
                    printf("res=%d\n", res);
                    //
                    //hex_dump(io_bf, page);
                    //
                    if(test_ff(page, io_bf) == 0) {
                        c='=';
                        break;
                        }
                    else {
                        c='x';
                        write(fd_r, io_bf, page);
                        }
                    //
                    printf("%c",c);
                    fflush(stdout);
                    }
                close(fd_r);
                }
            else {
                printf("Error create file \"flash.bin\"\n");
                }

            printf("\n");
            }
        //
        //===========================================================
        }
    if(io_bf != NULL) free(io_bf);
//////////////////////////////////////////////////////////////////
    //
    if(vrf==1) {
        p_start(ctx, command, 1);
        printf("Reset.\n");
        }
    else if(run) {
        p_run(ctx);
        //
        printf("Runnung.\n");
        }
    else {
        return(0);
        p_start(ctx, command, 0);
        // ?????????????????????????????????????????
        printf("Disconnect.\n");
        }
    //
    //fflush(stdout);
    //
    //if(ctx->s > 0) p_close(ctx);
    return(0);

end:;
    printf("Error = %d.\n", ret);
    //
    if(vrf==1) {
        p_start(ctx, command,1);
        printf("Reset target.\n");
        }
    else {
        p_start(ctx, command, 0);
        printf("Disconnect target.\n");
        }
    //if(ctx->s > 0) p_close(ctx);
	return(ret);
}


