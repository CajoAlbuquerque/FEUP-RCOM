#include "transmitter.h"
#include "../macros.h"
#include "../state_machine/statemachine.h"

static unsigned char *msg;
static unsigned int NS = 0;

int write_suFrame(int fd, unsigned char control) {
  unsigned char set[SU_FRAME_SIZE];

  set[F1_INDEX] = FLAG;
  set[A_INDEX] = A;
  set[C_INDEX] = control;
  set[BCC_INDEX] = A ^ control;
  set[F2_INDEX] = FLAG;

  if (write(fd, set, SU_FRAME_SIZE) <= 0) {
    return -1;
  }

  return 0;
}

int parseMessage(char *buffer, int length) {
  unsigned char bcc2 = 0;
  unsigned char control;

  msg = (unsigned char *) calloc(SET_SIZE + 2 * (length) + 1, sizeof(unsigned char));
  NS ? control = CONTROL_1 : CONTROL_0;

  msg[F1_INDEX] = FLAG;
  msg[A_INDEX] = A;
  msg[C_INDEX] = control;
  msg[BCC_INDEX] = (A ^ control);

  for (int i = 0; i < length; i++) {
    // Byte stuffing
    if (buffer[i] == ESC) {
      msg[BCC_INDEX + i + 1 + j] = ESC;
      j++;
      msg[BCC_INDEX + i + 1 + j] = ESC_SOL; // ESC_SOL = ESC ^ 0x20
    }
    else if (buffer[i] == FLAG) {
      msg[BCC_INDEX + i + 1 + j] = ESC;
      j++;
      msg[BCC_INDEX + i + 1 + j] = FLAG_SOL; // FLAG_SOL = FLAG ^ 0x20
    }
    else {
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
}


int sendMessage(int fd) {
  unsigned char control;

  write(fd, buffer, length);
  read_responseFrame(fd);

  return control;
}

int parseControl(unsigned char control) {
  if(control == REJ_0) {
	return FALSE;
  }
  else if (control == REJ_1) {
	return FALSE;
  }
  else if (control == RR_0) {
	free(msg);
	NS = 0;
	return TRUE;
  }
  else if (control == RR_1) {
	free(msg);
	NS = 1;
	return TRUE;
  }
}
