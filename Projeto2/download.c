#include "getip.c"
#include "clientTCP.c"

int main(int argc, char *argv[]) {
    int socket;
    char *ip_address;
    char *user = "anonymous";
    char *pass = "ga_";

    if(argc != 2){
        printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        return -1;
    }
    printf("Nice arguments bro!\n");
    // Coneverting host name to ip address
    ip_address = getip(argv[1]); //TODO: parse argv[1]
    printf("IP = %s\n", ip_address);

    // Opening socket and connection to server
    socket = open_socket(21, ip_address);
    printf("WOW that socket mate! It is %d\n", socket);
    if(socket < 0) {
        return -1;
    }

    if(configure_server(socket, user, pass) < 0) {
        return -1;
    }
    
    printf("It configured CRL!\n");

    close_socket(socket);

    return 0;
}