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

int writeResponse(int fd, unsigned int data_ok)
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
    if(NR == 1)
      write_SUframe(fd, REJ_1);
    else
      write_SUframe(fd, REJ_0);
  }
}
