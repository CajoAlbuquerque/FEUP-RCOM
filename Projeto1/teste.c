#include "protocol/protocol.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if(argc < 3){ 
    }
    
    int fd;

    if (atoi(argv[2]) == TRANSMITTER)
    { // 0
        printf("-------------------Started Open Phase \n");
        fd = llopen(atoi(argv[1]), TRANSMITTER);
        if (fd < 0)
        {
            perror("Error on open");
            exit(1);
        }
        printf("-------------------Ended Open Phase \n");
        printf("-------------------Started Data Phase \n");
        sleep(7);
        unsigned char msg[255] = "ola";
        printf("Message is : %s\n", msg);
        printf("msg size is = %d\n", 255);
        if (llwrite(fd, msg, 255) < 0)
        {
            perror("Error on write");
            exit(1);
        }
        sleep(4);
        unsigned char msg2[30] = "ola a t~odos{ Amigos ~{!";
        printf("Message is : %s\n", msg2);
        printf("msg2 size is = %d\n", strlen(msg2) + 1);
        if (llwrite(fd, msg2, strlen(msg2) + 1) < 0)
        {
            perror("Error on write");
            exit(1);
        }
        sleep(3);
        unsigned char msg3[30] = "~{~{~{ Ja chega fds...";
        printf("Message is : %s\n", msg3);
        printf("msg3 size is = %d\n", 30 + 1);
        if (llwrite(fd, msg3, 30 + 1) < 0)
        {
            perror("Error on write");
            exit(1);
        }
        sleep(7);

        printf("-------------------Ended Data Phase \n");
        printf("-------------------Started Close Phase \n");
        if (llclose(fd) < 0)
        {
            perror("Error on close");
            exit(1);
        }
        printf("-------------------Ended Close Phase \n");
    }
    else if (atoi(argv[2]) == RECEIVER)
    {
        printf("-------------------Started Open Phase \n");
        fd = llopen(atoi(argv[1]), RECEIVER);
        if (fd < 0)
        {
            perror("Error on open");
            exit(1);
        }
        printf("-------------------Ended Open Phase \n");
        printf("-------------------Started Data Phase \n");
        unsigned char msg[255];
        while (1)
        {
            int size = llread(fd, msg);
            if (size == -1)
            {
                break;
            }
            else if (size < 0)
            {
                perror("Error on read");
                exit(1);
            }
            else if (size > 0)
            {
                printf("Success! MSG = %s\n", msg);
                printf("msg size is = %d\n", size);
                printf("---------------------\n");
            }
        }
        printf("-------------------Ended Data Phase and Closed \n");
    }

    return 0;
}
