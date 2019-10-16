#include "statemachine.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define F1_INDEX 0
#define A_INDEX 1
#define C_INDEX 2
#define BCC_INDEX 3
#define F2_INDEX 4
#define SET_SIZE 5

#define CONTROL_0 0x00
#define CONTROL_1 0x40
#define RR_0 0x05
#define RR_1 0x85
#define REJ_0 0x01
#define REJ_1 0x81

#define ESC 0x7d

unsigned int NR = 0;

/**
 * 	Writes a supervision or unumbered frame
 *
 * 	@param fd File descriptor of the serial port
 * 	@param control Control character value to be written
 *
 * 	@return Number of characters written
 */
int write_SUframe(int fd, unsigned char control);

/**
 *  Reads a data frame from the serial port
 *
 *  @param fd File descriptor of the serial port
 *  @param buffer Array of characters on which the data will be saved
 *
 *  @return Number of characters read (buffer length) in case of success;
 *  otherwise returns a negative value
 */
int llread(int fd, unsigned char *buffer);

int main(int argc, char **argv);