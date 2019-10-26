#include <stdio.h>

struct applicationLayer
{
    int fileDescriptor; /* Serial port descriptor */
    int status;         /* TRANSMITTER | RECEIVER */
} applicationLayer;

struct applicationLayer application;

/**
 *  gets all the chars of the file given to transmit and sends the pointer to them
 *
 *  @param filename name of the file to transmit given by the user
 *  @param fileSize size of the file to transmit know after get the chars
 *
 *  @return all the chars in the file
 */
unsigned char *getCharBuffer(char *filename, int *fileSize);

/**
 * creates the data package to send to llwrite
 *
 * @param sendSize size of the part of the file we are going to send
 * @param sequenceNumber number of files already send
 * @param data read of the file
 * @param packet buffer with the complete packet(4 bytes + data)
 *
 *  @return data package
 */
int sendDataPacket(int sendSize, int sequenceNumber, unsigned char *data, unsigned char *packet);

/**
 * creates the control package
 *
 * @param control 2 if start and 3 if end package control
 * @param fileSize of the all file
 * @param filename name of the file to transmit
 *
 *  @return 0 if success , -1 otherwise
 */
int sendControlPacket(unsigned int control, int fileSize, char *filename);

/**
 * sends a file
 *
 * @param filename name of the file to transmit given by the TRANSMITTER
 * @return 0 if success
 */
int sendFile(char *filename);

/**
 * receives the control package
 *
 * @param control expected control, 2 if start and 3 if end
 * @param filename gives the name of the file receiving (control 2) ou checks if it was well send(control 3)
 *
 *  @return success
 */
int receiveControlPacket(int control,unsigned char *filename);

/**
 *  receives a file
 *
 *  @return 0 if success
 */
int receiveFile();

/**
 * receives the packets with data
 * @param sendFile file where the data is going to be, the same name of the original data
 * @param fileWritten number of written chars, used to know when to stop 
 */ 
int receiveDataPacket(FILE *sendFile, int *fileWritten);