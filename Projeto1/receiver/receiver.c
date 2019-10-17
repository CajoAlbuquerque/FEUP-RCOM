#include "receiver.h"
#include "../macros.h"
#include "../state_machine/statemachine.h"

int read_suFrame(int fd, unsigned char control) {
  int read_res, state = START;
  unsigned char byte;

  while (state != END) {
    read_res = read(fd, &byte, 1);

    if (read_res > 0) {
      alarm(0);
    } else if (read_res < 0) {
      return -1;
    }

    state = suFrameSM(byte, control, state);
  }

  return 0;
}