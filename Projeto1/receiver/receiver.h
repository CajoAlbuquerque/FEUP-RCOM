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

int read_responseFrame(int fd);
