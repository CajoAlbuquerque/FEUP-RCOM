#define START 0
#define FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_OK 4

/**
 *  Receiver state machine. Processes the set message.
 * 
 *  @param byte Character to be processed
 */
int receiverSM(unsigned char byte);
