#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void print_usage(char *in);

int main(int argc, char *argv[]) {

    if(argc < 3 || argc > 3) {
        print_usage(argv[0]);
        exit(1);
    }

    const char *IP = argv[1];
    const int PORT = atoi(argv[2]);
    const char smsg[64] = "This is a message from the server LOL";

    if(PORT == 0) {
        printf("Port value must be a numerical value\n\n");
        print_usage(argv[0]);
        exit(1);
    }

    printf("IP: %s\n", IP);
    printf("PORT: %d\n", PORT);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ssock;
        ssock.sin_family = AF_INET;
        ssock.sin_port = htons(PORT);
        ssock.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*) &ssock, sizeof(ssock));

    for(;;) {
        listen(sockfd, 5);
        int csock = accept(sockfd, NULL, NULL);
        if(csock != -1) {
            printf("Client has connected with a return value of %d\n", csock);
        }
        send(csock, smsg, sizeof(smsg), 0);
        close(sockfd);
    }

    return EXIT_SUCCESS;
}

void print_usage(char *in) {
    printf("Usage: %s <IP> <PORT>\n", in);
    printf("Usage: Will require 'sudo' to run if the executable is not setuid root]n");
    exit(1);
}