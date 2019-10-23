#include "application.h"
#include "../protocol/protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

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

    printf("Get %d bytes form file %d\n", *fileSize, *filename );

    fileData = (unsigned char *)malloc(*fileSize);

    fread(fileData, sizeof(unsigned char), *fileSize, f);
    return fileData;
}

unsigned char* dataPacket(int sendSize, int sequenceNumber, unsigned char* data){
    int count = 0;
    unsigned char *packet = (unsigned char *)malloc(sendSize);

    packet[0] = 1;
    packet[1] = sequenceNumber;

    int L2 = sendSize/256;
    int L1 = sendSize - L2;

    packet[2] = L2;
    packet[3] = L1;
    while(count <= sendSize){
        packet[4+count] = data[count + sequenceNumber *150];
        count++;
    }

    return packet;
}

int receiveFile(){
    int L1, L2, counter;
    //int fileSize;
    int receiveSize = 0, sequenceNumber = 0, readSize;
    unsigned char *fileData, *dataReceive;

    dataReceive= (unsigned char *)malloc(155 * sizeof(unsigned char));
    readSize = llread(application.fileDescriptor, fileData);
    receiveSize = fileData[0];
    if(receiveSize < 0){
        printf("Wrong receive size\n");
        exit(-1);
    }
    sequenceNumber = fileData[1];
    if( sequenceNumber < 0){
        printf("Wrong sequece number\n");
        exit(-1);
    }
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
    
    return 0;
}

unsigned char * controlPacket(unsigned int control, int fileSize, unsigned char filename){
    if(control != 2 && control != 3){
        printf("control can't be different than 2 and 3");
        exit(-1);
    }

    unsigned char set[7];
    set[0] = control;
    set[1] = fileSize;
    set[2] = filename;

 //TODO

    return set;
}

int sendFile(char *filename){
    int fileSize, sendSize = 0, sequenceNumber = 0;
    unsigned char *fileData, *dataSend;

    fileData = getCharBuffer( filename, &fileSize);
    controlPacket(2, fileSize, *filename);

    while((fileSize - sendSize) >= 150){ //if possible sends 150 bytes of data
        dataSend = dataPacket(150, sequenceNumber, fileData);
        llwrite(application.fileDescriptor, dataSend, 150 + 4);
        free(dataSend);
        sequenceNumber = sequenceNumber++ % 255;
    } 
    if((fileSize - sendSize) > 0){ 
        dataSend = dataPacket((fileSize - sendSize), sequenceNumber, fileData);
        llwrite(application.fileDescriptor, dataSend, (fileSize - sendSize));
        free(dataSend);
    } 
    
    controlPacket(3, fileSize, *filename);
    return 0;
}


int main(int argc, char **argv){
    if (argc < 3) {
        printf("Usage: %s <serial port number> <TRANSMITTER || RECEIVER> <file name>\n", argv[0]);
        exit(1);
    }

    application.status = argv[2];
    if((strcmp("TRANSMITTER",application.status) != 0) && strcmp("RECEIVER", application.status) !=0) {
        printf("Usage: %s <serial port number> <TRANSMITTER || RECEIVER> <file name>\n", argv[0]);
        exit(1);
    }

    application.fileDescriptor = llopen(atoi(argv[1]), application.status);

    if(strcmp("TRANSMITTER", application.status) == 0){
        sendFile(argv[3]);
        llclose(application.fileDescriptor);
    } else {
        receiveFile();
    }
    
    printf("Sucessfull transmition\n");
    return 0;
}