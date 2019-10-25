#include "transmitter.h"
#include "../macros.h"
#include "../state_machine/statemachine.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *msg;
static unsigned int msg_length;
static unsigned int NS = 0;

int write_suFrame(int fd, unsigned char control)
{
  unsigned char set[SU_FRAME_SIZE];

  set[F1_INDEX] = FLAG;
  set[A_INDEX] = A;
  set[C_INDEX] = control;
  set[BCC_INDEX] = A ^ control;
  set[F2_INDEX] = FLAG;

  if (write(fd, set, SU_FRAME_SIZE) < 0)
    return -1;

  return 0;
}

int parseMessage(unsigned char *buffer, int length)
{
  unsigned char bcc2 = 0;
  unsigned char control;
  unsigned int j = 0;

  msg_length = SU_FRAME_SIZE + 2 * (length) + 1;

  msg = (unsigned char *)malloc(msg_length * sizeof(unsigned char));

  if (msg == NULL)
  {
    return -1;
  }

  if (NS == 1)
  {
    control = CONTROL_1;
  }
  else
  {
    control = CONTROL_0;
  }

  msg[F1_INDEX] = FLAG;
  msg[A_INDEX] = A;
  msg[C_INDEX] = control;
  msg[BCC_INDEX] = (A ^ control);

  // printf("%x\n",msg[F1_INDEX]);
  // printf("%x\n",msg[A_INDEX]);
  // printf("%x\n",msg[C_INDEX]);
  // printf("%x\n",msg[BCC_INDEX]);

  for (int i = 0; i < length; i++)
  {
    // Byte stuffing
    if (buffer[i] == ESC)
    {
      msg[BCC_INDEX + i + 1 + j] = ESC;
      j++;
      msg[BCC_INDEX + i + 1 + j] = ESC_SOL; // ESC_SOL = ESC ^ 0x20
    }
    else if (buffer[i] == FLAG)
    {
      msg[BCC_INDEX + i + 1 + j] = ESC;
      j++;
      msg[BCC_INDEX + i + 1 + j] = FLAG_SOL; // FLAG_SOL = FLAG ^ 0x20
    }
    else
    {
      msg[BCC_INDEX + i + 1 + j] = buffer[i];
    }

    /*
      Since bcc2 starts at 0, bcc2 ^ buffer[0] = buffer[0].
      This way we avoid an irrelevant condition.
      Additionally, as buffer remains unchanged, bcc2 wil not be affected by stuffing
    */
    bcc2 = bcc2 ^ buffer[i];
  }

  msg[BCC_INDEX + length + 1 + j] = bcc2;
  msg[BCC_INDEX + length + 2 + j] = FLAG;

  return 0;
}

int sendMessage(int fd)
{
  return write(fd, msg, msg_length);
}

int read_responseFrame(int fd)
{
  int state = START, read_res;
  unsigned char byte;
  unsigned char control;

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

    state = writeSM(byte, state);
    if (state == C_RCV)
    {
      control = byte;
    }
  }

  alarm(0);

  return control;
}

int parseControl(unsigned char control)
{
  if (control == RR_0 || control == RR_1)
  {
    if (control == RR_0)
      NS = 0;
    else
      NS = 1;

    free(msg);
    return TRUE;
  }

  return FALSE;
}
