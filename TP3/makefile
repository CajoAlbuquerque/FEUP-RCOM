CC = gcc
CFLAGS = -Wall -Wextra -pedantic #-Werror

EXEC = nserial

all: statemachine.o noncanonical.o writenoncanonical.o
	$(CC) $(CFLAGS) noncanonical.o statemachine.o -o $(EXEC)
	$(CC) $(CFLAGS) writenoncanonical.o statemachine.o -o wnserial

writenoncanonical.o: writenoncanonical.c writenoncanonical.h
	$(CC) $(CFLAGS) -c writenoncanonical.c

noncanonical.o: noncanonical.c noncanonical.h
	$(CC) $(CFLAGS) -c noncanonical.c

statemachine.o: statemachine.c statemachine.h
	$(CC) $(CFLAGS) -c statemachine.c

clean:
	rm -f $(EXEC)
	rm -f *.o
	rm -f wnserial
