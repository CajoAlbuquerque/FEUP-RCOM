#include "alarm.h"
#include "../macros.h"
#include "../transmitter/transmitter.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// This variables are static so no one can access them from other files.
static int serial_fd; /**< Serial port file descriptor. Used for frame retransmissions. */
static int timeouts = 0; /**< Number of timeouts ocurred. */
static phase_t phase; /**< Connection's phase. Can be open, data or close. */

void setPhase(phase_t new_phase) { phase = new_phase; }
                                                                                                                                                                   
void resetTimeouts() { timeouts = 0; }

int initializeHandler(int fd) {
  serial_fd = fd;
  if (signal(SIGALRM, timeoutHandler) == SIG_ERR) {
    return -1;
  }

  return 0;
}

void timeoutHandler(int signo) {
  if (signo != SIGALRM)
    return;

  timeouts++;
  if (timeouts < TIMEOUT_MAX_ATTEMPTS) {
    if (phase == open_phase) {
      write_suFrame(serial_fd, C_SET);
    } else if (phase == data_phase) {
      printf("Alarm on data phase\n");
      sendMessage(serial_fd); //TODO: FIX
      printf("Resent Data\n");
    } else if (phase == close_phase) {
      write_suFrame(serial_fd, C_DISC);
    }
    // Alarm is set again
    alarm(TIMEOUT_INTERVAL);
  } else {
    fprintf(stderr, "timeout: other end took too long to respond.\n");
    exit(-1);
  }
}
