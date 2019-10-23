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

int dataPacket(int sendSize){
  /*  unsigned char set[SU_FRAME_SIZE];

    set[F1_INDEX] = FLAG;
    set[A_INDEX] = A;
    set[C_INDEX] = control;
    set[BCC_INDEX] = A ^ control;
    set[F2_INDEX] = FLAG;*/

    if (write(fd, set, SU_FRAME_SIZE) <= 0) {
        return -1;
    }
    return 0;
}

int sendFile(unsigned char *filename){
    int fileSize, sendSize = 0;
    unsigned char *fileData;
    fileData = getCharBuffer((unsigned char*) filename, &fileSize);
    if((fileSize - sendSize) > 200){ //max is 258
        dataPacket(200);
    } 
    llwrite(application.fileDescriptor, fileData, &fileSize);
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

    application.fileDescriptor = llopen(argv[0], argv[1]);

    if(strcmp("TRANSMITTER",argv[2]) == 0){
        sendFile(argv[3]);
        llclose(application.fileDescriptor);
    } else {
        llread(application.fileDescriptor, fileData);
    }
    
    printf("Sucessfull transmition\n");

}