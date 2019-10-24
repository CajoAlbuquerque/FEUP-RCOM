#include "application.h"
#include "../protocol/protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

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
        packet[4+count] = data[count + sequenceNumber *150];
        count++;
    }

    return packet;
}

int receiveDataPacket(int sendSize, unsigned char* data){
	int control;
	int L1, L2, counter;
    static int sequenceNumber = 0;
    int receiveSize = 0, readSize, receiveSequenceNumber;
    unsigned char fileData[155], dataReceive[155];
    
	readSize = llread(application.fileDescriptor, fileData);
    control = fileData[0];
    if(control == 3){
		receivePacket(3);
	} else if(control == 2){
		return -1;
	}
    else if(receiveSize < 0){
        printf("Wrong receive size\n");
        exit(-1);
    }
    receiveSequenceNumber = fileData[1];
    if(receiveSequenceNumber != sequenceNumber){
		printf("Packet not receive\n");
		exit(-1);
	}
    sequenceNumber = (sequenceNumber+1) % 255;
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

int receiveFile(){   
    int fileSize;
    
    fileSize = receivePacket(2);
      
    return 0;
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
    
	llwrite(application.fileDescriptor, &set, sizeof(set));

    return 1;
}

int receivePacket(unsigned int controlE)
{
	unsigned int control;
	unsigned char set[24];
	int sizeoffileSize, sizeoffilename;
	int fileSize; unsigned char filename;
	
	llread(application.fileDescriptor, &set);
	int i = 1;
    control = set[0];
    if(control != controlE){
		printf("Control diferent then the expected\n");
		exit(-1);
	}
	while(set[i] != '\0'){		
		if(set[i] == 0 ){
			sizeoffileSize = set[i + 1];
			fileSize = set[i + 2];
			i = i + 2 + sizeoffileSize;
		
		} else if(set[i] == 1 ){
			sizeoffilename = set[i+1];
			filename = set[i+2];
			i = i + 2 + sizeoffilename;
		} else{
			return -1;
		}
   
	}    
    return fileSize;
	
}

int sendFile(char filename){
    int fileSize, sendSize = 0, sequenceNumber = 0;
    unsigned char *fileData, *dataSend;

    fileData = getCharBuffer( filename, &fileSize);
    controlPacket(2, fileSize, filename);

    while((fileSize - sendSize) >= 150){ //if possible sends 150 bytes of data
        dataSend = dataPacket(150, sequenceNumber, fileData);
        llwrite(application.fileDescriptor, dataSend, 150 + 4);
        free(dataSend);
        sequenceNumber = (sequenceNumber+1) % 255; //sequecial number in modules of 255
    } 
    if((fileSize - sendSize) > 0){ 
        dataSend = dataPacket((fileSize - sendSize), sequenceNumber, fileData);
        llwrite(application.fileDescriptor, dataSend, (fileSize - sendSize));
        free(dataSend);
    } 
    
    controlPacket(3, fileSize, filename);
    return 0;
}


int main(int argc, char **argv){
    if (argc < 3) {
        printf("Usage: %s <serial port number> <TRANSMITTER || RECEIVER> <file name>\n", argv[0]);
        exit(1);
    }

    if((strcmp("TRANSMITTER",argv[2]) != 0) && strcmp("RECEIVER", argv[2]) !=0) {
        printf("Usage: %s <serial port number> <TRANSMITTER || RECEIVER> <file name>\n", argv[0]);
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
    
    printf("Sucessfull transmition\n");
    return 0;
}
