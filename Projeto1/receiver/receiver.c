#include "receiver.h"
#include "../macros.h"
#include "../state_machine/statemachine.h"

void initFlags(flags_t flags){
  flags.data_ok = FALSE;
  flags.repeated_data = FALSE;
  flags.escape_byte = FALSE;
  flags.send_disc = FALSE;
}

int read_suFrame(int fd, unsigned char control) {
  int read_res, state = START;
  unsigned char byte;

  alarm(3);
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

int read_responseFrame(int fd) {
  int read_res, state = START;
  unsigned char byte;
  unsigned char control;

	alarm(3);
  while (state != END) {
    read_res = read(fd, &byte, 1);

    if (read_res > 0) {
      alarm(0);
    } else if (read_res < 0) {
      return -1;
    }

    state = suFrameSM(byte, state);
	if(state == C_RCV) {
		control = byte;
  	}
  }

  return control;
}
