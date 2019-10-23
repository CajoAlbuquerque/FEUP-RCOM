/*Non-Canonical Input Processing*/
#include "writenoncanonical.h"

int fd;
volatile int STOP = FALSE;
volatile int READY = FALSE;
phase_t phase = Set;
int timeouts = 0;

void send_message(){
  char cenas[6] = "yo}}yo";
  llwrite(fd, cenas, strlen(cenas));
}

void timeout() { //handler of alarm
  timeouts++;
  if (timeouts < 3) {
    printf("Resent\n");
    if(phase == Set){
      send_frameSU(C_SET); //resend set_transmission
    } else if(phase == Transmit){
      send_message();
    } else {
      send_frameSU(C_DISC);
    }
    alarm(3);
  } else {
    write(STDERR_FILENO, "Couldnt establish connection.",
          strlen("Couldnt establish connection."));
    exit(-2);
  }
}

void send_frameSU(unsigned char control) {
  unsigned char set[SET_SIZE];
  int set_res;

  set[F1_INDEX] = FLAG;
  set[A_INDEX] = A;
  set[C_INDEX] = control;
  set[BCC_INDEX] = (A ^ control);
  set[F2_INDEX] = FLAG;

  set_res = write(fd, set, SET_SIZE);
  if (set_res != SET_SIZE) {
    write(STDERR_FILENO, "Set message was not entirely sent.",
          strlen("Set message was not entirely sent."));
    exit(-1);
  }
  alarm(3); //for time out
}

int llwrite(int fd, char *buffer, int length) {
  unsigned char set_write[SET_SIZE + 2 * (length) + 1];
  unsigned char bcc2 = 0;
  int j = 0;

  set_write[F1_INDEX] = FLAG;
  set_write[A_INDEX] = A;

  unsigned char control;
  if (NS == 0) {
    control = CONTROL_0;
  } else
    control = CONTROL_1;

  set_write[C_INDEX] = control;
  set_write[BCC_INDEX] = (A ^ control);

  for (int i = 0; i < length; i++) {
    // Byte stuffing
    if (buffer[i] == ESC) {
      set_write[BCC_INDEX + i + 1 + j] = ESC;
      j++;
      set_write[BCC_INDEX + i + 1 + j] = ESC_SOL; // ESC_SOL = ESC ^ 0x20
    }
    else if (buffer[i] == FLAG) {
      set_write[BCC_INDEX + i + 1 + j] = ESC;
      j++;
      set_write[BCC_INDEX + i + 1 + j] = FLAG_SOL; // FLAG_SOL = FLAG ^ 0x20
    }
    else {
      set_write[BCC_INDEX + i + 1 + j] = buffer[i];
    }

    /*
      Since bcc2 starts at 0, bcc2 ^ buffer[0] = buffer[0].
      This way we avoid an irrelevant condition.
      Additionally, as buffer remains unchanged, bcc2 wil not be affected by stuffing
    */
    bcc2 = bcc2 ^ buffer[i];
  }

  set_write[BCC_INDEX + length + 1 + j] = bcc2;
  set_write[BCC_INDEX + length + 2 + j] = FLAG;

  write(fd, set_write, SET_SIZE + length + j + 1);
  alarm(3);
  return 0;
}

void  receive_confimation() {
  bool STOP_R = FALSE, resend = false;
  unsigned char c_message;
  unsigned char read_byte[1], input_byte[1];
  int state = 0;

  while (STOP_R == FALSE) {  /* loop for input */
    read(fd, input_byte, 1); /* returns after 5 chars have been input */
    read_byte[0] = input_byte[0];

    printf("STATE %d\n", state);

    switch (state) {
    case 0:
      if (read_byte[0] == FLAG) {
        state++;
      }
      break;
    case 1:
      if (read_byte[0] == A) {
        state++;
      }
      break;
    case 2:
      if (read_byte[0] == RR_0) {
        printf("RR0\n");
        c_message = RR_0;
        state++;
        NS = 0;
        resend = FALSE;
      } else if (read_byte[0] == RR_1) {
        printf("RR1\n");
        c_message = RR_1;
        state++;
        NS = 1;
        resend = FALSE;
      } else if (read_byte[0] == REJ_0) {
        printf("REJ0\n");
        c_message = REJ_0;
        state++;
        resend = TRUE;
      } else if (read_byte[0] == REJ_1) {
        printf("REJ1\n");
        c_message = REJ_1;
        state++;
        resend = TRUE;
      } else if (read_byte[0] == FLAG) {
        state = 1;
      }
      break;
    case 3:
      if (read_byte[0] == (A ^ c_message)) {
        state++;
      } else if (read_byte[0] == FLAG) {
        state = 1;
      }
      break;
    case 4:
      if (read_byte[0] == FLAG) {
        state++;
        STOP_R = TRUE;
      }
      break;
    default:
      break;
    }

  }
  printf("END\n");
  alarm(0);

  if(resend){
    printf("END\n");
    send_message();
  }

}

int receive_frameSU(unsigned char control){
  int set_res;
  unsigned char byte[1];
  unsigned char READY = FALSE;
  //waits for the frame DISC
  while (READY == FALSE) {
    set_res = read(fd, byte, 1);

    if (set_res > 0)
      alarm(0);


    if (openSM(byte[0], control))
      READY = TRUE;
  }
  return 0;
}

int main(int argc, char **argv) {
  struct termios oldtio, newtio;

  (void)signal(SIGALRM, timeout);

  if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage: SerialPort\n\tex: /dev/ttyS1\n");
    exit(1);
  }

  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0) {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio)); /*memory allocation with zeros*/
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 5;  /* blocking read until 5 chars received */

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  printf("Set is set!\n");

  send_frameSU(C_SET);
  printf("Send SET\n");

  receive_frameSU(C_UA);
  printf("Receive UA\n");

  timeouts = 0; //resent because is possible that were timeouts in set_transmission
  phase = Transmit;
  send_message();
  receive_confimation();

  phase = End;
  timeouts = 0;
  send_frameSU(C_DISC); //Send Disconect
  printf("Send C_DISC\n");

  receive_frameSU(C_DISC);
  printf("Receive DISC\n");

  //if receive C_DISC send UA
  send_frameSU(C_UA);
  printf("Send C_UA \n" );

  // printf("Write your words:\n");
  // gets(output);
  // output[strlen(output)] = 0;
  //
  // res = write(fd, output, strlen(output) + 1);
  // if (res != strlen(output) + 1) {
  //   write(STDERR_FILENO, "Word was not entirely sent.",
  //         strlen("Word was not entirely sent."));
  //   exit(-1);
  // }

  // while (STOP == FALSE) {     /* loop for input */
  //   res = read(fd, input, 1); /* returns after 5 chars have been input */
  //   str[k] = input[0];
  //   k++;
  //   input[res] = 0; /* so we can printf... */
  //   printf(":%s:%d\n", input, res);
  //   if (input[0] == '\0')
  //     STOP = TRUE;
  // }

  if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
