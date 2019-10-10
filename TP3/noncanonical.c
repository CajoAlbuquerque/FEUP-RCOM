/*Non-Canonical Input Processing*/
#include "noncanonical.h"
#include "statemachine.h"

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,res;
    struct termios oldtio,newtio;
    char buf[3];
	int state = 0;
	unsigned char set[SET_SIZE];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

/* Estabelicemnto */
    while (!STOP) {       /* loop for input */
      res = read(fd,buf,1);   /* returns after 1 chars have been input */ 
    }
	sleep(4);
	set[F1_INDEX] = FLAG;
	set[A_INDEX] = A;
	set[C_INDEX] = C_UA;
	set[BCC_INDEX] = A ^ C_UA;
	set[F2_INDEX] = FLAG;

	res = write(fd, set, SET_SIZE);
    printf("Send\n");

    sleep(1);



    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
