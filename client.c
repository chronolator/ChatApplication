#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// Function prototypes
void print_usage();
void error(const char *msg);

int main(int argc, char *argv[]) {

    // Establish variables
    int sockfd, portno, n;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char buf[256];

    if(argc < 3) {
        fprintf(stderr, "usage %s, hostname port\n", argv[0]);
        exit(1);
    }

    // Get port and create socket
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        error("Error opening socket");
    }
    
    // Set the server IP
    server = gethostbyname(argv[1]);
    if(server == NULL) {
        fprintf(stderr, "Error no such IP/host");
    }

    // Zero out server_addr, set to IPv4, copy data from server->h_addr to server_addr.sin_addr.sin_addr
    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy((char *) &server_addr.sin_addr.s_addr, (char *) server->h_addr, server->h_length);
    server_addr.sin_port = htons(portno);
    if(connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error("Connection failed");
    }

    for(;;) {
        memset((char *) buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), stdin);
        n = write(sockfd, buf, strlen(buf));
        if(n < 0) {
            error("Error writing");
        }

        memset((char *) buf, 0, sizeof(buf));
        n = read(sockfd, buf, sizeof(buf));
        if(n < 0) { 
            error("Error reading"); 
        }

        printf("Server: %s\n", buf);
        int i = strncmp("Bye", buf, 3);
        if(i == 0) { break; }
    }
    close(sockfd);

    return EXIT_SUCCESS;
}

void print_usage(char *in) {
    printf("Usage: %s <PORT>\n", in);
    printf("Usage: Will require 'sudo' to run if the executable is not setuid root\n");
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}
