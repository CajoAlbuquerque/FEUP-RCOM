CC = gcc
CFLAGS = -Wall -Wextra -pedantic #-Werror

EXEC = receiver

all: statemachine.o receiver.o transmitter.o alarm.o application.o protocol.o macro.o
	$(CC) $(CFLAGS) receiver.o statemachine.o alarm.o application.o protocol.o macro.o -o $(EXEC)
	$(CC) $(CFLAGS) transmitter.o statemachine.o alarm.o application.o protocol.o macro.o -o transmit

transmitter.o: transmitter/transmitter.c transmitter/transmitter.h
	$(CC) $(CFLAGS) -c transmitter/transmitter.c

receiver.o: receiver/receiver.c receiver/receiver.h
	$(CC) $(CFLAGS) -c receiver/receiver.c

statemachine.o: state_machine/statemachine.c state_machine/statemachine.h
	$(CC) $(CFLAGS) -c state_machine/statemachine.c

alarm.o: alarm/alarm.c alarm/alarm.h
	$(CC) $(CFLAGS) -c alarm/alarm.c

application.o: application/application.c application/application.h
	$(CC) $(CFLAGS) -c application/application.c

protocol.o: protocol/protocol.c protocol/protocol.h
	$(CC) $(CFLAGS) -c protocol/protocol.c

macro.o: macro.c
	$(CC) $(CFLAGS) -c macro.c

clean:
	rm -f $(EXEC)
	rm -f *.o
	rm -f transmit
