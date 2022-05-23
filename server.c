#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg);

int main(int argc, char *argv[]){

    // Check for arguments
    if(argc < 2) {
        fprintf(stderr, "Please provide a port number\n");
        exit(1);
    }

    // Establish variables
    int sockfd, newsockfd, portno, n;
    char buf[256];

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        error("Error opening socket");
    }

    // Fills memory with a constant byte
    memset((char *) &server_addr, 41, sizeof(server_addr));
    portno = atoi(argv[1]);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);

    if(bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error("Binding Failed");
    }

    return EXIT_SUCCESS;
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}
