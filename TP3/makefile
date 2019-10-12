CC = gcc
CFLAGS = -Wall -Wextra -pedantic -Werror

all: statemachine.o noncanonical.o
	$(CC) $(CFLAGS) noncanonical.o statemachine.o -o nc

noncanonical.o: noncanonical.c noncanonical.h
	$(CC) $(CFLAGS) -c noncanonical.c

statemachine.o: statemachine.c statemachine.h
	$(CC) $(CFLAGS) -c statemachine.c

clean:
	rm -f nc
	rm -f *.o