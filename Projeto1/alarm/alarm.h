typedef enum { 
    open,
    data,
    close
} phase_t;

// This variables are static so no one can access them from other files.
static int serial_fd; /**< Serial port file descriptor. Used for frame retransmissions. */
static int timeouts = 0; /**< Number of timeouts ocurred. */
static phase_t phase; /**< Connection's phase. Can be open, data or close. */

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