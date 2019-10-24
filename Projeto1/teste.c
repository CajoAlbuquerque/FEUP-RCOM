#include "protocol/protocol.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int fd;

    if(argv[2] == TRANSMITTER){ // 0
        fd = llopen(atoi(argv[1]), TRANSMITTER);
        if(fd < 0){
            printf("Error on open\n");
            exit(1);
        }
        unsigned char msg[25] = "ola Cajo, Helena e Joca!";
        if(llwrite(fd, msg, strlen(msg)) < 0){
            printf("Error on write\n");
            exit(1);
        }

        if(llclose(fd) < 0){
            printf("Error on close\n");
            exit(1);
        }
    }
    else
    {
        fd = llopen(atoi(argv[1]), RECEIVER);
        if(fd < 0){
            printf("Error on open\n");
            exit(1);
        }
        unsigned char msg[255];
        if(llread(fd, msg) < 0) {
            printf("Error on read\n");
            exit(1);
        }

        printf("Success MSG = %s\n", msg);
    }
    

    return 0;
}