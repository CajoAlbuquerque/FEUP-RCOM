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

int llwrite(int fd, char *buffer, int length) {
  unsigned char bcc2 = 0;
  int j = 0, result;

  setPhase(data);

  parseMessage(buffer, length);

  do{
  	control = send_message(fd);
  }while(!parseControl(control));

  return 0;
}

int llread(int fd, unsigned char *buffer)
{
  unsigned char current_bcc2 = 0;
  unsigned char byte;
  unsigned int current_index = 0;

  int state = START;

  flags_t flags;
  initFlags(flags);

  while (state != END)
  {
    if (read(fd, byte, 1) < 0)
    {
      perror("llread");
      exit(-1);
    }
    state = readSM(&byte, state);

    if (state == C_RCV)
    { //Checking for repeated data
      if (byte == CONTROL_0 && NR == 1)
      {
        printf("REPEATED DATA 0\n");
        write_SUframe(fd, RR_1);
        flags.repeated_data = TRUE;
      }
      else if (byte == CONTROL_1 && NR == 0)
      {
        printf("REPEATED DATA 1\n");
        write_SUframe(fd, RR_0);
        flags.repeated_data = TRUE;
      }
      else if (byte == C_DISC)
      {
        printf("Send Disc True\n");
        flags.send_disc = TRUE;
      }
      /*
      When repeated data is sent by the transmitter,
      the receiver ignores all data in the frame.
    */
    }
    else if (state == DATA_LOOP && !flags.repeated_data)
    { //Data is being received
      if (current_bcc2 == byte)
      { // If current byte is bcc2 would data be ok?
        flags.data_ok = TRUE;
      }
      else
      {
        flags.data_ok = FALSE;
      }

      if (byte == ESC)
      { //byte used for stuffing
        flags.escape_byte = TRUE;
        continue;
      }
      else if (flags.escape_byte == TRUE)
      { //destuffing the byte
        if (byte == 0x5e)
          buffer[current_index] = 0x7e; //original byte was 0x7e
        //byte = 0x5d
        else
        {
          buffer[current_index] = 0x7d; //original byte was 0x7d
        }
        flags.escape_byte = FALSE;
        current_index++;
      }
      else
      {
        buffer[current_index] = byte;
        current_index++;
      }

      current_bcc2 = current_bcc2 ^ buffer[current_index - 1];
    }
  }

  //When there is repeated data buffer will have no content
  if (flags.repeated_data)
    return 0;

  if (flags.send_disc)
  {
    processDisc(fd);
    return -1;
  }

  if (flags.data_ok)
  {
    if (NR)
    {
      NR = 0;
      write_SUframe(fd, RR_0);
    }
    else
    {
      NR = 1;
      write_SUframe(fd, RR_1);
    }
  }
  else
  {
    NR ? write_SUframe(fd, REJ_1) : write_SUframe(fd, REJ_0);
  }

  return current_index;
}
