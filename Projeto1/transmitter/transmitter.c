#include "transmitter.h"
#include "../macros.h"
#include "../state_machine/statemachine.h"

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