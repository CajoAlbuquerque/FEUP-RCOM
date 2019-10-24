struct applicationLayer
{
    int fileDescriptor; /* Serial port descriptor */
    int status; /* TRANSMITTER | RECEIVER */     
} applicationLayer;

struct applicationLayer application;

/**
 *  gets all the chars of the file given to transmit and sends the pointer to them
 *
 *  @param filename name of the file to transmit
 *  @param fileSize size of the file to transmit
 *
 *  @return all the chars in the file
 */
unsigned char* getCharBuffer(char filename, int *fileSize);

/**
 * creates the data package to send to llwrite
 *
 * @param sendSize size of the part of the file we are going to send
 * @param sequenceNumber number of files a ready send
 * @param filename name of the file to transmit
 *
 *  @return data package
 */
unsigned char* dataPacket(int sendSize, int sequenceNumber, unsigned char* filename);

/**
 *  receives a file
 *
 *
 *  @return 
 */
int receiveFile();

/**
 * creates the control package
 *
 * @param control 2 if start and 3 if end
 * @param size of the all file
 * @param filename name of the file to transmit
 *
 *  @return success
 */
int controlPacket(unsigned int control, int fileSize, unsigned char filename);

/**
 *  sends a file
 *
 * @param filename name of the file to transmit
 *  @return 
 */
int sendFile(char filename);

/**
 * receives the control package
 *
 * @param expected control, 2 if start and 3 if end
 * @param filename name of the file receiving
 *
 *  @return success
 */
int receiveControlPacket2(unsigned char *filename);

