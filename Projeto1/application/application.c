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

int sendDataPacket(int sendSize, int sequenceNumber, unsigned char *data, unsigned char *packet)
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
        count++;
    }

    return 0;
}

int sendControlPacket(unsigned int control, int fileSize, char *filename)
{
    char sizeString[16];
	sprintf(sizeString, "%d", fileSize);

	int size = 5 + strlen(sizeString) + strlen(filename);
    printf("size: %d", size);
	unsigned char ctrlPckg[size];

	ctrlPckg[0] = control + '0';
	ctrlPckg[1] = 0 + '0';
	ctrlPckg[2] = strlen(sizeString) + '0';

	size_t i, j = 3;
	for(i = 0; i < strlen(sizeString); i++) {
		ctrlPckg[j] = sizeString[i];
		j++;;
	}

	ctrlPckg[j] = 1 + '0';
	j++;
	ctrlPckg[j] = strlen(filename) + '0';
	j++;

	for(i = 0; i < strlen(filename); i++) {
		ctrlPckg[j] = filename[i];
		j++;
	}

	if ( llwrite(application.fileDescriptor, ctrlPckg, size) < 0) {
		printf("ERROR in sendCtrlPkt(): llwrite() function error!\n");
		return -1;
	}

	return 0;
 
}

int sendFile(char *filename)
{
    int fileSize;
    int sendSize = 0, sequenceNumber = 0;
    unsigned char *fileData, *dataSend;

    dataSend = (unsigned char *)malloc(TRANSMIT_SIZE * sizeof(char));

    fileData = getCharBuffer(filename, &fileSize);

    sendControlPacket(2, fileSize, filename);

    printf("Sended control packet with control 2\n");

    while ((fileSize - sendSize) >= TRANSMIT_SIZE)
    { //if possible sends TRANSMIT_SIZE bytes of data
       
        sendSize += TRANSMIT_SIZE; //each time only TRANSMIT_SIZE are really data
        sendDataPacket(TRANSMIT_SIZE, sequenceNumber, fileData, dataSend );
        sequenceNumber = (sequenceNumber + 1) % 255; //sequencial number in modules of 255

        llwrite(application.fileDescriptor, dataSend, TRANSMIT_SIZE + 4);      

    }
    if ((fileSize - sendSize) > 0)
    {
        sendDataPacket((fileSize - sendSize) , sequenceNumber, fileData, dataSend);
        llwrite(application.fileDescriptor, dataSend, ((fileSize - sendSize)+4));
        sendSize += (fileSize - sendSize);
        
    }
    printf("sendSize: %d\n", sendSize);

    printf("Sending control packet with control 3\n");
    sendControlPacket(3, fileSize, filename);

    free(dataSend);
    return 0;
}

int receiveControlPacket(int control,unsigned char *filename)
{
    unsigned char * controlPac;
    int fileSize = 0;

    controlPac = (unsigned char *)malloc(70 *sizeof(unsigned char));

	if (llread(application.fileDescriptor, controlPac) < 0) {
		printf("ERROR in rcvCtrlPkt(): \n");
		return -1;
	}
	
	if ((controlPac[0] - '0') != control) {
		printf("ERROR in rcvCtrlPkt(): unexpected control field!\n");
		return -2;
	}

	if ((controlPac[1] - '0') != 0) {
		printf("ERROR in rcvCtrlPkt(): unexpected size param!\n");
		return -3;
	}

	int i, fileSizeLength = (controlPac[2] - '0'), j = 3;

	char fileSizeStr[25];

	for(i = 0; i < fileSizeLength; i++) {
		fileSizeStr[i] = controlPac[j];
		j++;
	}

	fileSizeStr[j - 3] = '\0';

	fileSize = atoi(fileSizeStr);

	if((controlPac[j] - '0') != 1) {
		printf("ERROR in rcvCtrlPkt(): unexpected name param!\n");
		return -4;
	}

	j++;	
	int pathLength = (controlPac[j] - '0');
	j++;

	char pathStr[30];
	
	for(i = 0; i < pathLength; i++) {
		pathStr[i] = controlPac[j];
		j++;
	}

	pathStr[i] = '\0';

    if (3 == control && (strcmp(filename,pathStr) != 0)) {
        //compare with the other filename
        printf("Name of received file can be wrong!\n");
    }
    else{
        strcpy(filename, pathStr);
    }

    free(controlPac);
	return fileSize;

}

//Functions of the receiver
int receiveDataPacket(FILE *sendFile, int *fileWritten)
{
    int control;
    int L1, L2;
    static int sequenceNumber = 0;
    int readSize, receiveSequenceNumber;
    unsigned char fileData[TRANSMIT_SIZE +4];

    readSize = llread(application.fileDescriptor, fileData);

    control = fileData[0];

    if ((fileData[0] - '0') == 3)
    {
        printf("Receive control Packet 3 to soon \n");
        return -1;
    } 
    else if (control == 2)
    {
        //receive control 2 twice, ignoring
        return -1;
    }
    else if (control < 0)
    {
        printf("Wrong control receive\n");
        exit(-1);
    }

    receiveSequenceNumber = fileData[1];
    printf("Sequencial Number %d\n", sequenceNumber);
    if (receiveSequenceNumber != sequenceNumber)
    {
        printf("Packet not receive, wrong sequence number\n");
        return -1;
    } 

    L2 = fileData[2];
    L1 = fileData[3];

    if ((readSize-4)  != ((256 * L2) + L1))
    {
        printf("Wrong Reception, L1 and L2 don't mach readSize \n");
        exit(-1);
        //return -1;
    }

    //if everything went well, and only then, change values
    sequenceNumber = (sequenceNumber + 1) % 255;
    *fileWritten += (readSize - 4); //control, sequence number and L1 and L2 aren't data
    
    //puts char by char
    for(int i =0 ; i < (readSize-4); i++)
    {
        putc(fileData[4 + i], sendFile);
    } 

    return 0;
}

int receiveFile()
{
    FILE *sendFile;
    int fileSize, fileWritten = 0;
    unsigned char *filename;
    filename = (unsigned char *)malloc(25*sizeof(unsigned char));

    fileSize = receiveControlPacket(2, filename); 

    //in the same pc has to exist !!!
    filename = "catPic.jpg";

    sendFile = fopen(filename, "a");
    if(sendFile == NULL){
        printf("couldn't open file\n");
        exit(-1);
    }
    while (fileWritten < fileSize)
    {
        receiveDataPacket(sendFile, &fileWritten);     
    }
    printf("file Written: %d\n", fileWritten);
    printf("Receiving control packet with control 3\n");
    receiveControlPacket(3, filename);
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
