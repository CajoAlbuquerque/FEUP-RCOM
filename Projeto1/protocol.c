#include "protocol.h"
#include "alarm/alarm.h"
#include "macros.h"
#include "receiver/receiver.h"
#include "state_machine/statemachine.h"
#include "transmitter/transmitter.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static struct termios oldtio;

/**
 *  Initializes the serial port connection.
 *  Configures the serial port with non-canonical mode.
 *
 *  @param port Serial port number
 *  @return Serial port file descriptor
 */
int initSerialPort(int port) {
  struct termios newtio;
  char serialPort[11];
  int fd;

  switch (port) {
  case 0:
    strcpy(serialPort, "/dev/ttyS0");
    break;
  case 1:
    strcpy(serialPort, "/dev/ttyS1");
    break;
  case 2:
    strcpy(serialPort, "/dev/ttyS2");
    break;
  case 3:
    strcpy(serialPort, "/dev/ttyS3");
    break;

  default:
    fprintf(stderr, "Invalid serial port number\n");
    return -1;
  }

  fd = open(serialPort, O_RDWR | O_NOCTTY);
  if (fd < 0) {
    perror(serialPort);
    return -1;
  }

  if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    return -1;
  }

  bzero(&newtio, sizeof(newtio)); /*memory allocation with zeros*/
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;
  newtio.c_lflag = 0; /* set input mode (non-canonical, no echo,...) */

  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;  /* blocking read until 1 char received */

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
    perror("tcsetattr");
    return -1;
  }

  printf("New termios structure set\n");

  return fd;
}

/**
 *  Safely closes the serial port connection.
 *  Configures the serial port back to its initial state.
 *
 *  @param port Serial port number
 *  @return Serial port file descriptor
 */
int closeSerialPort(int fd) {
  if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
    perror("tcsetattr");
    return -1;
  }

  close(fd);
}

int llopen(int port, int mode) {
  int fd = initSerialPort(port);

  if (initializeHandler(fd) == -1)
    return -1;

  switch (mode) {
  case TRANSMITTER:
    if (write_suFrame(fd, C_SET) == -1) {
      return -1;
    }
    printf("Sent SET\n");

    if (read_suFrame(fd, C_UA) == -1) {
      return -1;
    }
    printf("Received UA\n");
    break;

  case RECEIVER:
    if (read_suFrame(fd, C_SET) == -1) {
      return -1;
    }
    printf("Received SET\n");

    if (write_suFrame(fd, C_UA) == -1) {
      return -1;
    }
    printf("Sent UA\n");
    break;

  default:
    return -1;
  }

  return fd;
}