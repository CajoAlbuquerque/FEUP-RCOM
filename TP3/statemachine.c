#include "statemachine.h"

int isControl(unsigned char byte) {
  return  byte == CONTROL_0 || byte == CONTROL_1 || byte == C_DISC;
}

int openSM(unsigned char byte, unsigned char control) {
  static int state = START;
  switch (state) {
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
    if (byte == FLAG){
      state = START;
      return TRUE;
    }
    else
      state = START;
    break;
  }

  return FALSE;
}

int readSM(unsigned char byte, int state) {
  static unsigned char control;
  switch (state) {
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
    else if (isControl(byte)){
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
