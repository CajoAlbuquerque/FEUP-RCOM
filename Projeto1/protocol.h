// Data connection protocol services
#define COM0 "/dev/ttyS0"
#define COM1 "/dev/ttyS1"
#define COM2 "/dev/ttyS2"
#define COM3 "/dev/ttyS3"
#define TRANSMITTER 0
#define RECEIVER 1

/**
 *  Opens the connection on the serial port.
 *
 *  @param port Serial port to use; use values from 0 to 3
 *  @param mode Flag to switch between TRANSMITTER and RECEIVER modes
 *
 *  @return Connection port file descriptor if OK, -1 otherwise.
 */
int llopen(int port, int mode);

/**
 *  Reads a data frame from the serial port.
 *
 *  @param fd File descriptor of the serial port
 *  @param buffer Array of characters on which the data will be saved
 *
 *  @return Number of characters read (buffer length) if OK, -1 otherwise.
 */
int llread(int fd, unsigned char *buffer);

/**
 *  Writes a data frame to the serial port.
 *
 *  @param fd File descriptor of the serial port
 *  @param buffer Array of characters to be written
 *  @param length Array's length
 *
 *  @return Number of written characters if OK, -1 otherwise.
 */
int llwrite(int fd, unsigned char *buffer, int length);

/**
 *  Closes the connection on the serial port.
 *
 *  @param fd File descriptor of the serial port
 *
 *  @return 0 if OK, -1 otherwise.
 */
int llclose(int fd);