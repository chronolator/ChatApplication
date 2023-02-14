#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

// Globals
WINDOW *chatlog_border_win;
WINDOW *chat_border_win;
WINDOW *chatlog_win;
WINDOW *chat_win;

// Function Prototypes
WINDOW *create_newwin(int height, int width, int starty, int startx);

int main(int argc, char *argv[]) {
    WINDOW *winTest; //The window
    int rows, cols; //Rows and colums in the terminal

    initscr(); //Starting NCurses
    printw("COLS: %d\n", COLS);
    printw("LINES: %d\n", LINES);
    refresh();
    //raw(); //Calling 'getch()' doesn't wait for '\n' 
    getmaxyx(stdscr, rows, cols); //How many rows and colums in stdscr (the terminal)

    //winTest = newwin(10, 10, rows/2, cols/2); //Creates a square WINDOW at the center of the terminal
    chatlog_border_win = create_newwin(LINES-4, COLS-1, 0, 0);
    box(chatlog_border_win, 0, 0);
    chat_border_win = create_newwin(3, COLS-1, LINES-4, 0);
    box(chat_border_win, 0, 0);

    chatlog_win = create_newwin(LINES-6, COLS-3, 1, 1);
    //chat_win = create_newwin(1, COLS-2, LINES-2, 1);
    refresh();

    wprintw(chatlog_border_win, "chatlog_border_win"); //Prints "Test" on the created window
    wprintw(chat_border_win, "chat_border_win");
    wprintw(chatlog_win, "chatlog_win");
    wrefresh(chatlog_border_win);
    wrefresh(chat_border_win);
    wrefresh(chatlog_win);
    wgetch(chatlog_border_win); //Pause
    //wgetch(chat_border_win); //Pause

    delwin(chatlog_win); //Free the memory for winTest
    delwin(chat_win);
    delwin(chatlog_border_win);
    delwin(chat_border_win);
    endwin(); //Ends NCurses
    return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    wrefresh(local_win);

    return local_win;
}
