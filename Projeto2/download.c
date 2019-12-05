#include "getip.c"
#include "clientTCP.c"

int main(int argc, char *argv[]) {
    int socket, port;
    char *ip_address, pasv_ip[MAX_SIZE];
    char *user = "anonymous";
    char *pass = "ga_";

    if(argc != 2){
        printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        return -1;
    }
    
    // Coneverting host name to ip address
    ip_address = getip(argv[1]); //TODO: parse argv[1]
    printf("IP = %s\n", ip_address);

    // Opening socket and connection to server
    socket = open_socket(21, ip_address);
    printf("WOW that socket mate! It is %d\n", socket);
    if(socket < 0) {
        return -1;
    }

    port = configure_server(socket, user, pass, pasv_ip);
    if( port < 0) {
        return -1;
    }
    printf("Port is: %d\n", port);
    
    printf("It configured CRL!\n");

    close_socket(socket);

    return 0;
}