#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>

#define MAX_SIZE 256

int open_socket(int port, char *address)
{
	int sockfd;
	struct sockaddr_in server_addr;

	/*server address handling*/
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(address); /*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);			  /*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket()");
		return -1;
	}
	/*connect to the server*/
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("connect()");
		return -1;
	}

	return sockfd;
}

void clear_buffer(int socket) {
	char buf[MAX_SIZE];
	int bytes;

	while((bytes = read(socket, buf, MAX_SIZE)) > 0){
		printf("%d bytes read\n", bytes);
		continue;
	}
	printf("%d bytes read\n", bytes);
	printf("Buffer cleared\n");
}

int configure_server(int socket, const char *user,const char *pass){
	char cmd[MAX_SIZE];
	int bytes, cat_result;


	cat_result = sprintf(cmd, "user %s", user);
	bytes = write(socket, cmd, cat_result + 1);
	if(bytes < 0){
		perror("writing user");
		return -1;
	}

	// bytes = read(socket, cmd, MAX_SIZE);
	// if(bytes < 0){
	// 	perror("reading user");
	// 	return -1;
	// }
	// cmd[bytes] = '\0';

	// printf("Response : %s\n",cmd);

	cat_result = sprintf(cmd, "pass %s", pass);
	bytes = write(socket, cmd, cat_result + 1);
	if(bytes < 0){
		perror("writing password");
		return -1;
	}

	// bytes = read(socket, cmd, MAX_SIZE);
	// if(bytes < 0){
	// 	perror("reading pass");
	// 	return -1;
	// }
	// cmd[bytes] = '\0';

	// printf("Response : %s\n",cmd);

	clear_buffer(socket);
	cat_result = sprintf(cmd, "pasv");
	bytes = write(socket, cmd, cat_result + 1);
	if(bytes < 0){
		perror("writing pasv");
		return -1;
	}

	bytes = read(socket, cmd, MAX_SIZE);
	if(bytes < 0){
		perror("reading pass");
		return -1;
	}
	cmd[bytes] = '\0';
	printf("Response : %s\n",cmd);

	return 0;
}

int close_socket(int socket) {
	close(socket);
	return 0;
}