CC = gcc
CFLAGS = -Wall -Wextra -pedantic #-Werror

EXEC = download

all:  download.c
	$(CC) $(CFLAGS) download.c -o $(EXEC)

# clientTCP.o: clientTCP.c
# 	$(CC) $(CFLAGS) -c clientTCP.c

# getip.o: getip.c
# 	$(CC) $(CFLAGS) -c getip.c

# download.o: download.c
# 	$(CC) $(CFLAGS) -c download.c

clean:
	rm -f $(EXEC)
	rm -f *.o
