#include "statemachine.h"
#include "../macros.h"

int suFrameSM(unsigned char byte, unsigned char control, int state)
{
  switch (state)
  {
  case START:
    if (byte == FLAG)
      state = FLAG_RCV;
    break;
  case FLAG_RCV:
    if (byte == FLAG)
      state = FLAG_RCV;
    else if (byte == A)
      state = A_RCV;
    else
      state = START;
    break;
  case A_RCV:
    if (byte == FLAG)
      state = FLAG_RCV;
    else if (byte == control)
      state = C_RCV;
    else
      state = START;
    break;
  case C_RCV:
    if (byte == (A ^ control))
      state = BCC_OK;
    else if (byte == FLAG)
      state = FLAG_RCV;
    else
      state = START;
    break;
  case BCC_OK:
    if (byte == FLAG)
      state = END;
    else
      state = START;
    break;
  }

  return state;
}

int readSM(unsigned char byte, int state)
{
  static unsigned char control;
  switch (state)
  {
  case START:
    if (byte == FLAG)
      state = FLAG_RCV;
    break;
  case FLAG_RCV:
    if (byte == FLAG)
      state = FLAG_RCV;
    else if (byte == A) {
      state = A_RCV;
    }
    else
      state = START;
    break;
  case A_RCV:
    if (byte == FLAG) {
      state = FLAG_RCV;
    }
    else if (byte == CONTROL_0 || byte == CONTROL_1 || byte == C_DISC)
    {
      control = byte;
      state = C_RCV;
    }
    else {
      state = START;
    }
    break;
  case C_RCV:
    if (byte == (A ^ control))
      state = BCC_OK;
    else if (byte == FLAG)
      state = FLAG_RCV;
    else
      state = START;
    break;
  case BCC_OK:
    if (byte == FLAG && control == C_DISC)
      state = END;
    else if (byte == FLAG)
      state = FLAG_RCV;
    else
      state = DATA_LOOP;
    break;
  case DATA_LOOP:
    if (byte == FLAG)
      state = END;
    break;
  }

  return state;
}

int writeSM(unsigned char byte, int state)
{
  static unsigned char control;
  switch (state)
  {
  case START:
    if (byte == FLAG)
      state = FLAG_RCV;
    break;
  case FLAG_RCV:
    if (byte == FLAG)
      state = FLAG_RCV;
    else if (byte == A)
      state = A_RCV;
    else
      state = START;
    break;
  case A_RCV:
    if (byte == FLAG)
      state = FLAG_RCV;
    else if (byte == RR_0 || byte == RR_1 || byte == REJ_0 || byte == REJ_1)
    {
      control = byte;
      state = C_RCV;
    }
    else
      state = START;
    break;
  case C_RCV:
    if (byte == (A ^ control))
      state = BCC_OK;
    else if (byte == FLAG)
      state = FLAG_RCV;
    else
      state = START;
    break;
  case BCC_OK:
    if (byte == FLAG)
      state = END;
    else
      state = START;
    break;
  }

  return state;
}
