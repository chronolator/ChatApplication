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
#include <ncurses.h>

#define BUF_SIZE 256

// Globals
int sockfd, ret;
struct sockaddr_in ser_addr, cli_addr;

/* NCURSES Globals */
WINDOW *chatlog_border_win;
WINDOW *chat_border_win;
WINDOW *chatlog_win;
WINDOW *chat_win;

// Functions
void *recMsg(void *socket);
void sig_handler(int signo);

/* NCURSES Functions */
WINDOW *create_newwin(int height, int width, int starty, int startx);

int main(int argc, char *argv[]) {
    const int PORT = 8000;

    char buffer[BUF_SIZE];
    char *server_address;
    pthread_t rThread;
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(PORT);
    ser_addr.sin_addr.s_addr = INADDR_ANY;

    // Args
    if(argc < 2) {
        printf("usage: client <IP ADDRESS>\n");
        exit(1);
    }
    server_address = argv[1];


    /* NCURSES Format Chatlog Border Windows */
    chatlog_border_win = create_newwin(LINES-4, COLS-1, 0, 0);
    box(chatlog_border_win, 0, 0);
    chat_border_win = create_newwin(3, COLS-1, LINES-4, 0);
    box(chat_border_win, 0, 0);
    wrefresh(chatlog_border_win);
    wrefresh(chat_border_win);
    
    /* NCURSES Format Chatlog Windows */
    chatlog_win = create_newwin(LINES-6, COLS-3, 1, 1);
    chat_win = create_newwin(1, COLS-3, LINES-3, 1);
    wrefresh(chatlog_win);
    wrefresh(chat_win);

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
    ret = connect(sockfd, (struct sockaddr *) &ser_addr, sizeof(ser_addr));
    if(ret < 0) {
        printf("Error connecting to the server\n");
        exit(1);
    }
    printf("Connected to the server\n");

    // Create pthread for receiving messages from server
    ret = pthread_create(&rThread, NULL, recMsg, (void *) sockfd);
    if(ret < 0) {
        printf("Error from pthread_create() is %d\n", ret);
        exit(1);
    }

    // Stdin data sent to the server
    printf("CLIENT> ");
    while(fgets(buffer, BUF_SIZE, stdin) != NULL) {
        printf("CLIENT> ");
        ret = sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &ser_addr, sizeof(ser_addr));
        if(ret < 0) {
            printf("Error sending the data below:\n\t%s\n", buffer);
            exit(1);
        }
    }
    
    // Close and return everything
    close(sockfd);
    pthread_exit(NULL);
    return EXIT_SUCCESS;
}

// Pthread receiving a message from the server socket
void *recMsg(void *socket) {
    bool init = true;
    int rret;
    int rsockfd = ((int) socket);
    char rbuffer[BUF_SIZE];

    for(;;) {
        rret = recvfrom(rsockfd, rbuffer, BUF_SIZE, 0, NULL, NULL);

        if(rret < 0) {
            printf("Error receiving data\n");   
        } else {
            if(init) {
                printf("\nserver: %s", rbuffer);
                init = false;
            }
            if(strncmp(rbuffer, "/exit\0", BUF_SIZE) == 0) {
                printf("Obtained the exit\n");
                close(sockfd);
                //pthread_exit(NULL);
                exit(1);
            }
            //if(strncmp(rbuffer, '\0', BUF_SIZE) == 0) {
            //    exit(1);
            //}
            printf("server: %s\n", rbuffer);
            //printf("server: "),;
            //fputs(buffer, stdout);
            //printf("\n");
        }
    }
}

// SIGINT will terminate both sockets
void sig_handler(int signo) {
    if(signo == SIGINT) {
        char rbuffer[BUF_SIZE] = "/exit\0";
        ret = sendto(sockfd, rbuffer, BUF_SIZE, 0, (struct sockaddr *) &ser_addr, sizeof(ser_addr));
        if(ret < 0) {
            printf("Error sending the data below:\n\t%s\n", rbuffer);
            exit(1);
        }

        close(sockfd);
        //pthread_exit(NULL);
        printf("Caught sigint\n");
        exit(1);
    }
}


WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win;
           
    local_win = newwin(height, width, starty, startx);
    wrefresh(local_win);

    return local_win;
}

