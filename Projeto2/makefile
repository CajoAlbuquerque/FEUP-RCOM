CC = gcc
CFLAGS = -Wall -Wextra -pedantic #-Werror

EXEC = download

all:  download.c
	$(CC) $(CFLAGS) download.c -o $(EXEC)

clean:
	rm -f $(EXEC)
	rm -f *.o
