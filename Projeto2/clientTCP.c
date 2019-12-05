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

int calculate_port(int left, int right) {
	return left * 256 + right;
}

int parse_ip_port(const char * line, char * ip) {
	char * raw = strchr(line, '(');
	int length = strlen(raw);
	char values[length];
	char nums[6][3];
	char *token;
	
	strncpy(values, raw + 1, length - 5);
	printf("Values: %s\n", values);
   
	token = strtok(values, ",");
	strcpy(nums[0], token);

  	for(int i = 1; i < 6; i++) {
		token = strtok(NULL, ",");
		printf("token %d: %s\n", i, token);
		strcpy(nums[i], token);
		printf("NUMS %d: %s\n", i-1, nums[i-1]);
   	}
	
	sprintf(ip, "%s.%s.%s.%s", nums[0], nums[1], nums[2], nums[3]);
	printf("IP: %s\n", ip);

	return calculate_port(atoi(nums[4]), atoi(nums[5]));	
}

int get_ip_port(int fd, char * ip) {
	FILE* socket = fdopen(fd, "r");
	char buf[MAX_SIZE], code[4]; //code is the line first 3 digits

	while(1) {
		fgets(buf, MAX_SIZE, socket);
		strncpy(code, buf, 3);
		code[3] = '\0';

		if(strcmp(code, "227") == 0)
			break;
	}

	return parse_ip_port(buf, ip);
}

int configure_server(int socket, const char *user, const char *pass, char * ip){
	char cmd[MAX_SIZE];
	int bytes, cat_result;

	// Send user comand
	cat_result = sprintf(cmd, "user %s\n", user);
	bytes = write(socket, cmd, cat_result);
	if(bytes < 0){
		perror("writing user");
		return -1;
	}

	// Send pass comand
	cat_result = sprintf(cmd, "pass %s\n", pass);
	bytes = write(socket, cmd, cat_result);
	if(bytes < 0){
		perror("writing password");
		return -1;
	}

	// Send pasv comand
	cat_result = sprintf(cmd, "pasv\n");
	bytes = write(socket, cmd, cat_result);
	if(bytes < 0){
		perror("writing pasv");
		return -1;
	}

	return get_ip_port(socket, ip);
}

int close_socket(int socket) {
	close(socket);
	return 0;
}