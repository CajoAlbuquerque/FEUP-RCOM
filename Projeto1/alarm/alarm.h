#pragma once

typedef enum { 
    open_phase,
    data_phase,
    close_phase,
    receiver_phase
} phase_t;

/**
 *  Sets phase variable value.
 * 
 *  @param new_phase New phase value
 */
void setPhase(phase_t new_phase);

/**
 *  Resets the number of timeouts to 0.
 */
void resetTimeouts();

/**
 *  Installs the timeoutHandler for SIGALRM.
 *  Also sets the serial port file descriptor to be used on retransmissions.
 * 
 *  @param fd Serial port file descriptor
 *  @return 0 in case of success; -1 on error.
 */
int initializeHandler(int fd);

/**
 *  Handles SIGALRM. Calls serial port transmission methods to retransmit frames.
 * 
 *  @param signo Signal number of the received signal
 */
void timeoutHandler(int signo);