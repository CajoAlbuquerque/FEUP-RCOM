#include "alarm.h"
#include "../macros.h"
#include "../transmitter/transmitter.h"
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// This variables are static so no one can access them from other files.
static int serial_fd;    /**< Serial port file descriptor. Used for frame retransmissions. */
static int timeouts = 0; /**< Number of timeouts ocurred. */
static phase_t phase;    /**< Connection's phase. Can be open, data or close. */

void setPhase(phase_t new_phase)
{
  phase = new_phase;
}

void resetTimeouts() { timeouts = 0; }

int initializeHandler(int fd)
{
  struct sigaction new_action;
  serial_fd = fd;

  memset(&new_action, 0, sizeof(new_action));
  new_action.sa_handler = timeoutHandler;

  if (sigaction(SIGALRM, &new_action, NULL) < 0)
  {
    return -1;
  }

  return 0;
}

void timeoutHandler(int signo)
{
  if (signo != SIGALRM)
    return;

  timeouts++;
  if (timeouts <= TIMEOUT_MAX_ATTEMPTS)
  {
    printf("Timeout\n");
    switch (phase)
    {
    case open_phase:
      write_suFrame(serial_fd, C_SET);
      printf("Resent SET\n");
      break;
    case data_phase:
      sendMessage(serial_fd);
      printf("Resent Data\n");
      break;
    case close_phase:
      write_suFrame(serial_fd, C_DISC);
      printf("Resent DISC\n");
      break;
    
    default:
      break;
    }

    // Alarm is set again
    alarm(TIMEOUT_INTERVAL);
  }
  else
  {
    fprintf(stderr, "Timeout: other end took too long to respond.\n");
    exit(-1);
  }

  return;
}
