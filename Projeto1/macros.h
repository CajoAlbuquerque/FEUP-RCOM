#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

// Supervision and Unumbered (SU) frames structure
#define F1_INDEX 0
#define A_INDEX 1
#define C_INDEX 2
#define BCC_INDEX 3
#define F2_INDEX 4
#define SET_FRAME_SIZE 5

// Flag and A characters values
#define FLAG 0x7e
#define A 0x03

// Control character values for SU frames
#define C_SET 0x03
#define C_UA 0x07
#define RR_0 0x05
#define RR_1 0x85
#define REJ_0 0x01
#define REJ_1 0x81

// Control character values for Data frames
#define CONTROL_0 0x00
#define CONTROL_1 0x40

// Stuffing bytes
#define ESC 0x7d
#define ESC_SOL 0x5d

// Timeout related values
#define TIMEOUT_INTERVAL 3
#define TIMEOUT_MAX_ATTEMPTS 3