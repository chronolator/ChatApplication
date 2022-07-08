#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ncurses.h>

#define BUF_SIZE 256
#define CLIADDR_LEN 100

int sockfd, len, ret, newsockfd;
struct sockaddr_in ser_addr, cli_addr;

void *recMsg(void *sock);
void sig_handler(int signo);

int main(int argc, char *argv[]){ 
    const int PORT = 8000;
    const int REUSE_ENABLE = 1;

    char buffer[BUF_SIZE];
    char clientAddr[CLIADDR_LEN];
    pthread_t rThread;
    //pid_t childpid;
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(PORT);
    ser_addr.sin_addr.s_addr = INADDR_ANY;

    /* NCURSES Programming */
    initscr();
    printw("Hello World\n");
    refresh();
    getch();

    /* NCURSES Programming */
    
    

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


    // Set reuse socket option
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &REUSE_ENABLE, sizeof(REUSE_ENABLE)) < 0) {
        perror("setsockopt");
        exit(1);
    }

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
    printf("Size of cli_addr: %ld\n", sizeof(cli_addr));
    printf("Size of ser_addr: %ld\n", sizeof(ser_addr));
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *) &len);
    //newsockfd = accept(sockfd, NULL, NULL);
    if(newsockfd < 0) {
        printf("Error accepting connection\n");
        exit(1);
    }

    // Convet IPv4 and IPV6 address from binary to text form
    inet_ntop(AF_INET, &(cli_addr.sin_addr), clientAddr, CLIADDR_LEN);
    printf("Connection accepted from %s...\n", clientAddr);
    
    // Create a pthread for receiving messages from the client
    ret = pthread_create(&rThread, NULL, recMsg, (void *) newsockfd);
    if(ret < 0) {
        printf("Error from pthread_create() is %d\n", ret);
        exit(1);
    }

    // Stdin data sent to the client
    printf("SERVER> ");
    while(fgets(buffer, BUF_SIZE, stdin) != NULL) {
        printf("SERVER> ");
        ret = sendto(newsockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &cli_addr, len);
        if(ret < 0) {
            printf("Error sending the data below to the client:\n\t%s\n", buffer);
            exit(1);
        }
    }

    // Close and return everything 
    close(newsockfd);
    close(sockfd);
    pthread_exit(NULL);
    endwin();   /* NCURSES: closes curses mode */
    return EXIT_SUCCESS;
}

// Pthread receiving a message from the client socket
void *recMsg(void *socket) {
    int rret;
    int rsockfd = ((int) socket);
    char rbuffer[BUF_SIZE];

    for(;;) {
        rret = recvfrom(rsockfd, rbuffer, BUF_SIZE, 0, NULL, NULL);

        if(rret < 0) {
            printf("Error receiving data.\n");
        } else {
            if(strncmp(rbuffer, "/exit\0", BUF_SIZE) == 0) {
                printf("Obtained the exit\n");
                close(newsockfd);
                close(sockfd);
                //pthread_exit(NULL);
                exit(1);
            }
            printf("\nclient: %s", rbuffer);
            //printf("client: ");
            //fputs(buffer, stdout);
            //printf("\n");
        }
    }
}

// SIGINT will terminate both sockets
void sig_handler(int signo) {
    if(signo == SIGINT) {
        char rbuffer[BUF_SIZE] = "/exit\0";
        ret = sendto(newsockfd, rbuffer, BUF_SIZE, 0, (struct sockaddr *) &cli_addr, len);
        if(ret < 0) {
            printf("Error sending the data below to the client:\n\t%s\n", rbuffer);
            exit(1);
        }

        close(newsockfd);
        close(sockfd);
        //pthread_exit(NULL);
        printf("Caught sigint\n"); 
        exit(1);
    }
}
