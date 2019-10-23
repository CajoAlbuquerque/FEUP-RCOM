/**
 *  State machine to process a supervision or unumbered frame.
 *  The frame's expected control character must be passed as an argument.
 *
 *  @param byte Character to be processed
 *  @param control Expected control character value
 *  @param state State of the machine before reading byte
 *  @return State of the machine after reading byte
 */
int suFrameSM(unsigned char byte, unsigned char control, int state);

/**
 * 	Receiver state machine to parse the data frame.
 *
 * 	@param byte Character to be processed
 *  @param state State of the machine before reading byte
 *  @return State of the machine after reading byte
 */
int readSM(unsigned char byte, int state);


/**
 * 	Transmitter state machine to parse the response frame.
 *
 * 	@param byte Character to be processed
 *  @param state State of the machine before reading byte
 *  @return State of the machine after reading byte
 */
int writeSM(unsigned char byte, int state);
