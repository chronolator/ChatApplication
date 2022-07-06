#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define BUF_SIZE 256
#define CLIADDR_LEN 100

int gsockfd, gnewsockfd;

void *recMsg(void *sock);
void sig_handler(int signo, int server, int client);

int main(int argc, char *argv[]){ 
    const int PORT = 8000;

    int sockfd, len, ret, newsockfd;
    char buffer[BUF_SIZE];
    char clientAddr[CLIADDR_LEN];
    pthread_t rThread;
    pid_t childpid;
    struct sockaddr_in ser_addr, cli_addr;
        ser_addr.sin_family = AF_INET;
        ser_addr.sin_port = htons(PORT);
        ser_addr.sin_addr.s_addr = INADDR_ANY;
        

    // Catch sigint
    if(signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("Unable to catch SIGINT\n");
        exit(1);
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    gsockfd = sockfd;
    if(sockfd < 0) {
        printf("Error creating socket\n");
        exit(1);
    }
    printf("Socket created\n");

    // Bind to socket
    ret = bind(sockfd, (struct sockaddr *) &ser_addr, sizeof(ser_addr));
    if(ret < 0) {
        printf("Error binding to socket\n");
        exit(1);
    }
    printf("Bound to socket\n");

    // Socket is listening
    printf("\nListenting for any incoming clients\n");
    listen(sockfd, 5);

    // Accept incoming connections
    len = sizeof(cli_addr);
    printf("Size of cli_addr: %d\n", cli_addr);
    printf("Size of ser_addr: %d\n", ser_addr);
    newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &len);
    gnewsockfd = newsockfd;
    if(newsockfd < 0) {
        printf("Error accepting connection\n");
        exit(1);
    }

    // Convet IPv4 and IPV6 address from binary to text form
    inet_ntop(AF_INET, &(cli_addr.sin_addr), clientAddr, CLIADDR_LEN);
    printf("Connection accepted from %s...\n", clientAddr);
    
    // Create a pthread for receiving messages from the client
    printf("SERVER> ");
    ret = pthread_create(&rThread, NULL, recMsg, (void *) newsockfd);
    if(ret < 0) {
        printf("Error from pthread_create() is %d\n", ret);
        exit(1);
    }

    // Stdin data sent to the client
    while(fgets(buffer, BUF_SIZE, stdin) != NULL) {
        ret = sendto(newsockfd, buffer, BUF_SIZE, 0, (struct sockaddr*) &cli_addr, len);
        if(ret < 0) {
            printf("Error sending the data below to the client:\n\t%s\n", buffer);
            exit(1);
        }
    }

    // Close and return everything 
    close(newsockfd);
    close(sockfd);
    pthread_exit(NULL);
    return EXIT_SUCCESS;
}

void *recMsg(void *socket) {
    int ret;
    int sockfd = ((int) socket);
    char buffer[BUF_SIZE];
    //memset(buffer, 0, BUF_SIZE);

    for(;;) {
        ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);

        if(ret < 0) {
            printf("Error receiving data.\n");
        } else {
            printf("client: %s\n", buffer);
            //printf("client: ");
            //fputs(buffer, stdout);
            //printf("\n");
        }
    }
}

void sig_handler(int signo) {
    if(signo == SIGINT) {
        // Kill the client program somehow here
        close(gnewsockfd);
        close(gsockfd);
        printf("Caught sigint\n"); 
        exit(1);
    }
}
