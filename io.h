#ifndef	_P_IO_H
#define	_P_IO_H	1

#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>

#define L475 0

typedef struct _port {
    /* Socket or file descriptor */
    int s;
    int debug;
    //int error
    //struct timeval response_timeout;
    //struct timeval byte_timeout;
    // Device: "/dev/ttyS0", "/dev/ttyUSB0" or "/dev/tty.USA19*"
    char device[16];
    /* Bauds: 9600, 19200, 57600, 115200, etc */
    int baud;
    /* Data bit */
    uint8_t data_bit;
    /* Stop bit */
    uint8_t stop_bit;
    /* Parity: 'N', 'O', 'E' */
    char parity;
    /* Save old termios settings */
    struct termios old_tios;
    int serial_mode;
} port_t;

int p_connect(port_t *ctx);
//int p_set_serial_mode(port_t *ctx, int mode);
//int p_get_serial_mode(port_t *ctx);
void p_close(port_t *ctx);
//int p_flush(port_t *ctx);
//ssize_t p_send(port_t *ctx, const uint8_t *buff, int length);
//ssize_t p_recv(port_t *ctx, uint8_t *buff, int length);
int p_set_parity(port_t *ctx, char par);

int p_reset(port_t *ctx);
int p_start(port_t *ctx, char *command, int en);
int p_run(port_t *ctx);

#endif

