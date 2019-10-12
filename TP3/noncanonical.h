#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "statemachine.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define F1_INDEX 0
#define A_INDEX 1
#define C_INDEX 2
#define BCC_INDEX 3
#define F2_INDEX 4
#define SET_SIZE 5

#define RR_0 0x05
#define RR_1 0x85
#define REJ_0 0x01
#define REJ_1 0x81
#define C_0 0x00
#define C_1 0x40

unsigned int NS = 0;

/**
 * 	Writes a supervision frame or an unumbered frame
 * 
 * 	@param fd File descriptor of the open serial port
 * 	@param control Control character value to be written
 * 	
 * 	@return Number of characters written
 */
int write_SUframe(int fd, unsigned char control);
int llread(int fd, unsigned char * buffer);
int main(int argc, char** argv);

