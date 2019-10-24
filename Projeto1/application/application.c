#include "application.h"
#include "../protocol/protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

// Functions of the transmitter
unsigned char* getCharBuffer(char filename, int *fileSize){
    FILE *f;
    unsigned char *fileData;
    struct stat fData;
    if((f = fopen(&filename, "rb")) == NULL)
    {
        perror("Error opening the file :");
        exit(-1);
    }
    stat(&filename, &fData);
    (*fileSize) = fData.st_size;

    printf("Get %d bytes form file %d\n", *fileSize, filename );

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
        packet[4+count] = data[count + sequenceNumber * 150];
        count++;
    }

    return packet;
}


int controlPacket(unsigned int control, int fileSize, unsigned char filename){
    if(control != 2 && control != 3){
        printf("control can't be different than 2 and 3");
        exit(-1);
    }

	unsigned char set[24];
	int i = 0;
    
    set[0] = control;
    set[1] = 0;
    set[2] = sizeof(fileSize);
    i = 3 + sizeof(fileSize);
    set[3] = fileSize;
    set[i] = 1;
    set[i+1] = sizeof(filename);
    set[i+2] = filename;
    i = 2 + sizeof(filename);
    set[i] = '\0';
    
	llwrite(application.fileDescriptor, set, sizeof(set));

    return 1;
}

int sendFile(char filename){
    int fileSize, sendSize = 0, sequenceNumber = 0;
    unsigned char *fileData, *dataSend;

    fileData = getCharBuffer(filename, &fileSize);
    controlPacket(2, fileSize, filename);
    printf("Sending control packet with control 2\n");

    while((fileSize - sendSize) >= 150){ //if possible sends 150 bytes of data
        dataSend = dataPacket(150, sequenceNumber, fileData);
        llwrite(application.fileDescriptor, dataSend, 150 + 4);
        sendSize += 150;
        printf("Send 150 bytes of data\n");
        free(dataSend);
        sequenceNumber = (sequenceNumber+1) % 255; //sequencial number in modules of 255
    } 
    if((fileSize - sendSize) > 0){ 
        dataSend = dataPacket((fileSize - sendSize), sequenceNumber, fileData);
        llwrite(application.fileDescriptor, dataSend, (fileSize - sendSize));
        printf("Send last bytes data \n");
        free(dataSend);
    } 
    printf("Sending control packet with control 3\n");
    controlPacket(3, fileSize, filename);
    return 0;
}


int receiveControlPacket2(unsigned char *filename)
{   
	unsigned char set[24];
	int sizeoffileSize, sizeoffilename;
	int fileSize;
	
	llread(application.fileDescriptor, set);
	int i = 1;
    if(atoi(set[0]) != 2){
		printf("Control different then the expected\n");
		exit(-1);
	}
	while(set[i] != '\0'){		
		if(set[i] == 0 ){
			sizeoffileSize = set[i + 1];
			fileSize = set[i + 2];
			i = i + 2 + sizeoffileSize;
		
		} else if(set[i] == 1 ){
			sizeoffilename = set[i+1];
			*filename = set[i+2];
			i = i + 2 + sizeoffilename;
		} else{
			return -1;
		}
   
	}    
    return fileSize;
	
}


int receiveControlPacket3(unsigned char *filename, unsigned char* set)
{   
	int sizeoffileSize, sizeoffilename;
	int fileSize;
	
	int i = 1;
    if(atoi(set[0]) != 3){
		printf("Control different then the expected\n");
		exit(-1);
	}
	while(set[i] != '\0'){		
		if(set[i] == 0 ){
			sizeoffileSize = set[i + 1];
			fileSize = set[i + 2];
			i = i + 2 + sizeoffileSize;
		
		} else if(set[i] == 1 ){
			sizeoffilename = set[i+1];
			*filename = set[i+2];
			i = i + 2 + sizeoffilename;
		} else{
			return -1;
		}
   
	}    
    return fileSize;
	
}

//Functions of the receiver
int receiveDataPacket(FILE *sendFile, unsigned char *filename, int *fileWritten ){ 
	int control;
	int L1, L2, counter;
    static int sequenceNumber = 0;
    int readSize, receiveSequenceNumber;
    unsigned char fileData[155]; //data received

	readSize = llread(application.fileDescriptor, fileData);
    fileWritten += readSize - 4; //control, sequence number and L1 and L2 aren't data
    control = fileData[0];
    if(control == 3){
		receiveControlPacket3(filename, fileData);
	} else if(control == 2){
		return -1;
	} else if(control < 0){
        printf("Wrong control receive\n");
        exit(-1);
    }
    receiveSequenceNumber = fileData[1];
    if(receiveSequenceNumber != sequenceNumber){
		printf("Packet not receive, wrong sequence number\n");
		exit(-1);
	}
    sequenceNumber = (sequenceNumber+1) % 255;
    L2 = fileData[2];
    L1 = fileData[3];
    if(readSize != 256 *L2 + L1 ){
        printf("Wrong Reception\n");
        exit(-1);
    }
    //sends the content to the file, char by char
    while(readSize >= counter){
        const char* data = &fileData[4 + counter];
        fputs(data, sendFile);
        counter++;
    }
    return 0;
}

int receiveFile(){  
    FILE *sendFile;   
    int fileSize, fileWritten;
    unsigned char filename[25];
    fileSize = receiveControlPacket2(filename);
    sendFile = fopen(filename, "a");  
    while (fileWritten < fileSize)
    {
        receiveDataPacket(sendFile, filename, &fileWritten);
    }
    fclose(sendFile);
    return 0;
}

int main(int argc, char **argv){
    if (argc < 2) {
        printf("Usage: %s <serial port number> <TRANSMITTER || RECEIVER> <file name to send>\n", argv[0]);
        exit(1);
    }

    if ((strcmp("TRANSMITTER",argv[2]) != 0) && argc < 3) {
        printf("Usage: %s <serial port number> <TRANSMITTER || RECEIVER>  <file name to send>\n", argv[0]);
        exit(1);
    }

    if((strcmp("TRANSMITTER",argv[2]) != 0) && strcmp("RECEIVER", argv[2]) !=0) {
        printf("Usage: %s <serial port number> <TRANSMITTER || RECEIVER> <file name to send>>\n", argv[0]);
        exit(1);
    }

    application.status = atoi(argv[2]);
    application.fileDescriptor = llopen(atoi(argv[1]), application.status);

    if(strcmp("TRANSMITTER", argv[2]) == 0){
        sendFile(*argv[3]);
        llclose(application.fileDescriptor);
    } else {
        receiveFile();
    }
    
    printf("Successfull transmition\n");
    return 0;
}
