#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_SIZE 256

int open_socket(int port, char *address)
{
	int sockfd;
	struct sockaddr_in server_addr;

	/*server address handling*/
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(address); /*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);				  /*server TCP port must be network byte ordered */

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

int parse_ip_port(const char *line, char *ip)
{
	char *raw = strchr(line, '(');
	int length = strlen(raw);
	char values[length];
	char nums[6][4];
	char *token;

	strncpy(values, raw + 1, length - 5);

	token = strtok(values, ",");
	strcpy(nums[0], token);

	for (int i = 1; i < 6; i++)
	{
		token = strtok(NULL, ",");
		strcpy(nums[i], token);
	}

	if (sprintf(ip, "%s.%s.%s.%s", nums[0], nums[1], nums[2], nums[3]) < -1)
	{
		perror("Constructing response IP");
		return -1;
	}

	return atoi(nums[4]) * 256 + atoi(nums[5]);
}

int get_ip_port(int fd, char *ip)
{
	FILE *socket = fdopen(fd, "r");
	char buf[MAX_SIZE], code[4]; //code is the line first 3 digits

	if (socket == NULL)
	{
		perror("Opening file descriptor");
		return -1;
	}

	while (1)
	{
		if (fgets(buf, MAX_SIZE, socket) == NULL)
		{
			perror("Reading response");
			return -1;
		}

		strncpy(code, buf, 3);
		code[3] = '\0';

		if (strcmp(code, "227") == 0)
			break;
	}

	return parse_ip_port(buf, ip);
}

int configure_server(int socket, const char *user, const char *pass, char *ip)
{
	char cmd[MAX_SIZE];
	int bytes, cat_result;

	// Send user command
	cat_result = sprintf(cmd, "user %s\r\n", user);
	bytes = write(socket, cmd, cat_result);
	if (bytes < 0)
	{
		perror("Writing user command");
		return -1;
	}

	// Send pass command
	cat_result = sprintf(cmd, "pass %s\r\n", pass);
	bytes = write(socket, cmd, cat_result);
	if (bytes < 0)
	{
		perror("Writing password command");
		return -1;
	}

	// Send pasv command
	cat_result = sprintf(cmd, "pasv\r\n");
	bytes = write(socket, cmd, cat_result);
	if (bytes < 0)
	{
		perror("Writing pasv command");
		return -1;
	}

	return get_ip_port(socket, ip);
}

int retrieve_file(int socket, char *path, char *filename)
{
	char cmd[MAX_SIZE];
	int bytes, cat_result;

	// Send user comand
	cat_result = sprintf(cmd, "retr %s/%s\r\n", path, filename);
	bytes = write(socket, cmd, cat_result);
	if (bytes < 0)
	{
		perror("Writing retr command");
		return -1;
	}

	return 0;
}

int save_file(int socket, char *filename)
{
	char buf[MAX_SIZE];
	int bytes, filefd = open(filename, O_WRONLY | O_CREAT | O_TRUNC,  0666);

	if (filefd < 0)
	{
		perror("Creating file");
		return -1;
	}

	while ((bytes = read(socket, buf, MAX_SIZE)) > 0)
	{
		if (write(filefd, buf, bytes) < 0)
		{
			perror("Writing to file");
			return -1;
		}
	}

	if (bytes < 0)
	{
		perror("Reading file");
		return -1;
	}

	return 0;
}

int close_socket(int socket)
{
	close(socket);
	return 0;
}