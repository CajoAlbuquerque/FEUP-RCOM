#include "protocol.h"
#include "macros.h"
#include "state_machines/statemachine.h"
#include <string.h>

volatile int STOP = FALSE;

int llopen(int port, int mode) {
    unsigned char byte[1];
    unsigned char set[SET_FRAME_SIZE];
    int fd, res;

  switch (mode) {
  case TRANSMITTER:
    set_transmission();

    alarm(3);
    while (!STOP) {
      res = read(fd, byte, 1);

      if (res > 0) {
        alarm(0);
      }

      if (openSM(byte[0], C_UA))
        STOP = TRUE;
  }
    break;

  case RECEIVER:
    while (!STOP) {
      res = read(fd, byte, 1);
      if (res < 0) {
        perror("llopen");
        exit(-1);
      }

      if (openSM(byte[0], C_SET))
        STOP = TRUE;
    }

    set[F1_INDEX] = FLAG;
    set[A_INDEX] = A;
    set[C_INDEX] = C_UA;
    set[BCC_INDEX] = A ^ C_UA;
    set[F2_INDEX] = FLAG;

    if (write(fd, set, SET_FRAME_SIZE) < 0) {
      perror("llopen");
      exit(-1);
    }
    break;

  default:
    break;
  }
}