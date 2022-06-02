#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MSG_LEN 2048

void print_usage();
void error(const char *msg);
void str_rm_nl(char *arr, int len);
void recv_handler(void *in);
void send_handler(void *in);

int main(int argc, char *argv[]){

    // Check for arguments
    if(argc < 2) {
        fprintf(stderr, "Please provide a port number\n");
        exit(1);
    }

    // Establish variables
    int sockfd, newsockfd, portno;
    const int bl = 5;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;

    // Create a socket of type IPv4, with two-way connection, and protocol TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        error("Error opening socket");
    }
    int *psockfd = malloc(sizeof(*psockfd));
    *psockfd = sockfd;

    // Zero out struct, convert argv[1] to an int, set the server typei to IPv4, listening range, and port
    memset((char *) &server_addr, 0, sizeof(server_addr));
    portno = atoi(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);

    // Bind to the socket
    if(bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error("Binding Failed");
    }

    // Listen on socket with a backlog(concurrent clients) of 5
    listen(sockfd, bl);
    client_len = sizeof(client_addr);

    // Server header
    getsockname(sockfd, (struct sockaddr*) &server_addr, (socklen_t *) &client_len);
    printf("Server IP: %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    printf("Backlog: %d\n", bl);

    for(;;) {
        // Accept the socket from the client
        newsockfd = accept(sockfd, (struct sockaddr*) &client_addr, &client_len);
        if(newsockfd < 0) {
            error("Error on accept");
        }

        // Print Client IP
        getpeername(newsockfd, (struct sockaddr*) &client_addr, (socklen_t*) &client_len);
        printf("Client from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Thread sending messages
        pthread_t send_thread;
        if(pthread_create(&send_thread, NULL, (void *) send_handler, psockfd) != 0) {
            printf("Failed to create send pthread\n");
            exit(1);
        }

        // Thread receiving messages
        pthread_t recv_thread;
        if(pthread_create(&recv_thread, NULL, (void *) recv_handler, psockfd) != 0) {
            printf("Failed to create recv pthread\n");
            exit(1);
        }

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

void str_rm_nl(char *arr, int len) {
    int i;
    for(i = 0; i < len; i++) {
        if(arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void recv_handler(void *in) {
    int sockfd = *((int *) in);
    char rmsg[MSG_LEN] = {};
    for(;;) {
        int r = recv(sockfd, rmsg, MSG_LEN, 0);
        free(in);
        if(r > 0) {
            printf("Client: %s\n", rmsg);   
        } else {
            break;
        }
    }
}

void send_handler(void *in) {
    int sockfd = *((int *) in);
    char smsg[MSG_LEN] = {};
    for(;;) {
        printf("> ");
        while(fgets(smsg, sizeof(smsg), stdin) != NULL) {
            str_rm_nl(smsg, MSG_LEN);
        }
        send(sockfd, smsg, MSG_LEN, 0);
        free(in);
        if(strcmp(smsg, "exit") == 0) {
            break;
        }
    }
}
