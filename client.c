#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Function prototypes
void print_usage();

int main(int argc, char *argv[]) {

    // Get exactly two commandline arguments
    if(argc < 3 || argc > 3) {
        print_usage(argv[0]);
        exit(1);
    }

    // Start of socket
    // TODO: Make IP not a string and make IP a const
    const char *IP = argv[1];
    const int PORT = atoi(argv[2]);

    if(PORT == 0) {
        printf("Port value must be a numerical value\n\n");
        print_usage(argv[0]);
        exit(1);
    }

    printf("IP: %s \n", IP);
    printf("PORT: %d\n", PORT);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ssock;
        ssock.sin_family = AF_INET;
        ssock.sin_port = htons(PORT);
        ssock.sin_addr.s_addr = INADDR_ANY;


    for(;;) {
        int sock_state = connect(sockfd, (struct sockaddr *) &ssock, sizeof(ssock));
        if(sock_state != 0 ) {
            printf("There was an error with making a connection to the server\n");
            exit(1);
        }

        char ssend[1024] = "";
        recv(sockfd, &ssend, sizeof(ssend), 0);
        printf("Server Response: %s\n", ssend);
        
        close(sockfd);
    }



    return EXIT_SUCCESS;
}

// Function for print usage
void print_usage(char *in) {
    printf("Usage: %s <IP> <PORT>\n", in);
    printf("Usage: Will require 'sudo' to run if the executable is not setuid root\n");
}
