#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "statemachine.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07
#define F1_INDEX 0
#define A_INDEX 1
#define C_INDEX 2
#define BCC_INDEX 3
#define F2_INDEX 4
#define SET_SIZE 5

#define ESC 0x7d
#define ESC_SOL 0x5d
#define FLAG_SOL 0x5e

#define CONTROL_0 0x00
#define CONTROL_1 0x40
#define RR_0 0x05
#define RR_1 0x85
#define REJ_0 0x01
#define REJ_1 0x81

unsigned int NS = 0;
enum state{ Set, Transmit, End}state;

void timeout();
void set_transmission();
int llwrite(int fd, char *buffer, int length);
int main(int argc, char **argv);