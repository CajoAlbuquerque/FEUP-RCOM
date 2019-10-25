#include "receiver.h"
#include "../macros.h"
#include "../state_machine/statemachine.h"
#include "../alarm/alarm.h"

#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

static unsigned int NR = 1;

void initFlags(flags_t *flags)
{
  flags->data_ok = FALSE;
  flags->repeated_data = FALSE;
  flags->escape_byte = FALSE;
  flags->send_disc = FALSE;
}

void setNR(unsigned int new_nr)
{
  NR = new_nr;
}

unsigned int getNR()
{
  return NR;
}

int read_suFrame(int fd, unsigned char control)
{
  int read_res, state = START;
  unsigned char byte;

  alarm(TIMEOUT_INTERVAL);
  while (state != END)
  {
    read_res = read(fd, &byte, 1);

    if (read_res < 0 && errno == EINTR)
    {
      errno = 0;
      continue;
    }
    else if (read_res < 0)
      return -1;

    state = suFrameSM(byte, control, state);
  }

  alarm(0);

  return 0;
}

int read_dataFrame(int fd, unsigned char *buffer, flags_t *flags)
{
  unsigned char current_bcc2 = 0;
  unsigned char byte;
  unsigned int current_index = 0, read_res;
  int state = START;

  alarm(TIMEOUT_INTERVAL);
  while (state != END)
  {
    read_res = read(fd, &byte, 1);
    if (read_res < 0 && errno == EINTR)
    {
      errno = 0;
      continue;
    }
    else if (read_res < 0)
    {
      return -1;
    }

    state = readSM(byte, state);

    if (state == C_RCV)
    { //Checking for repeated data
      if (byte == CONTROL_0 && NR == 1 ||
          byte == CONTROL_1 && NR == 0)
        flags->repeated_data = TRUE;
      else if (byte == C_DISC)
        flags->send_disc = TRUE;
    }
    /*
      When repeated data is sent by the transmitter,
      the receiver ignores all data in the frame.
    */
    else if (state == DATA_LOOP && !flags->repeated_data)
    { //Data is being received
      // If current byte is bcc2 would data be ok?
      if (current_bcc2 == byte)
        flags->data_ok = TRUE;
      else
        flags->data_ok = FALSE;

      if (byte == ESC)
      { //byte used for stuffing
        flags->escape_byte = TRUE;
        continue;
      }

      if (flags->escape_byte == TRUE)
      { //destuffing the byte
        if (byte == FLAG_SOL)
          byte = FLAG; //original byte was 0x7e
        else if (byte == ESC_SOL)
          byte = ESC; //original byte was 0x7d

        flags->escape_byte = FALSE;
      }

      current_bcc2 = current_bcc2 ^ byte;
      buffer[current_index] = byte;
      current_index++;
    }
  }
  alarm(0);

  return current_index - 1;
}
