#define FALSE 0
#define TRUE 1

#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07
#define C_DISC 0x0B
#define CONTROL_0 0x00
#define CONTROL_1 0x40

#define START 0
#define FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_OK 4
#define DATA_LOOP 5
#define BCC2_OK 6
#define END 7

/**
 * 
 */
int isControl(unsigned char byte);

/**
 *  State machine to processes the comunication establishment.
 *
 *  @param byte Character to be processed
 *  @param control Control character value (C_SET for receiver and C_UA for
 * transmitter)
 *  @return 1 if the end state has been reached, 0 otherwise.
 */
int openSM(unsigned char byte, unsigned char control);

/**
 * 	Receiver state machine to parse the data frame.
 *
 * 	@param byte Character to be processed
 *  @return current state
 */
int readSM(unsigned char byte, int state);
