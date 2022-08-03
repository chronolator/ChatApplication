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

// Globals
int sockfd, len, ret, newsockfd;
struct sockaddr_in ser_addr, cli_addr;

/* NCURSES Globals */
WINDOW *chatlog_border_win;
WINDOW *chat_border_win;
WINDOW *chatlog_win;
WINDOW *chat_win;

// Functions
void *recMsg(void *sock);
void sig_handler(int signo);

/* NCURSES Functions */
WINDOW *create_newwin(int height, int width, int starty, int startx);

int main(int argc, char *argv[]){ 
    const int PORT = 8000;
    const int REUSE_ENABLE = 1;
    const char NT[1] = {'\0'};

    char buffer[BUF_SIZE];
    char clientAddr[CLIADDR_LEN];
    pthread_t rThread;
    //pid_t childpid;
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(PORT);
    ser_addr.sin_addr.s_addr = INADDR_ANY;

    /* NCURSES Programming */
    initscr();
    //printw("COLS: %d\n", COLS);
    //printw("LINES: %d\n", LINES);
    //getch();
    //refresh();

    //int height = 50;
    //int width = 50;
    //int starty = 0; //COLS/2;
    //int startx = 0; //LINES/2;
    
    /* NCURSES Format Chatlog Border Window */
    chatlog_border_win = create_newwin(LINES-4, COLS-1, 0, 0);
    box(chatlog_border_win, 0, 0);
    chat_border_win = create_newwin(3, COLS-1, LINES-4, 0);
    box(chat_border_win, 0, 0);
    wrefresh(chatlog_border_win);
    wrefresh(chat_border_win);

    /* NCURSES Format Chatlog Window */
    chatlog_win = create_newwin(LINES-6, COLS-3, 1, 1);
    chat_win = create_newwin(1, COLS-3, LINES-3, 1);
    wrefresh(chatlog_win);
    wrefresh(chat_win);

    /* NCURSES Programming */

    
    // Catch sigint
    if(signal(SIGINT, sig_handler) == SIG_ERR) {
        wprintw(chatlog_win, "Unable to catch SIGINT\n");
        wrefresh(chatlog_win);
        exit(1);
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        wprintw(chatlog_win, "Error creating socket\n");
        wrefresh(chatlog_win);
        exit(1);
    }
    wprintw(chatlog_win, "Socket created\n");
    wrefresh(chatlog_win);


    // Set reuse socket option
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &REUSE_ENABLE, sizeof(REUSE_ENABLE)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    // Bind to socket
    ret = bind(sockfd, (struct sockaddr *) &ser_addr, sizeof(ser_addr));
    if(ret < 0) {
        wprintw(chatlog_win, "Error binding to socket\n");
        wrefresh(chatlog_win);
        exit(1);
    }
    wprintw(chatlog_win, "Bound to socket\n");
    wrefresh(chatlog_win);

    // Socket is listening
    wprintw(chatlog_win, "\nListenting for any incoming clients\n");
    wrefresh(chatlog_win);
    listen(sockfd, 5);

    // Accept incoming connections
    len = sizeof(cli_addr);
    wprintw(chatlog_win, "Size of cli_addr: %ld\n", sizeof(cli_addr));
    wprintw(chatlog_win, "Size of ser_addr: %ld\n", sizeof(ser_addr));
    wrefresh(chatlog_win);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *) &len);
    //newsockfd = accept(sockfd, NULL, NULL);
    if(newsockfd < 0) {
        wprintw(chatlog_win, "Error accepting connection\n");
        wrefresh(chatlog_win);
        exit(1);
    }

    // Convet IPv4 and IPV6 address from binary to text form
    inet_ntop(AF_INET, &(cli_addr.sin_addr), clientAddr, CLIADDR_LEN);
    wprintw(chatlog_win, "Connection accepted from %s...\n", clientAddr);
    wrefresh(chatlog_win);
    
    // Create a pthread for receiving messages from the client
    ret = pthread_create(&rThread, NULL, recMsg, (void *) newsockfd);
    if(ret < 0) {
        wprintw(chatlog_win, "Error from pthread_create() is %d\n", ret);
        wrefresh(chatlog_win);
        exit(1);
    }

    // Stdin data sent to the client
    /*printw(chatlog_win, "SERVER> ");
    while(fgets(buffer, BUF_SIZE, stdin) != NULL) {
        printw("SERVER> ");
        ret = sendto(newsockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &cli_addr, len);
        if(ret < 0) {
            printw("Error sending the data below to the client:\n\t%s\n", buffer);
            exit(1);
        }
    }*/
    //wprintw(chatlog_win, "SERVER> ");

    // Move cursor to the chat_win window
    wmove(chat_win, 0, 0);

    // Input do while to refresh chat_win 
    do {
        wclear(chat_win);
        mvwprintw(chat_win, 0, 0, "SERVER> ");
        wrefresh(chat_win);
        wgetnstr(chat_win, buffer, sizeof(buffer)); // This might the problem with the socket o rmaybe its not null terminated
        //fflush(stdout);
        //buffer[sizeof(BUF_SIZE-1)] = '\0';
        wprintw(chatlog_win, "Length of string buffer: %d\n", strlen(buffer));
        memset(buffer, 0, sizeof(buffer));
        ret = sendto(newsockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &cli_addr, len); // Or this
        //ret = sendto(newsockfd, NT, sizeof(NT), 0, (struct sockaddr *) &cli_addr, len); // Or this
        if(ret < 0) {
            wprintw(chatlog_win, "Error sending the data below to the client:\n\t%s\n", buffer);
            wrefresh(chatlog_win);
            exit(1);
        }
        wprintw(chatlog_win, "YOU: %s\n", buffer);
        wrefresh(chatlog_win);
        
    } while(1);

    // Close and return everything 
    close(newsockfd);
    close(sockfd);
    pthread_exit(NULL);
    delwin(chatlog_border_win);
    delwin(chat_border_win);
    delwin(chatlog_win);
    delwin(chat_win);
    endwin();   /* NCURSES: closes curses mode */
    return EXIT_SUCCESS;
}

// Pthread receiving a message from the client socket
void *recMsg(void *socket) {
    int rret;
    int rsockfd = ((int) socket);
    char rbuffer[BUF_SIZE];

    for(;;) {
        memset(rbuffer, 0, sizeof(rbuffer));
        //rret = recvfrom(rsockfd, rbuffer, BUF_SIZE, 0, NULL, NULL);

        if(rret < 0) {
            wprintw(chatlog_win, "Error receiving data.\n");
            wrefresh(chatlog_win);
        } else {
            if(strncmp(rbuffer, "/exit\0", BUF_SIZE) == 0) {
                wprintw(chatlog_win, "Obtained the exit\n");
                wrefresh(chatlog_win);
                close(newsockfd);
                close(sockfd);
                system("stty sane; clear");
                //pthread_exit(NULL);
                exit(1);
            }
            wprintw(chatlog_win, "client: %s", rbuffer);
            wrefresh(chatlog_win);
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
            wprintw(chatlog_win, "Error sending the data below to the client:\n\t%s\n", rbuffer);
            wrefresh(chatlog_win);
            exit(1);
        }

        close(newsockfd);
        close(sockfd);
        //pthread_exit(NULL);
        wprintw(chatlog_win, "Caught sigint\n"); 
        wrefresh(chatlog_win);
        system("stty sane; clear;");
        exit(1);
    }
}

/* NCURSES Functions */
WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win;
    
    local_win = newwin(height, width, starty, startx);
    wrefresh(local_win);

    return local_win;
}
