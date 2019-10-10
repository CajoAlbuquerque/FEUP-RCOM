/*Non-Canonical Input Processing*/
#include "noncanonical.h"

volatile int STOP=FALSE;

/**
 * 	Writes a supervision frame or a unumbered frame
 * 
 * 	@param fd File descriptor of the open serial port
 * 	@param control Control character value to be written
 * 	
 * 	@return Number of characters written
 */
int write_SUframe(int fd, unsigned char control) {
	unsigned char set[SET_SIZE];
	
	set[F1_INDEX] = FLAG;
	set[A_INDEX] = A;
	set[C_INDEX] = control;
	set[BCC_INDEX] = A ^ control;
	set[F2_INDEX] = FLAG;
	
	int res = write(fd, set, SET_SIZE);
    printf("Sent RR\n");
    
    return res;
}

int llread(int fd, char * buffer) {
	unsigned char current_bcc2, byte[1];
	unsigned int current_index = 0;
	unsigned int data_ok = FALSE;
	unsigned int entered_data = TRUE;
	
	while(!STOP) {
		read(fd, byte, 1);
		int state = openSM(byte[0]);
		
		if(state == DATA_LOOP && entered_data) {
			current_bcc2 = byte[0];
			entered_data = FALSE;
		}
		
		if(state == DATA_LOOP) {
			if(current_bcc2 == byte[0])
				data_ok = TRUE;
			
			current_bcc2 = current_bcc2 ^ byte[0];
			buffer[current_index] = byte[0];
			current_index++;
		}
		else if(state == END) {
			STOP = TRUE;
		}
	}
	
	if(data_ok) {
		if(NS) {
			write_SUframe(fd, RR_1);
			NS = 0;
		}
		else {
			write_SUframe(fd, RR_0);
			NS = 1;
		}
	}
	else {
		if(NS)
			write_SUframe(fd, REJ_1);
		else
			write_SUframe(fd, REJ_0);
	}
		
	
	return current_index;
}

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
    
    //----------------------------------------------FIM DA CONFIGURACAO

/* Estabelicemnto */
    while (!STOP) {       /* loop for input */
      res = read(fd,buf,1);   /* returns after 1 chars have been input */
      openSM(buf[0]);
    }
	sleep(4);
	set[F1_INDEX] = FLAG;
	set[A_INDEX] = A;
	set[C_INDEX] = C_UA;
	set[BCC_INDEX] = A ^ C_UA;
	set[F2_INDEX] = FLAG;

	res = write(fd, set, SET_SIZE);
    printf("Send\n");
	//-----------------------------------------------FIM DO READ OPEN
	unsigned char msg[256];
	
	llread(fd, msg);

    sleep(1);

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
