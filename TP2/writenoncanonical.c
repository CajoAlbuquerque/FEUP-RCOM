/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07
#define F1_INDEX 0
#define A_INDEX 1
#define C_INDEX 2
#define BCC_INDEX 3
#define F2_INDEX 4
#define SET_SIZE 5


int fd;
volatile int STOP=FALSE;
volatile int READY = FALSE;


void timeout() {
  static int timeouts = 1;

  if (timeouts < 3) {
    printf("Resent\n");
    set_transmission();
    timeouts++;
    alarm(3);
  }
  else  {
    write(STDERR_FILENO, "Couldnt establish connection.", strlen("Couldnt establish connection."));
    exit(-2);
  }

}

void set_transmission() {
  unsigned char set[SET_SIZE];
  int set_res;

  set[F1_INDEX] = FLAG;
  set[A_INDEX] = A;
  set[C_INDEX] = C_SET;
  set[BCC_INDEX] = (A^C_SET);
  set[F2_INDEX] = FLAG;

  set_res = write(fd, set, SET_SIZE);
  if(set_res != SET_SIZE){
    write(STDERR_FILENO, "Set message was not entirely sent.", strlen("Set message was not entirely sent."));
    exit(-1);
  }

}

int main(int argc, char** argv)
{
    int res, set_res, state = 0;
    struct termios oldtio,newtio;
    char input[255], output[255], str[255];
    unsigned char set[SET_SIZE], set_reception[SET_SIZE];
    unsigned char byte[1];
    int i = 0;

    (void) signal(SIGALRM, timeout);

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage: SerialPort\n\tex: /dev/ttyS1\n");
      exit(1);
    }

    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd < 0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio)); /*memory allocation with zeros*/
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

    tcflush(fd, TCIOFLUSH);



    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    printf("Set is set!\n");

    set_transmission();

    alarm(3);
    while (READY == FALSE) {
      set_res = read(fd, byte,1);

      if(set_res > 0) {
        alarm(0);
      }

      printf("%d\n", set_res);

      switch(state) {
        case 0:
          if (byte[0] == FLAG) {
            state++;
          }
          break;
        case 1:
          if (byte[0] == A) {
            state++;
          }
          break;
        case 2:
          if (byte[0] == C_UA) {
            state++;
          }
          break;
        case 3:
          if (byte[0] == (A^C_UA)) {
            state++;
          }
          break;
        case 4:
        if (byte[0] == FLAG) {
          READY = TRUE;
          printf("Message received\n");
        }
      }

    }


    printf("Write your words:\n");
    gets(output);
    output[strlen(output)] = 0;




    res = write(fd, output, strlen(output)+1);
    if(res != strlen(output) + 1){
      write(STDERR_FILENO, "Word was not entirely sent.", strlen("Word was not entirely sent."));
      exit(-1);
    }

    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,input,1);   /* returns after 5 chars have been input */
      str[i] = input[0];
      i++;
      input[res]=0;               /* so we can printf... */
      printf(":%s:%d\n", input, res);
      if (input[0]=='\0')
        STOP=TRUE;
    }

    printf("String:%s", str);

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}
