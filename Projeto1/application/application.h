struct applicationLayer
{
    int fileDescriptor; /* Descritor correspondente à porta série */
    int status; /* TRANSMITTER | RECEIVER */     
} applicationLayer;

struct applicationLayer application;

/**
 *  gets the chars of the file given to transmit
 *
 *  @param filename name of the file to transmit
 *  @param fileSize size of the file to transmit
 *
 *  @return Cchars with the content of the file
 */
unsigned char* getCharBuffer(char filename, int *fileSize);

/**
 * creats the data package to send to llwrite
 *
 * @param sendSize size of the part of the file we are going to send
 * @param sequeceNumber number of files a ready send
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
 * creats the control package
 *
 * @param control 2 if start and 3 if end
 * @param size of the all file
 * @param filename name of the file to transmit
 *
 *  @return sucess
 */
int controlPacket(unsigned int control, int fileSize, unsigned char filename);

/**
 *  sends a file
 *
 * @param filename name of the file to transmit
 *  @return 
 */
int sendFile(char filename);

int receivePacket(unsigned int controlE, unsigned char *filename);

