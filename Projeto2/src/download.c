#include "clientTCP.c"
#include "getip.c"
#include <stdio.h>
#include <string.h>

void parseFilename(char *path, char *filename){
 	int beginFilename = 0; // found is a flag
	for(size_t j = strlen(path) - 1; j > 0; j--){ //search in the path for the /
		if(path[j]=='/'){
		   beginFilename = j + 1;
			break;
		}
	}

	int j = 0;
	for(size_t i = beginFilename; i < strlen(path); i++){ //search in the path for the /
		    filename[j] = path[i];
		    j++;
	}
	filename[j] = '\0';		
}


int parseURL(char *argument, char *user, char *pass, char *host, char *path)
{
	char start[] = "ftp://";
	char temp[MAX_SIZE];
	int index = 0;
	int state = 0;
	size_t length = strlen(argument);
	for(size_t i = 0; i < length; i++) { 
		switch (state)
		{
		case 0: //reads the ftp://
			if (argument[i] == start[i] && i < 5)
			{
				break;
			}
			if (i == 5 && argument[i] == start[i])
				state = 1;
			else {
				printf("argument 2 must start with ftp://");
				return -1;
			}
			break;
		case 1: //reads the username
			if (argument[i] == ':')
			{
				temp[index] = '\0';
				strncpy(user, temp, index + 1);	
				state = 2;
				index = 0;
			}
			else if (argument[i] == '/')
			{
				temp[index] = '\0';
				strncpy(host, temp, index + 1);
				state = 4;
				index = 0;
			}
			else
			{
				temp[index] = argument[i];
				index++;
			}
			break;
		case 2:
			if (argument[i] == '@')
			{
				pass[index] = '\0';
				state = 3;
				index = 0;
			}
			else
			{
				pass[index] = argument[i];
				index++;
			}
			break;
		case 3:
			if (argument[i] == '/')
			{
				host[index] = '\0';
				state = 4;
				index = 0;
			}
			else
			{
				host[index] = argument[i];
				index++;
			}
			break;
		case 4:
			path[index] = argument[i];
			index++;
			break;
		}
	}
	
	path[index] = '\0';
	return 0;
}

int main(int argc, char *argv[])
{
    int socket, port, file_socket;
    char *ip_address, pasv_ip[MAX_SIZE];
    char user[MAX_SIZE] = "anonymous";
	char pass[MAX_SIZE] = "ga-";
char host[MAX_SIZE];
    char filename[MAX_SIZE], path[MAX_SIZE];
    char cwd[256];

    if (argc > 4 || argc < 2)
    {
        printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        return -1;
    }
 	
	parseURL(argv[1], user, pass, host, path);
	parseFilename(path, filename);
printf("User : %s \n",user);
	printf("Pass : %s \n",pass);
printf("Host : %s \n",host);
printf("Path : %s \n",path);
printf("Filename : %s \n",filename);

    // Converting host name to ip address
    ip_address = getip(host);
    printf("Server IP address = %s\n", ip_address);

    // Opening socket to server
    socket = open_socket(21, ip_address);
    if (socket < 0)
    {
        return -1;
    }

    // Configuring the server to operate in pasive mode
    printf("Configuring server...\n");
    port = configure_server(socket, user, pass, pasv_ip);
    if (port < 0)
    {
        return -1;
    }

    // Opening the socket to where the file will arrive
    printf("Listening on %s:%d...\n", pasv_ip, port);
    file_socket = open_socket(port, pasv_ip);
    if (file_socket < 0)
    {
        return -1;
    }

    // Asking the server for a file
    printf("Retrieving file...\n");
    if (retrieve_file(socket, path) < 0)
    {
        return -1;
    }

    // Saving the file information
    if(getcwd(cwd, 256) == NULL) {
        perror("getcwd()");
        return -1;
    }

    printf("Saving file to %s/%s...\n", cwd, filename);
    if (save_file(file_socket, filename))
    {
        return -1;
    }

    // Closing sockets/connections
    printf("Closing sockets...\n");
    close_socket(file_socket);
    close_socket(socket);

    return 0;
}


