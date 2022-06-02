#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>

#define MSG_LEN 2048

// Function prototypes
void print_usage();
void error(const char *msg);
void sig_handler();
void str_rm_nl(char *arr, int len);
void recv_handler(void *in);
void send_handler(void *in);

int main(int argc, char *argv[]) {

    // Catch sigint
    signal(SIGINT, sig_handler);

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        printf("Faield to create a socket\n");
        exit(1);
    }
    int *psockfd = malloc(sizeof(*psockfd));
    *psockfd = sockfd;

    // Socket settings
    struct sockaddr_in server, client;
    int s_len = sizeof(server);
    int c_len = sizeof(client);
    memset(&server, 0, sizeof(s_len));
    memset(&client, 0, sizeof(c_len));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(1337);

    // Connect to socket
    int err = connect(sockfd, (struct sockaddr *) &server, s_len);
    if(err == -1) {
        printf("Failed to connect to socket\n");
        exit(1);
    }

    // Header info
    getsockname(sockfd, (struct sockaddr *) &client, (socklen_t *) &c_len);
    getpeername(sockfd, (struct sockaddr *) &server, (socklen_t *) &s_len);
    printf("Connected to server: %s:%d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));
    printf("You are: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

    // Thread sending messages
    pthread_t send_thread;
    if(pthread_create(&send_thread, NULL, (void *) send_handler, psockfd) != 0) {
        printf("Failed to create pthread\n");
        exit(1);
    }

    // Thread receiving messages
    pthread_t recv_thread;
    if(pthread_create(&recv_thread, NULL, (void *) recv_handler, psockfd) != 0) {
        printf("Failed to create pthread\n");
        exit(1);
    }

    // Keep socket open
    for(;;) {
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

void sig_handler(int sockfd) {
    close(sockfd);
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
        if(r > 0) {
            printf("Server: %s\n", rmsg);
        }
        else {
            free(in);
            break;
        }
    }
}

void send_handler(void *in) {
    int sockfd = *((int *) in);
    char smsg[MSG_LEN] = {};
    printf(">>> ");
    for(;;) {
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
