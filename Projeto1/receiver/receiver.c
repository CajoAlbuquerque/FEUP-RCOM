#include "receiver.h"
#include "../macros.h"
#include "../state_machine/statemachine.h"

static unsigned int NR = 0;

void initFlags(flags_t flags)
{
  flags.data_ok = FALSE;
  flags.repeated_data = FALSE;
  flags.escape_byte = FALSE;
  flags.send_disc = FALSE;
}

int read_suFrame(int fd, unsigned char control)
{
  int read_res, state = START;
  unsigned char byte;

  alarm(3);
  while (state != END)
  {
    read_res = read(fd, &byte, 1);

    if (read_res > 0)
    {
      alarm(0);
    }
    else if (read_res < 0)
    {
      return -1;
    }

    state = suFrameSM(byte, control, state);
  }

  return 0;
}

int read_dataFrame(int fd, unsigned char *buffer, flags_t *flags)
{
  unsigned char current_bcc2 = 0;
  unsigned char byte;
  unsigned int current_index = 0;
  int state = START;

  while (state != END)
  {
    if (read(fd, &byte, 1) < 0)
      return -1;

    state = readSM(byte, state);

    if (state == C_RCV)
    { //Checking for repeated data
      if (byte == CONTROL_0 && NR == 1)
      {
        printf("REPEATED DATA 0\n");
        write_SUframe(fd, RR_1);
        flags->repeated_data = TRUE;
      }
      else if (byte == CONTROL_1 && NR == 0)
      {
        printf("REPEATED DATA 1\n");
        write_SUframe(fd, RR_0);
        flags->repeated_data = TRUE;
      }
      else if (byte == C_DISC)
      {
        printf("Received DISC\n");
        flags->send_disc = TRUE;
      }
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

  return current_index;
}

void writeResponse(int fd, unsigned int data_ok)
{
  if (data_ok)
  {
    if (NR == 1)
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
    if (NR == 1)
      write_SUframe(fd, REJ_1);
    else
      write_SUframe(fd, REJ_0);
  }
}
