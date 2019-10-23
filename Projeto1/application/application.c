#include "application.h"
#include "../protocol/protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

unsigned char* getCharBuffer(unsigned char *filename, int *fileSize){
    FILE *f;
    unsigned char *fileData;
    struct stat fData;
    if((f = fopen((char *) filename, "rb")) == NULL)
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
        count++;
    }

    return fileData;
}

int receiveFile(){
    int L1, L2, counter;
    int fileSize, receiveSize = 0, sequenceNumber = 0, readSize;
    unsigned char *fileData, *dataReceive;

    readSize = llread(application.fileDescriptor, fileData);
    receiveSize = fileData[0];
    sequenceNumber = fileData[1];
    L2 = fileData[2];
    L1 = fileData[3];
    if(readSize != 256 *L2 + L1 ){
        printf("Wrong Reception\n");
        exit(-1);
    }
    while(readSize >= counter){
        dataReceive[counter] = fileData[4 + counter];
        counter++;
    }
    
}

int controlPacket(unsigned int control, int fileSize, unsigned char filename){
    if(control != 2 && control != 3){
        printf("control can't be different than 2 and 3");
        exit(-1);
    }
    int count = 0;
    unsigned char set[7];

    set[0] = control;
    set[1] = fileSize;
    set[2] = filename;

 //TODO

    return 0;
}

int sendFile(unsigned char *filename){
    int fileSize, sendSize = 0, sequenceNumber = 0;
    unsigned char *fileData, *dataSend;
    fileData = getCharBuffer((unsigned char*) filename, &fileSize);
    controlPacket(0x02, fileSize, filename);
    while((fileSize - sendSize) > 150){ //if possible sends 150 bits of data
        *dataSend = dataPacket(150, sequenceNumber, filename);
        llwrite(application.fileDescriptor, dataSend, &fileSize);
        free(dataSend);
    } 
    if((fileSize - sendSize) > 0){ 
        *dataSend = dataPacket((fileSize - sendSize), sequenceNumber, filename);
        llwrite(application.fileDescriptor, dataSend, &fileSize);
        free(dataSend);
    } 
    
    controlPacket(0x03, fileSize, filename);
}


int main(int argc, char **argv){
    
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

    application.fileDescriptor = llopen(argv[1], application.status);

    if(strcmp("TRANSMITTER", application.status) == 0){
        sendFile(argv[3]);
        llclose(application.fileDescriptor);
    } else {
        receiveFile();
    }
    
    printf("Sucessfull transmition\n");

}