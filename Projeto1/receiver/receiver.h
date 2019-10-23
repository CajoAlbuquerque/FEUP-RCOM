#pragma once

typedef struct flags {
    unsigned int data_ok;
    unsigned int repeated_data;
    unsigned int nr_value;
    unsigned int escape_byte;
    unsigned int send_disc;
} flags_t;

/**
 *  Changes NR value
 *  
 *  @param new_nr New NR value
 */
void setNR(unsigned int new_nr);

/**
 *  Access NR value
 *  
 *  @return NR value
 */
unsigned int getNR();

/**
 * Initializes the flags_t struct with everything false.
 */
void initFlags(flags_t *flags);

/**
 *  Reads a supervision or an unumbered frame from the serial port.
 * 
 *  @param fd Serial port file descriptor
 *  @param control Expected control character value
 *  @return 0 in case of success; -1 on error.
 */
int read_suFrame(int fd, unsigned char control);

/**
 *  Reads a data frame from the serial port.
 * 
 *  @param fd File descriptor of the serial port
 *  @param buffer Output buffer with the data content
 *  @param flags Output argument with the error and related flags
 *  @return Number of read characters (buffer length) in case of success; -1 in case of error
 */
int read_dataFrame(int fd, unsigned char *buffer, flags_t *flags);