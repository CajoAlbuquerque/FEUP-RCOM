#include "protocol.h"
#include "../alarm/alarm.h"
#include "../macros.h"
#include "../receiver/receiver.h"
#include "../state_machine/statemachine.h"
#include "../transmitter/transmitter.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#define DEBUG 1

static struct termios oldtio;

/**
 *  Initializes the serial port connection.
 *  Configures the serial port with non-canonical mode.
 *
 *  @param port Serial port number
 *  @return Serial port file descriptor
 */
int initSerialPort(int port)
{
  struct termios newtio;
  char serialPort[11];
  int fd;

  switch (port)
  {
  case 0:
    if(DEBUG)
      strcpy(serialPort, "/dev/pts/3");
    else
      strcpy(serialPort, "/dev/ttyS0");
    break;
  case 1:
    if(DEBUG)
      strcpy(serialPort, "/dev/pts/4");
    else
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
  if (fd < 0)
  {
    perror(serialPort);
    return -1;
  }

  if (tcgetattr(fd, &oldtio) == -1)
  { /* save current port settings */
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

  if (tcsetattr(fd, TCSANOW, &newtio) == -1)
  {
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
int closeSerialPort(int fd)
{
  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  {
    perror("tcsetattr");
    return -1;
  }

  close(fd);
  return 0;
}

int llopen(int port, int mode)
{
  int fd = initSerialPort(port);

  if (initializeHandler(fd) == -1)
    return -1;

  setPhase(open_phase);

  printf("Enter switch\n");
  switch (mode)
  {
  case TRANSMITTER:
    printf("TRANSMITTER\n");
    if (write_suFrame(fd, C_SET) == -1)
    {
      return -1;
    }
    printf("Sent SET\n");

    if (read_suFrame(fd, C_UA) == -1)
    {
      return -1;
    }
    printf("Received UA\n");
    break;

  case RECEIVER:
        printf("RECEIVER\n");

    if (read_suFrame(fd, C_SET) == -1)
    {
      return -1;
    }
    printf("Received SET\n");

    if (write_suFrame(fd, C_UA) == -1)
    {
      return -1;
    }
    printf("Sent UA\n");
    break;

  default:
    return -1;
  }

  return fd;
}

int llread(int fd, unsigned char *buffer)
{
  unsigned int result;
  unsigned int NR = getNR();
  flags_t flags;
  initFlags(&flags);

  result = read_dataFrame(fd, buffer, &flags);
  printf("Received DATA\n");

  //When there is repeated data, buffer will have no content
  if (flags.repeated_data){
    if(NR == 1)
      write_suFrame(fd, RR_1);
    else
      write_suFrame(fd, RR_0);

    return 0;
  }

  //When DISC is received, buffer will have no content
  if (flags.send_disc)
  {
    if(write_suFrame(fd, C_DISC) < 0)
      return -1;
    printf("Sent DISC\n");

    setPhase(close_phase);
    if(read_suFrame(fd, C_UA) < 0)
      return -1;
    printf("Received UA\n");

    closeSerialPort(fd);
    return 0;
  }

  if (flags.data_ok)
  {
    if (NR == 1)
    {
      setNR(0);
      write_suFrame(fd, RR_0);
      printf("Sent RR_0\n");
    }
    else
    {
      setNR(0);
      write_suFrame(fd, RR_1);
      printf("Sent RR_1\n");
    }
  }
  else
  {
    if (NR == 1){
      write_suFrame(fd, REJ_1);
      printf("Sent REJ_1\n");
    }
    else{
      write_suFrame(fd, REJ_0);
      printf("Sent REJ_0\n");
    }
  }

  return result;
}

int llwrite(int fd, unsigned char *buffer, int length)
{
  unsigned char control;
  int result;

  setPhase(data_phase);

  parseMessage(buffer, length);

  do
  {
    result = sendMessage(fd);
    printf("Sent Data\n");

    if (result < 0)
      return -1;
    
    control = read_responseFrame(fd);
    printf("Received %x\n", control);

  } while (!parseControl(control));

  return result;
}

int llclose(int fd) {
  setPhase(close_phase);
  if(write_suFrame(fd, C_DISC) < 0)
    return -1;
  printf("Sent DISC\n");

  if(read_suFrame(fd, C_DISC) < 0)
    return -1;
  printf("Received DISC\n");

  if(write_suFrame(fd, C_UA) < 0)
    return -1;
  printf("Sent C_UA\n");

  if(closeSerialPort(fd) < 0)
    return -1;

  return 0;
}