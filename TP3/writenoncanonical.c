/*Non-Canonical Input Processing*/
#include "writenoncanonical.h"

int fd;
volatile int STOP = FALSE;
volatile int READY = FALSE;
enum state state = Set;

void send_message(){
  char cenas[4] = "yoyo";
  llwrite(fd, cenas, strlen(cenas) + 1);
}

void timeout() { //handler of alarm
  static int timeouts = 1;

  if (timeouts < 3) {
    printf("Resent\n");
    if(state == Set){
      set_transmission();
    } else if(state == Transmit){
      send_message();
    }
    timeouts++;
    alarm(3);
    timeouts = 0;
  } else {
    write(STDERR_FILENO, "Couldnt establish connection.",
          strlen("Couldnt establish connection."));
    exit(-2);
  }
}

void set_transmission() {
  unsigned char set[SET_SIZE];
  int set_res;

  set[F1_INDEX] = FLAG;
  set[A_INDEX] = A;
  set[C_INDEX] = C_SET;
  set[BCC_INDEX] = (A ^ C_SET);
  set[F2_INDEX] = FLAG;

  set_res = write(fd, set, SET_SIZE);
  if (set_res != SET_SIZE) {
    write(STDERR_FILENO, "Set message was not entirely sent.",
          strlen("Set message was not entirely sent."));
    exit(-1);
  }
}

int llwrite(int fd, char *buffer, int length) {
  unsigned char set_write[SET_SIZE + 2 * (length) + 1], set_receive[SET_SIZE],
      read_byte[1], input_byte[1];
  unsigned char bcc2 = 0, c_message;
  int j = 0, state = 0;
  bool STOP_R = FALSE, RR = TRUE;

  set_write[F1_INDEX] = FLAG;
  set_write[A_INDEX] = A;

  if (NS == 0) {
    set_write[C_INDEX] = CONTROL_0;
  } else
    set_write[C_INDEX] = CONTROL_1;

  set_write[BCC_INDEX] = (A ^ C_SET);

  for (unsigned int i = 0; i < length; i++) {
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

  //TODO: fazer um alarme para timeout e reenviar dados
  while (STOP_R == FALSE) {  /* loop for input */
    read(fd, input_byte, 1); /* returns after 5 chars have been input */
    read_byte[0] = input_byte[0];

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
        c_message = RR_0;
        state++;
        NS = 0;
        RR = true;
      } else if (read_byte[0] == RR_1) {
        c_message = RR_1;
        state++;
        NS = 1;
        RR = true;
      } else if (read_byte[0] == REJ_0) {
        c_message = REJ_0;
        state++;
        RR = false;
      } else if (read_byte[0] == REJ_1) {
        c_message = REJ_1;
        state++;
        RR = false;
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
  alarm(0);
  if(!RR) send_message();
  return 1;
}

int main(int argc, char **argv) {
  int res, set_res, state = 0;
  struct termios oldtio, newtio;
  char input[255], output[255], str[255];
  unsigned char set[SET_SIZE], set_reception[SET_SIZE];
  unsigned char byte[1];
  int k = 0;

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

  set_transmission();

  alarm(3);
  while (READY == FALSE) {
    set_res = read(fd, byte, 1);

    if (set_res > 0) {
      alarm(0);
    }

    if (openSM(byte[0], C_UA))
      READY = TRUE;
  }
  state = Transmit;
  send_message();


  printf("Write your words:\n");
  gets(output);
  output[strlen(output)] = 0;

  res = write(fd, output, strlen(output) + 1);
  if (res != strlen(output) + 1) {
    write(STDERR_FILENO, "Word was not entirely sent.",
          strlen("Word was not entirely sent."));
    exit(-1);
  }

  while (STOP == FALSE) {     /* loop for input */
    res = read(fd, input, 1); /* returns after 5 chars have been input */
    str[k] = input[0];
    k++;
    input[res] = 0; /* so we can printf... */
    printf(":%s:%d\n", input, res);
    if (input[0] == '\0')
      STOP = TRUE;
  }

  printf("String:%s", str);

  if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
