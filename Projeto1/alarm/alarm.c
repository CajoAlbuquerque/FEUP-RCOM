#include "alarm.h"
#include "../macros.h"
#include "transmitter/transmitter.h"
#include <signal.h>
#include <stdio.h>

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
    if (phase == open) {
      write_suFrame(serial_fd, C_SET); // resend set_transmission
    } else if (phase == data) {
      //   send_message(); TODO: resend data
    } else if (phase == close) {
      write_suFrame(serial_fd, C_DISC);
    }
    // Alarm is set again
    alarm(TIMEOUT_INTERVAL);
  } else {
    fprintf(stderr, "timeout: other end took too long to respond.\n");
    exit(-1);
  }
}