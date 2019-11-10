#pragma once

/**
 *  Writes a supervision or an unumbered frame to the serial port.
 * 
 *  @param fd Serial port file descriptor
 *  @param control Expected control character value
 *  @return 0 in case of success; -1 on error.
 */
int write_suFrame(int fd, unsigned char control);

/**
 *  Creates the message to send in the data frame based on the content
 *  of buffer.
 *  
 *  @param buffer Array containing the data
 *  @param length Length of the array
 *  @return 0 in case of success; -1 on error
 */
int parseMessage(unsigned char *buffer, int length);

/**
 *  Sends the message through the serial port.
 *  
 *  @param fd File descriptor of the serial port
 *  @return number of characters written
 */
int sendMessage(int fd);

/**
 *  Reads the response to a data frame.
 *  
 *  @param fd File descriptor of the serial port
 *  @return Control character read
 */
int read_responseFrame(int fd);

/**
 *  Parses the control character (RR or REJ) read from the response frame.
 *  
 *  @param control Control character received
 *  @return 1 when RR was received, 0 when REJ was received
 */
int parseControl(unsigned char control);