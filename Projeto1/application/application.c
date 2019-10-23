#include "application.h"
#include "protocol.h"

#include <stdio.h>
#include <sys/types.h>

unsigned char* getCharBuffer(unsigned char *filename, int *fileSize){
    FILE *f;
    unsigned char *fileData;
    struct stat fData;
    if((f = open((char *) filename, "rb")) == NULL)
    {
        perror("Error opening the file :");
        exit(-1);
    }
    stat((char*) filename, &fData);
    (*fileSize) = fData.st_size;

    printf("Get %ld bytes form file %d\n", *fileSize, *filename );

    fileData = (unsigned char *)malloc(*fileSize);

    fread(fileData, sizeof(unsigned char), *fileSize, f);
    return fileData;

}
unsigned char* dataPacket(int sendSize, int sequenceNumber, unsigned char* filename){
    int count = 0;
    unsigned char *fileData = (unsigned char *)malloc(sendSize);

    fileData[0] = sendSize;
    fileData[1] = sequenceNumber;

    int L2 =  1;
    int L1 = sendSize - 256 * L2;

    fileData[2] = L2;
    fileData[3] = L1;
    while(count <= sendSize){
        fileData[4+count] = filename + count;
    }

    return fileData;
}

int controlPacket(unsigned int control, int fileSize, unsigned char filename){
    int count = 0;
    unsigned char set[7];

    set[0] = control;
    set[1] = fileSize;

 //TODO

    return 0;
}

int sendFile(unsigned char *filename){
    int fileSize, sendSize = 0, sequenceNumber = 0;
    unsigned char *fileData, *dataSend;
    fileData = getCharBuffer((unsigned char*) filename, &fileSize);
    controlPacket(0x02, fileSize, filename);
    while((fileSize - sendSize) > 200){ //max is 256
        *dataSend = dataPacket(200, sequenceNumber, filename);
        llwrite(application.fileDescriptor, dataSend, &fileSize);
        free(dataSend);
    } 
    if((fileSize - sendSize) > 0){ //max is 256
        *dataSend = dataPacket((fileSize - sendSize), sequenceNumber, filename);
        llwrite(application.fileDescriptor, dataSend, &fileSize);
        free(dataSend);
    } 
    
    controlPacket(0x03, fileSize, filename);
}

int main(int argc, char **argv){
    unsigned char *fileData;
    application.status = argv[2];

    if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
        printf("Usage: SerialPort\n\tex: /dev/ttyS1\n");
        exit(1);
    }

    if((strcmp("TRANSMITTER",application.status) !=0) && strcmp("RECEIVER", application.status) !=0) {
        printf("Usage: SerialPort\n\tex: /dev/ttyS1\n TRANSMITTER || RECEIVER \n");
        exit(1);
    }

    if(strcmp("TRANSMITTER", application.status) == 0 && argc < 3){
        printf("Usage: SerialPort\n\tex: /dev/ttyS1\n TRANSMITTER || RECEIVER \n filename\n");
        exit(1);
    }

    application.fileDescriptor = llopen(argv[0], argv[1]);

    if(strcmp("TRANSMITTER",argv[2]) == 0){
        sendFile(argv[3]);
        llclose(application.fileDescriptor);
    } else {
        llread(application.fileDescriptor, fileData);
    }
    
    printf("Sucessfull transmition\n");

}