
/**
 *  Writes a supervision or an unumbered frame to the serial port.
 * 
 *  @param fd Serial port file descriptor
 *  @param control Expected control character value
 *  @return 0 in case of success; -1 on error.
 */
int write_suFrame(int fd, unsigned char control)