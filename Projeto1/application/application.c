#include "application.h"
#include "../macros.h"
#include "../protocol/protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

// Functions of the transmitter
unsigned char *getCharBuffer(char *filename, int *fileSize)
{
    FILE *f;
    unsigned char *fileData;
    struct stat fData;
    if ((f = fopen(filename, "rb")) == NULL)
    {
        perror("Error opening the file :");
        exit(-1);
    }
    stat(filename, &fData);
    (*fileSize) = fData.st_size;

    printf("Get %d bytes form file %s\n", *fileSize, filename);

    fileData = (unsigned char *)malloc(*fileSize);

    fread(fileData, sizeof(unsigned char), *fileSize, f);

    fclose(f);
    return fileData;
}

int dataPacket(int sendSize, int sequenceNumber, unsigned char *data, unsigned char *packet)
{
    int count = 0;

    packet[0] = 1;
    packet[1] = sequenceNumber;

    int L2 = sendSize / 256;
    int L1 = sendSize - (256 * L2);

    packet[2] = L2;
    packet[3] = L1;
  
    while (count < sendSize)
    {
        packet[4 + count] = data[count + (sequenceNumber * TRANSMIT_SIZE)];
        printf("%c", packet[4 + count]);
        count++;
    }

    return 0;
}

int controlPacket(unsigned int control, int fileSize, char *filename)
{
    if (control != 2 && control != 3)
    {
        printf("control can't be different than 2 and 3");
        exit(-1);
    }

    unsigned char *set;
    set = (unsigned char *)malloc(25 *sizeof(char));

    int i = 0;
    size_t fileLength = strlen(filename);

    set[0] = control;
    //File Size
    set[1] = 0;
    set[2] = sizeof(fileSize);

    printf("size of the file: %d\n",fileSize);
    i = 3 + sizeof(fileSize);
    set[3] = fileSize;
    printf("set [3] = %d\n", set[3]);
    printf("set [4] = %d\n", set[4]);
    printf("i: %d\n", i);
    // File Name
    set[i] = 1;
    set[i + 1] = fileLength + 1;
    
    printf("%ld", fileLength);
    for(size_t j = 0; j <= (fileLength+1); j++) {
        if(j == fileLength){
            printf("total: %d\n", (i + 1 + j));
            set[i+1+j] = '\0';
        }
        set[i+1+j] = filename[j];
    }
   
    llwrite(application.fileDescriptor, set, sizeof(set));
    free(set);
    return 1;
}

int sendFile(char *filename)
{
    int fileSize;
    int sendSize = 0, sequenceNumber = 0;
    unsigned char *fileData, *dataSend;

    dataSend = (unsigned char *)malloc(TRANSMIT_SIZE * sizeof(char));

    fileData = getCharBuffer(filename, &fileSize);
    //controlPacket(2, fileSize, filename);
    //printf("Sended control packet with control 2\n");

    while ((fileSize - sendSize) >= TRANSMIT_SIZE)
    { //if possible sends TRANSMIT_SIZE bytes of data
       
        sendSize += TRANSMIT_SIZE; //each time only TRANSMIT_SIZE are really data
        dataPacket(TRANSMIT_SIZE, sequenceNumber, fileData, dataSend );
        llwrite(application.fileDescriptor, dataSend, TRANSMIT_SIZE + 4);      

        printf("Send size: %d \n", sendSize );
        sequenceNumber = (sequenceNumber + 1) % 255; //sequencial number in modules of 255

    }
    if ((fileSize - sendSize) > 0)
    {
        printf("Send last %d bytes data \n",(fileSize - sendSize) );
        dataPacket((fileSize - sendSize) , sequenceNumber, fileData, dataSend);
        llwrite(application.fileDescriptor, dataSend, ((fileSize - sendSize)+4));
        sendSize += (fileSize - sendSize);
        
    }
    printf("sendSize: %d\n", sendSize);

   // printf("Sending control packet with control 3\n");
   // controlPacket(3, fileSize, filename);

   free(dataSend);
    return 0;
}

int receiveControlPacket2(unsigned char *filename)
{
    unsigned char* set;
    int sizeoffileSize, sizeoffilename;
    int fileSize;
    set = (unsigned char *)malloc(25 *sizeof(char));

    int readBytes = llread(application.fileDescriptor, set);
    printf("%d\n", readBytes);
    printf("Done llRead\n");
    printf("%d", set[0]);

    int i = 1;
    if (atoi(set[0]) != 2)
    {
        printf("Control different then the expected\n");
        exit(-1);
    }
    while (set[i] != '\0')
    {
        printf("y");
        if (set[i] == 0)
        {
            sizeoffileSize = set[i + 1];
            fileSize = set[i + 2];
            i = i + 2 + sizeoffileSize;
        }
        else if (set[i] == 1)
        {
            sizeoffilename = set[i + 1];
            *filename = set[i + 2];
            i = i + 2 + sizeoffilename;
        }
        else
        {
            return -1;
        }
    }
    free(set);
    return fileSize;
}

int receiveControlPacket3(unsigned char *filename, unsigned char *set)
{
    int sizeoffileSize, sizeoffilename;
    int fileSize;

    int i = 1;

    if (atoi(set[0]) != 3)
    {
        printf("Control different then the expected\n");
        exit(-1);
    }
    while (set[i] != '\0')
    {
        if (set[i] == 0)
        {
            sizeoffileSize = set[i + 1];
            fileSize = set[i + 2];
            i = i + 2 + sizeoffileSize;
        }
        else if (set[i] == 1)
        {
            sizeoffilename = set[i + 1];
            *filename = set[i + 2];
            i = i + 2 + sizeoffilename;
        }
        else
        {
            return -1;
        }
    }
    return fileSize;
}

//Functions of the receiver
int receiveDataPacket(FILE *sendFile, unsigned char *filename, int *fileWritten)
{
    int control;
    int L1, L2;
    static int sequenceNumber = 0;
    int readSize, receiveSequenceNumber;
    unsigned char fileData[TRANSMIT_SIZE +4];
    unsigned char *data;

    readSize = llread(application.fileDescriptor, fileData);

    data = (unsigned char *)malloc((readSize-4) *sizeof(unsigned char)); //the size of data the size of the effective data

    printf("Read Size: %d\n", readSize);

    *fileWritten += (readSize - 4); //control, sequence number and L1 and L2 aren't data

    control = fileData[0];
 /*   if (control == 3)
    {
        receiveControlPacket3(filename, fileData);
    } 
    else */if (control == 2)
    {
        return -1;
    }
    else if (control < 0)
    {
        printf("Wrong control receive\n");
        exit(-1);
    }

    receiveSequenceNumber = fileData[1];

    if (receiveSequenceNumber != sequenceNumber)
    {
        printf("Packet not receive, wrong sequence number\n");
        exit(-1);
    } 
    sequenceNumber = (sequenceNumber + 1) % 255;
    L2 = fileData[2];
    L1 = fileData[3];

    if ((readSize-4)  != ((256 * L2) + L1))
    {
        printf("Wrong Reception, L1 and L2 don't mach readSize \n");
        exit(-1);
    }

    for(int i =0 ; i < (readSize-4); i++)
    {
        data[i] = fileData[4 + i];
        printf("%c", data[i]);

    } 
    fputs(data, sendFile);

    free(data);

    return 0;
}

int receiveFile()
{
    FILE *sendFile;
    int fileSize, fileWritten = 0;
    unsigned char *filename;
    filename = (unsigned char *)malloc(25*sizeof(char));

   /* fileSize = receiveControlPacket2(filename);   
    printf("File Size: %d\n", fileSize);*/
    //for testing only!
    //fileSize = 10968;
    //filename = "pinguim1.gif";

   
   fileSize = 897;
   filename = "Wor.txt";


    printf("Size of the file %d\n", fileSize);

    sendFile = fopen(filename, "a");
    if(sendFile == NULL){
        printf("couldn't open file\n");
        exit(-1);
    }
    while (fileWritten < fileSize)
    {
        receiveDataPacket(sendFile, filename, &fileWritten);
        printf("file Written: %d\n", fileWritten);
    }
    fclose(sendFile);
    return 0;
}

int main(int argc, char **argv)
{   
    if (argc < 3 || (strcmp("TRANSMITTER", argv[2]) != 0 && strcmp("RECEIVER", argv[2]) != 0))
    {
        printf("Usage: %s <serial port number> <TRANSMITTER || RECEIVER>\n", argv[0]);
        exit(1);
    }
    if(strcmp("TRANSMITTER", argv[2]) != 0 && !(argc < 4))
    {
        printf("Usage: %s <serial port number> <TRANSMITTER || RECEIVER> <file name to send>\n", argv[0]);
        exit(1);
    }
    if(strcmp("TRANSMITTER", argv[2]) == 0)
        application.status = TRANSMITTER;
    else
        application.status = RECEIVER;

    application.fileDescriptor = llopen(atoi(argv[1]), application.status);

    if (strcmp("TRANSMITTER", argv[2]) == 0)
    {
        sendFile(argv[3]);
        llclose(application.fileDescriptor);
    }
    else
    {      
        receiveFile();
        llclose(application.fileDescriptor);
    }

    printf("Successfull transmition\n");
    return 0;
}
