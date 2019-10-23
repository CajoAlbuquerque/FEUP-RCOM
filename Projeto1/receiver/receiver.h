typedef struct flags {
    unsigned int data_ok;
    unsigned int repeated_data;
    unsigned int escape_byte;
    unsigned int send_disc;
} flags_t;

/**
 * Initializes the flags_t struct with everything false.
 */
void initFlags(flags_t flags);

/**
 *  Reads a supervision or an unumbered frame from the serial port.
 * 
 *  @param fd Serial port file descriptor
 *  @param control Expected control character value
 *  @return 0 in case of success; -1 on error.
 */
int read_suFrame(int fd, unsigned char control);

/**
 *  Writes the response to a data frame on the serial port.
 * 
 *  @param fd File descriptor of the serial port
 *  @param data_ok Flag indicating if the data received was OK
 */
void writeResponse(int fd, unsigned int data_ok);

/**
 *  Reads a data frame from the serial port.
 * 
 *  @param fd File descriptor of the serial port
 *  @param buffer Output buffer with the data content
 *  @param flags Output argument with the error and related flags
 *  @return Number of read characters (buffer length) in case of success; -1 in case of error
 */
int read_dataFrame(int fd, unsigned char *buffer, flags_t *flags);