#include "io.h"
#include "cmd.h"

// Ожидание и чтение подттверждения команды
//
extern port_t s_ctx;
extern int m_info;

#define SZ_128	0x00000080
#define SZ_256	0x00000100
#define SZ_1K	0x00000400
#define SZ_2K	0x00000800
#define SZ_16K	0x00004000
#define SZ_32K	0x00008000
#define SZ_64K	0x00010000
#define SZ_128K	0x00020000
#define SZ_256K	0x00040000

//typedef struct stm32_dev stm32_dev_t;

/* fixed size pages */
static uint32_t p_128[] = { SZ_128, 0 };
static uint32_t p_256[] = { SZ_256, 0 };
static uint32_t p_1k[]  = { SZ_1K,  0 };
static uint32_t p_2k[]  = { SZ_2K,  0 };
/* F2 and F4 page size */
static uint32_t f2f4[]  = { SZ_16K, SZ_16K, SZ_16K, SZ_16K, SZ_64K, SZ_128K, 0 };
/* F4 dual bank page size */
static uint32_t f4db[]  = {
	SZ_16K, SZ_16K, SZ_16K, SZ_16K, SZ_64K, SZ_128K, SZ_128K, SZ_128K,
	SZ_16K, SZ_16K, SZ_16K, SZ_16K, SZ_64K, SZ_128K, 0
};
/* F7 page size */
static uint32_t f7[]    = { SZ_32K, SZ_32K, SZ_32K, SZ_32K, SZ_128K, SZ_256K, 0 };

typedef enum {
	F_NO_ME = 1 << 0,	/* Mass-Erase not supported */
	F_OBLL  = 1 << 1,	/* OBL_LAUNCH required */
} flags_t;

/*
 * Device table, corresponds to the "Bootloader device-dependant parameters"
 * table in ST document AN2606.
 * Note that the option bytes upper range is inclusive!
 */
const stm32_dev_t t_dev[] = {
	/* ID   "name"                              SRAM-address-range      FLASH-address-range    PPS  PSize   Option-byte-addr-range  System-mem-addr-range   Flags */
	/* F0 */
	{0x440, "STM32F030x8/F05xxx"              , 0x20000800, 0x20002000, 0x08000000, 0x08010000,  4, p_1k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFEC00, 0x1FFFF800, 0},
	{0x442, "STM32F030xC/F09xxx"              , 0x20001800, 0x20008000, 0x08000000, 0x08040000,  2, p_2k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFC800, 0x1FFFF800, F_OBLL},
	{0x444, "STM32F03xx4/6"                   , 0x20000800, 0x20001000, 0x08000000, 0x08008000,  4, p_1k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFEC00, 0x1FFFF800, 0},
	{0x445, "STM32F04xxx/F070x6"              , 0x20001800, 0x20001800, 0x08000000, 0x08008000,  4, p_1k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFC400, 0x1FFFF800, 0},
	{0x448, "STM32F070xB/F071xx/F72xx"        , 0x20001800, 0x20004000, 0x08000000, 0x08020000,  2, p_2k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFC800, 0x1FFFF800, 0},
	/* F1 */
	{0x412, "STM32F10xxx Low-density"         , 0x20000200, 0x20002800, 0x08000000, 0x08008000,  4, p_1k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFF000, 0x1FFFF800, 0},
	{0x410, "STM32F10xxx Medium-density"      , 0x20000200, 0x20005000, 0x08000000, 0x08020000,  4, p_1k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFF000, 0x1FFFF800, 0},
	{0x414, "STM32F10xxx High-density"        , 0x20000200, 0x20010000, 0x08000000, 0x08080000,  2, p_2k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFF000, 0x1FFFF800, 0},
	{0x420, "STM32F10xxx Medium-density VL"   , 0x20000200, 0x20002000, 0x08000000, 0x08020000,  4, p_1k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFF000, 0x1FFFF800, 0},
	{0x428, "STM32F10xxx High-density VL"     , 0x20000200, 0x20008000, 0x08000000, 0x08080000,  2, p_2k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFF000, 0x1FFFF800, 0},
	{0x418, "STM32F105xx/F107xx"              , 0x20001000, 0x20010000, 0x08000000, 0x08040000,  2, p_2k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFB000, 0x1FFFF800, 0},
	{0x430, "STM32F10xxx XL-density"          , 0x20000800, 0x20018000, 0x08000000, 0x08100000,  2, p_2k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFE000, 0x1FFFF800, 0},
	/* F2 */
	{0x411, "STM32F2xxxx"                     , 0x20002000, 0x20020000, 0x08000000, 0x08100000,  1, f2f4  , 0x1FFFC000, 0x1FFFC00F, 0x1FFF0000, 0x1FFF7800, 0},
	/* F3 */
	{0x432, "STM32F373xx/F378xx"              , 0x20001400, 0x20008000, 0x08000000, 0x08040000,  2, p_2k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFD800, 0x1FFFF800, 0},
	{0x422, "STM32F302xB(C)/F303xB(C)/F358xx" , 0x20001400, 0x2000A000, 0x08000000, 0x08040000,  2, p_2k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFD800, 0x1FFFF800, 0},
	{0x439, "STM32F301xx/F302x4(6/8)/F318xx"  , 0x20001800, 0x20004000, 0x08000000, 0x08010000,  2, p_2k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFD800, 0x1FFFF800, 0},
	{0x438, "STM32F303x4(6/8)/F334xx/F328xx"  , 0x20001800, 0x20003000, 0x08000000, 0x08010000,  2, p_2k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFD800, 0x1FFFF800, 0},
	{0x446, "STM32F302xD(E)/F303xD(E)/F398xx" , 0x20001800, 0x20010000, 0x08000000, 0x08080000,  2, p_2k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFD800, 0x1FFFF800, 0},
	/* F4 */
	{0x413, "STM32F40xxx/41xxx"               , 0x20003000, 0x20020000, 0x08000000, 0x08100000,  1, f2f4  , 0x1FFFC000, 0x1FFFC00F, 0x1FFF0000, 0x1FFF7800, 0},
	{0x419, "STM32F42xxx/43xxx"               , 0x20003000, 0x20030000, 0x08000000, 0x08200000,  1, f4db  , 0x1FFEC000, 0x1FFFC00F, 0x1FFF0000, 0x1FFF7800, 0},
	{0x423, "STM32F401xB(C)"                  , 0x20003000, 0x20010000, 0x08000000, 0x08040000,  1, f2f4  , 0x1FFFC000, 0x1FFFC00F, 0x1FFF0000, 0x1FFF7800, 0},
	{0x433, "STM32F401xD(E)"                  , 0x20003000, 0x20018000, 0x08000000, 0x08080000,  1, f2f4  , 0x1FFFC000, 0x1FFFC00F, 0x1FFF0000, 0x1FFF7800, 0},
	{0x458, "STM32F410xx"                     , 0x20003000, 0x20008000, 0x08000000, 0x08020000,  1, f2f4  , 0x1FFFC000, 0x1FFFC00F, 0x1FFF0000, 0x1FFF7800, 0},
	{0x431, "STM32F411xx"                     , 0x20003000, 0x20020000, 0x08000000, 0x08080000,  1, f2f4  , 0x1FFFC000, 0x1FFFC00F, 0x1FFF0000, 0x1FFF7800, 0},
	{0x421, "STM32F446xx"                     , 0x20003000, 0x20020000, 0x08000000, 0x08080000,  1, f2f4  , 0x1FFFC000, 0x1FFFC00F, 0x1FFF0000, 0x1FFF7800, 0},
	{0x434, "STM32F469xx"                     , 0x20003000, 0x20060000, 0x08000000, 0x08200000,  1, f4db  , 0x1FFEC000, 0x1FFFC00F, 0x1FFF0000, 0x1FFF7800, 0},
	/* F7 */
	{0x449, "STM32F74xxx/75xxx"               , 0x20004000, 0x20050000, 0x08000000, 0x08100000,  1, f7    , 0x1FFF0000, 0x1FFF001F, 0x1FF00000, 0x1FF0EDC0, 0},
	/* L0 */
	{0x425, "STM32L031xx/041xx"               , 0x20001000, 0x20002000, 0x08000000, 0x08008000, 32, p_128 , 0x1FF80000, 0x1FF8001F, 0x1FF00000, 0x1FF01000, 0},
	{0x417, "STM32L05xxx/06xxx"               , 0x20001000, 0x20002000, 0x08000000, 0x08010000, 32, p_128 , 0x1FF80000, 0x1FF8001F, 0x1FF00000, 0x1FF01000, 0},
	{0x447, "STM32L07xxx/08xxx"               , 0x20002000, 0x20005000, 0x08000000, 0x08030000, 32, p_128 , 0x1FF80000, 0x1FF8001F, 0x1FF00000, 0x1FF02000, 0},
	/* L1 */
	{0x416, "STM32L1xxx6(8/B)"                , 0x20000800, 0x20004000, 0x08000000, 0x08020000, 16, p_256 , 0x1FF80000, 0x1FF8001F, 0x1FF00000, 0x1FF01000, F_NO_ME},
	{0x429, "STM32L1xxx6(8/B)A"               , 0x20001000, 0x20008000, 0x08000000, 0x08020000, 16, p_256 , 0x1FF80000, 0x1FF8001F, 0x1FF00000, 0x1FF01000, 0},
	{0x427, "STM32L1xxxC"                     , 0x20001000, 0x20008000, 0x08000000, 0x08040000, 16, p_256 , 0x1FF80000, 0x1FF8001F, 0x1FF00000, 0x1FF02000, 0},
	{0x436, "STM32L1xxxD"                     , 0x20001000, 0x2000C000, 0x08000000, 0x08060000, 16, p_256 , 0x1FF80000, 0x1FF8009F, 0x1FF00000, 0x1FF02000, 0},
	{0x437, "STM32L1xxxE"                     , 0x20001000, 0x20014000, 0x08000000, 0x08080000, 16, p_256 , 0x1FF80000, 0x1FF8009F, 0x1FF00000, 0x1FF02000, F_NO_ME},
	/* L4 */
	{0x415, "STM32L476xx/486xx"               , 0x20003100, 0x20018000, 0x08000000, 0x08100000,  1, p_2k  , 0x1FFF7800, 0x1FFFF80F, 0x1FFF0000, 0x1FFF7000, 0},
	/* These are not (yet) in AN2606: */
	{0x641, "Medium_Density PL"               , 0x20000200, 0x20005000, 0x08000000, 0x08020000,  4, p_1k  , 0x1FFFF800, 0x1FFFF80F, 0x1FFFF000, 0x1FFFF800, 0},
	{0x9a8, "STM32W-128K"                     , 0x20000200, 0x20002000, 0x08000000, 0x08020000,  4, p_1k  , 0x08040800, 0x0804080F, 0x08040000, 0x08040800, 0},
	{0x9b0, "STM32W-256K"                     , 0x20000200, 0x20004000, 0x08000000, 0x08040000,  4, p_2k  , 0x08040800, 0x0804080F, 0x08040000, 0x08040800, 0},
	{0x0}
};


typedef struct __name_of_ids__ {
    uint32_t id;
    char* name;
} name_of_ids;

name_of_ids n_cmd[]={
{ 0x0,  "Get" },
{ 0x1,  "Get Version & Read Protection Status"},
{ 0x2,  "Get ID"},
{ 0x11, "Read Memory"},
{ 0x21, "Go"},
{ 0x31, "Write Memory"},
{ 0x43, "Erase"},
{ 0x44, "Extended Erase"},
{ 0x63, "Write Protect"},
{ 0x73, "Write Unprotect"},
{ 0x82, "Readout Protect"},
{ 0x92, "Readout Unprotect"},
{ 0, NULL}
};

stm32_dev_t *dev=NULL;

int wait_ack(int fd, char *str)
{
int w;
int ret = 0;
uint8_t c = 0;
    //
    w = 3000;
    while(w != 0) {
        usleep(1000);
        ret = read(fd, &c, 1);
        if(ret == 1) {
            //
            //if(c == ACK) printf("ACK\n");
            //else if(c == NOACK) printf("NOACK\n");
            //else printf("ACK=??? c=%02x\n", c);
            //
            ///printf("ACK: [%s] [%x]\n", str, c);
            return(c);
            }
        w--;
        }
    //
    //if(m_info)
    printf("no ACK: [%s]\n", str);
    return(-1);
}


int cmd_Get(int fd)
{
//int i;
uint8_t cmd[8];
uint8_t bf[4];
int ret = 0;
uint8_t len;
uint8_t vers;
//char *s=NULL;
//int res=0;
    // Clear receiver
    printf("cmd_GET()\n");
    do {
        ret = read(fd, cmd, 8);
    } while(ret>0);
    //
    cmd[0] = 0x00;
    cmd[1] = 0xff;
    write(fd, cmd, 2);
    fsync(fd);
    //printf("write: fd=%d res=%d\n", fd, res);
    usleep(100000);
    //
    if(wait_ack(fd, "cmd_Get") != ACK) return(-1);
    //
    //usleep(1000000);
    //
    ret = read(fd, &len, 1);

    if(ret < 0) {printf("Error [cmd_Get]\n"); return(-2);}
    printf("cmd_Get [len=%d]\n", len);
    /*
    // Версия Bootloader
    ret = read(s_ctx.s, &vers, 1);
    if(ret < 0) {printf("Error [cmd_Get]\n"); return(-3);}
    printf("Bootloader Vers=%d.%d\n", vers/10, vers%10);
    */
    //
    ret = read(fd, bf, len+1);

    //int  k;
    if(ret == len+1) {
        //
        if(m_info) {
            vers = bf[0];
            printf("Bootloader Vers=%d.%d\n", vers/10, vers%10);
            //
            int i;
            printf("Supported cmd`s:\n");
            for(i=1; i<ret; i++) {
                //
                int k;
                char *s = NULL;
                for(k=0; k<12; k++) {
                    if(n_cmd[k].id == (uint32_t)bf[i]) {
                        s=n_cmd[k].name;
                        break;
                        }
                    }
                //
                if(s != NULL) printf("0x%02x [%s]\n", bf[i], s);
                else printf("0x%02x [UNKNOWN]\n", bf[i]);
                }
            }
        //
        printf("\n");
        }
    else {
        printf("Error [cmd_Get]\n");
        return(-4);
        }
    //
    if(wait_ack(fd, "cmd_Get") != ACK) return(-5);
    //
    return(0);
}

int cmd_GetVers(int fd)
{
uint8_t cmd[8];
uint8_t bf[4];
int ret = 0;
    // Clear receiver
    do {
        ret = read(fd, cmd, 8);
    } while(ret>0);
    //
    cmd[0] = 0x01;
    cmd[1] = 0xfe;
    write(fd, cmd, 2);
    usleep(100000);
    //
    if(wait_ack(fd, "cmd_GetVers") != ACK) return(-1);
    //
    ret = read(fd, &bf[0], 3);
    //
    if(wait_ack(fd, "cmd_GetVers") != ACK) return(-2);
    //
    if(ret>=3) {
        printf("[1] Bootloader Vers=%d.%d [%02x %02x]\n",
                        (uint32_t)bf[0]/10, (uint32_t)bf[0]%10,
                        (uint32_t)bf[1], (uint32_t)bf[2]);
        return(3);
        }
    //
    return(-4);
}

int cmd_GetID(int fd, uint32_t* sz_pg, uint32_t *len_pg)
{
uint8_t cmd[8];
uint8_t bf[4];
int ret = 0;
    printf("cmd_GetID()\n");
    // Clear receiver
    do {
        ret = read(fd, cmd, 8);
    } while(ret>0);
    //
    cmd[0] = 0x02;
    cmd[1] = 0xfd;
    write(fd, cmd, 2);
    fsync(fd);
    usleep(100000);
    //
    if(wait_ack(fd, "cmd_GetID") != ACK) return(-1);
    //
    ret = read(fd, &bf[0], 3);
    //
    if(wait_ack(fd, "cmd_GetID") != ACK) return(-2);
    //
    uint32_t pid = (((uint32_t)bf[1])<<8) | (uint32_t)bf[2];
    //char *s=NULL;
    int i;
    for(i=0;;i++) {
        if(t_dev[i].id == 0) break;
        if(t_dev[i].id == pid) {
            dev = (stm32_dev_t *)&t_dev[i];
            break;
            }
        }
    if(ret>=3) {
        //if(m_info) {
            printf("\n");
            //if(s != NULL) printf("PID: 0x%04x [%s]\n", pid, s);
            //else printf("PID: 0x%04x [UNKNOWN!]\n", pid);
            //printf("\n");
            //
            printf("Device ID=0x%04x (%s):\n", pid, dev->name);
            printf("  - RAM        : %dKiB  (%db reserved by bootloader)\n", (dev->ram_end - 0x20000000) / 1024, dev->ram_start - 0x20000000);
            printf("  - Flash      : %dKiB (size first sector: %dx%d)\n", (dev->fl_end - dev->fl_start ) / 1024, dev->fl_pps, dev->fl_ps[0]);
            printf("  - Page size  : %db\n", dev->fl_ps[0]); *sz_pg = dev->fl_ps[0];
            printf("  - N pages    : %d\n", (dev->fl_end - dev->fl_start ) / dev->fl_ps[0]);
            *len_pg = (dev->fl_end - dev->fl_start ) / dev->fl_ps[0];
            printf("  - Option RAM : %db\n", dev->opt_end - dev->opt_start + 1);
            printf("  - System RAM : %dKiB\n", (dev->mem_end - dev->mem_start) / 1024);
            //
            //}
        return(0);
        }
    //
    return(-4);
}

int cmd_ReadMem(int fd, uint32_t addr, uint16_t len, uint8_t *bf)
{
uint8_t cmd[8];
int ret = 0;
int n;
int w;
uint8_t ks;
    // Clear receiver
    do {
        ret = read(fd, cmd, 8);
    } while(ret>0);
    //
    cmd[0] = 0x11;
    cmd[1] = 0xee;
    write(fd, cmd, 2);
    //n = wait_ack();
    //printf("I=%02x  ", n);
    //if(n != ACK) return(-1);
    if(wait_ack(fd, "cmd_RM") != ACK) return(-1);
    //
    cmd[0] = addr >> 24; //MSB - старший байт
    cmd[1] = addr >> 16;
    cmd[2] = addr >> 8;
    cmd[3] = addr;       //LSB - младший байт
    cmd[4] = cmd[0] ^ cmd[1] ^ cmd[2] ^ cmd[3];

    write(fd, cmd, 5);
    if(wait_ack(fd, "Adr") != ACK) return(-2);
    /*
    cmd[0] = len-1;
    cmd[1] = cmd[0];
    printf("%08x %02x %02x =%02x\n", addr, cmd[0], cmd[1], cmd[0] ^ cmd[1]);
    write(fd, cmd, 2);
    if(wait_ack(fd, "Len") != ACK) return(-3);
    */
    //
    cmd[0] = len-1;
    ks = cmd[0];
    ks ^= cmd[0];
    //
    write(fd, cmd, 1);
    ///write(s_ctx.s, bf, len);
    write(fd, &ks, 1);
    //
    if(wait_ack(fd, "n_data_ks") != ACK) return(-3);

    w = 1000;
    n = 0;
    while(w--) {
        usleep(1000);
        ret = read(fd, &bf[n], len-n);
        if(ret>0) {n += ret;}
        if(n>=len) return(n);
        }
    //
    return(-4);
}

int cmd_GO(int fd, uint32_t addr)
{

    return(0);
}

int cmd_ErMem(int fd, uint8_t numbers_of_pages, uint8_t pages)
{
uint8_t cmd[8];
int ret = 0;
    printf("cmd_ErMem()\n");
    // Clear receiver
    do {
        ret = read(fd, cmd, 8);
    } while(ret>0);
    //
    cmd[0] = 0x43;
    cmd[1] = 0xbc;
    write(s_ctx.s, cmd, 2);
    if(wait_ack(fd, "cmd_ErMem") != ACK) return(-1);
    //
    if(numbers_of_pages == 0xff) {
        cmd[0] = 0xff; // Количество страниц для стирания (FF - все)
        cmd[1] = 0x00;
        write(s_ctx.s, cmd, 2);
        }
    else {
        cmd[0] = numbers_of_pages; // Количество страниц для стирания (FF - все)
        cmd[1] = pages;            // Номер страницы для стирания
        cmd[2] = cmd[0] ^ cmd[1];

        write(s_ctx.s, cmd, 3);
        }
    //
    if(wait_ack(fd, "cmd_ErMem") != ACK) return(-2);
    //
    return(0);
}

int cmd_erase_pages(int fd, uint8_t numbers_of_pages, uint8_t *pages)
{
uint8_t cmd[256];
int ret = 0;
    // Clear receiver
    do {
        ret = read(s_ctx.s, cmd, 8);
    } while(ret>0);
    //
    usleep(250000);
    //
    cmd[0] = 0x43;
    cmd[1] = 0xbc;
    write(s_ctx.s, cmd, 2);
    if(wait_ack(fd, "cmd_ErMem") != ACK) return(-1);
    //
    if(numbers_of_pages == 0xff) {
        cmd[0] = 0xff; // Количество страниц для стирания (FF - все)
        cmd[1] = 0x00;
        write(s_ctx.s, cmd, 2);
        }
    else {
        cmd[0] = numbers_of_pages-1; // Количество страниц для стирания (FF - все)
        printf("n=%02d ", cmd[0]);
        int i;
        uint8_t ks=cmd[0];
        for(i=0; i<numbers_of_pages; i++) {
            cmd[i+1] = pages[i];
            ks ^= cmd[i+1];
            printf("%02d ", cmd[i+1]);
            }
        cmd[i+1] = ks;
        printf("ks=%02d ", cmd[i+1]);
        write(s_ctx.s, cmd, i+2);
        }
    //
    if(wait_ack(fd, "cmd_ErMem") != ACK) return(-2);
    //
    return(0);
}


int cmd_ExErMem(int fd, uint16_t numb, uint16_t page)
{
uint8_t cmd[8];
int ret = 0;
    // Clear receiver
    do {
        ret = read(s_ctx.s, cmd, 8);
    } while(ret>0);
    //
    cmd[0] = 0x44;
    cmd[1] = 0xbb;
    write(s_ctx.s, cmd, 2);
    if(wait_ack(fd, "cmd_EER") != ACK) return(-1);
    //
    cmd[0] = numb>>8; // Количество страниц для стирания (FF - все)
    cmd[1] = numb;    //
    cmd[2] = cmd[0] ^ cmd[1];
    //
    if(numb == 0xffff) {
        write(s_ctx.s, cmd, 3);
        }
    else {
        cmd[1] = page; // Номера страниц для стирания
        cmd[2] = cmd[0] ^ cmd[1];
        write(s_ctx.s, cmd, 3);
        }
    if(wait_ack(fd, "EER_peges") != ACK) return(-2);
    return(0);
}

int cmd_WriteMem(int fd, uint32_t addr, uint16_t len, const uint8_t *bf)
{
uint8_t cmd[8];
int ret = 0;
int n;
uint8_t ks = 0;
    // Clear receiver
    do {
        ret = read(s_ctx.s, cmd, 8);
    } while(ret>0);
    //
    cmd[0] = 0x31;
    cmd[1] = 0xce;
    write(s_ctx.s, cmd, 2);
    //n = wait_ack();
    //printf("I=%02x  ", n);
    //if(n != ACK) return(-1);
    if(wait_ack(fd, "cmd_WR") != ACK) return(-1);
    //
    cmd[0] = addr >> 24; //MSB - старший байт
    cmd[1] = addr >> 16;
    cmd[2] = addr >> 8;
    cmd[3] = addr;       //LSB - младший байт
    cmd[4] = cmd[0] ^ cmd[1] ^ cmd[2] ^ cmd[3];
    write(s_ctx.s, cmd, 5);
    if(wait_ack(fd, "adr") != ACK) return(-2);
    //
    cmd[0] = len-1;
    ks = cmd[0];
    for(n=0;n<len;n++) ks ^= bf[n];
    //
    write(s_ctx.s, cmd, 1);
    write(s_ctx.s, bf, len);
    write(s_ctx.s, &ks, 1);
    //
    if(wait_ack(fd, "n_data_ks") != ACK) return(-3);
    /*
    w = 1000;
    n = 0;
    while(w--) {
        usleep(5000);
        ret = read(s_ctx.s, &bf[n], len-n);
        if(ret>0) {n += ret;}
        if(n>=len) return(n);
        }
    */
    return(len);
}


