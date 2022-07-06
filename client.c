#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#define BUF_SIZE 256

void *recMsg(void *socket);
void sig_handler(int signo);

int main(int argc, char *argv[]) {
    const int PORT = 8000;

    int ret;
    int sockfd = (int) socket;
    char buffer[BUF_SIZE];
    char *server_address;
    pthread_t rThread;
    struct sockaddr_in ser_addr, cli_addr;
        ser_addr.sin_family = AF_INET;
        ser_addr.sin_port = htons(PORT);
        ser_addr.sin_addr.s_addr = INADDR_ANY;

    // Args
    if(argc < 2) {
        printf("usage: client < ip address >\n");
        exit(1);
    }
    server_address = argv[1];

    // Catch sigint
    if(signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("Unable to catch SIGINT\n");
        exit(1);
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        printf("Error creating socket\n");
        exit(1);
    }
    printf("Socket created\n");

    // Conenct to socket
    ret = connect(sockfd, (struct sockaddr_in *) &ser_addr, sizeof(ser_addr));
    if(ret < 0) {
        printf("Error connecting to the server\n");
        exit(1);
    }
    printf("Connected to the server\n");

    // Create pthread for receiving messages from server
    printf("CLIENT> ");
    ret = pthread_create(&rThread, NULL, recMsg, (void *) sockfd);
    if(ret < 0) {
        printf("Error from pthread_create() is %d\n", ret);
        exit(1);
    }

    // Stdin data sent to the server
    while(fgets(buffer, BUF_SIZE, stdin) != NULL) {
        ret = sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &ser_addr, sizeof(ser_addr));
        if(ret < 0) {
            printf("Error sending the data below:\n\t%s\n", buffer);
        }
    }
    
    // Close and return everything
    close(sockfd);
    pthread_exit(NULL);
    return EXIT_SUCCESS;
}

void *recMsg(void *socket) {
    int ret;
    int sockfd = (int) socket;
    char buffer[BUF_SIZE];

    for(;;) {
        ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);

        if(ret < 0) {
            printf("Error receiving data\n");   
        } else {
            printf("server: %s\n", buffer);
            //printf("server: "),;
            //fputs(buffer, stdout);
            //printf("\n");
        }
    }
}

void sig_handler(int signo) {
    if(signo == SIGINT) {

        printf("Caught sigint\n");
        exit(1);
    }
}
