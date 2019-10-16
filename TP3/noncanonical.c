/*Non-Canonical Input Processing*/
#include "noncanonical.h"

volatile int STOP = FALSE;

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

int llread(int fd, unsigned char *buffer) {
  unsigned char current_bcc2 = 0;
  unsigned char byte[1];
  unsigned int current_index = 0;
  unsigned int data_ok = FALSE, repeated_data = FALSE;
  unsigned int escape_byte = FALSE;

  while (!STOP) {
    if (read(fd, byte, 1) < 0) {
      perror("llread");
      exit(-1);
    }
    int state = readSM(byte[0]);

	if (state == C_RCV){ //Checking for repeated data
		if(byte[0] == CONTROL_0 && NR == 1){
			write_SUframe(fd, RR_1);
		} else if (byte[0] == CONTROL_1 && NR == 0) {
			write_SUframe(fd, RR_0);
		}
    repeated_data = TRUE;
		/*
      When repeated data is sent by the transmitter,
      the receiver ignores all data in the frame.
    */
	} else if (state == DATA_LOOP && !repeated_data) { //Data is being received
      if (current_bcc2 == byte[0])
        data_ok = TRUE;
      else
        data_ok = FALSE;

      current_bcc2 = current_bcc2 ^ byte[0];

      if(byte[0] == ESC) //byte used for stuffing
        escape_byte = TRUE;
      else if(escape_byte == TRUE){ //destuffing the byte
        if(byte[0] == 0x5e)
          buffer[current_index] = 0x7e; //original byte was 0x7e
        else //byte[0] = 0x5d
          buffer[current_index] = 0x7d; //original byte was 0x7d
          escape_byte = FALSE;
        current_index++;
      }
      else {
        buffer[current_index] = byte[0];
        current_index++;
      }
      
    } else if (state == END) {
      STOP = TRUE;
    }
  }

  //When there is repeated data buffer will have no content
  if(repeated_data)
    return 0;

  if (data_ok) {
    if (NR) {
      NR = 0;
      write_SUframe(fd, RR_0);
    } else {
      NR = 1;
      write_SUframe(fd, RR_1);
    }
  } else
    NR ? write_SUframe(fd, REJ_1) : write_SUframe(fd, REJ_0);

  return current_index;
}

int main(int argc, char **argv) {
  int fd;
  struct termios oldtio, newtio;
  char buf[3];
  unsigned char set[SET_SIZE];

  if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
  Open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
  */
  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0) {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;  /* blocking read until 1 chars received */

  /*
  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  leitura do(s) pr�ximo(s) caracter(es)
  */
  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  //----------------------------------------------FIM DA CONFIGURACAO

  /* Estabelicemnto */
  while (!STOP) {
    if (read(fd, buf, 1) == -1) {
      perror("noncanonical read");
      exit(-1);
    }

    if (openSM(buf[0], C_SET))
      STOP = TRUE;
  }

  set[F1_INDEX] = FLAG;
  set[A_INDEX] = A;
  set[C_INDEX] = C_UA;
  set[BCC_INDEX] = A ^ C_UA;
  set[F2_INDEX] = FLAG;

  if (write(fd, set, SET_SIZE) < 0) {
    perror("noncanonical write");
    exit(-1);
  }
  printf("Send\n");
  //-----------------------------------------------FIM DO READ OPEN
  unsigned char msg[256];

  llread(fd, msg);

  sleep(1);

  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
  return 0;
}
