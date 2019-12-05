CC = gcc
CFLAGS = -Wall -Wextra -pedantic #-Werror

EXEC = download

all:  src/download.c
	$(CC) $(CFLAGS) src/download.c -o $(EXEC)

clean:
	rm -f $(EXEC)
	rm -f *.o
