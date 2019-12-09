#include "clientTCP.c"
#include "getip.c"

int main(int argc, char *argv[])
{
    int socket, port, file_socket;
    char *ip_address, pasv_ip[MAX_SIZE];
    char *user = "anonymous";
    char *pass = "ga_";
    char *filename, *path;
    char cwd[256];

    if (argc > 4)
    {
        printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        return -1;
    }
    
    path = argv[2];
    filename = argv[3]; // TODO: change argv[2] to actual filename pased from argv[1]

    // Converting host name to ip address
    ip_address = getip(argv[1]); //TODO: parse argv[1]
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
    if (retrieve_file(socket, path, filename) < 0)
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