// Implement Snake on the command line under linux using ncurses library.
//
// Inspired by:
//   https://youtu.be/e8lYLYlrGLg
//   
// Controls are Arrow keys Left, Right, Up, & Down, eat food, grow larger.
//  Avoid self!
//
//  Build: 
//	  gcc -Werror -Wall -Wextra -Wconversion -g -c rolling_log.c	
//    gcc -o snake snake.c -lncurses rolling_log.o -lm   
//

#include <stdio.h>          
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>    //  /usr/include/ncurses.h

#include "rolling_log.h"

#define nFieldWidth     80
#define nFieldHeight    20

extern char g_szLogFileName[MAX_LOG_FILE_NAME];


// Draw the 12 x 18 playing field with a border.
//
void draw(WINDOW *field_win)
{
    // Update display Frame
    wrefresh(field_win);
}
  
int main(int argc, char **argv)
{
    WINDOW *field_win;
    int c, nScore = 0;

    argc = argc;
    strncpy(g_szLogFileName, argv[0], MAX_LOG_FILE_NAME - 1);
    log_printf("Start\n");
    srand(time(0));
    
    initscr();  // Init ncurses library
    clear();    // Clear the screen and rerender on next refresh.
    noecho();   // Don't echo typed characters.
    cbreak();   // Line buffering disabled, w/ "ctrl-c" break signal.

	nodelay(stdscr, TRUE);		// Non-blocking wgetch
    keypad(stdscr, TRUE);    	// Enable reading of function keys like F1, arrow keys...
	
    static int startx = (80 - nFieldWidth) / 2;
    static int starty = (24 - nFieldHeight) / 2;
        
    field_win = newwin(nFieldHeight, nFieldWidth, starty, startx);
    mvprintw(0, 0, "Controls are Arrow keys Left, Right, Up, & Down.'Q' to quit.");
    mvprintw(1, 0, "Eat food, grow larger, avoid self!");
    refresh();

    bool bGameOver = false;

    while(!bGameOver) { 

        // Timing =======================
        //
        static struct timespec ts = {0, 50000000};	// Blocks the execution of the current thread for 50ms	
        nanosleep(&ts, NULL);

        // Input ========================
		//
        c = wgetch(stdscr);  // non-blocking
        if (c != ERR) {

	        // Handle player commands =======
	        // 
	        switch(c) { 
	            case KEY_LEFT:
	                break;
	                
	            case KEY_RIGHT:
	                break;

	            case KEY_UP:
	                break;
	                
	            case KEY_DOWN:
	                break;
	                
	            case 'Q':        // Quit.
	            case 'q':
	                bGameOver = true;
	                break;
            }
        }

		// Display ======================
		//
        draw(field_win);

		// Draw Score
		mvprintw(2, 0, "Score: %8d", nScore); clrtoeol();
		refresh();
		
    }   
    
    mvprintw(3, 0, "Game Over! Hit a key to exit."); clrtoeol();
	refresh();
    nodelay(stdscr, FALSE);		// Blocking wgetch
	wgetch(stdscr);				// Blocks
    endwin();   				// Cleanup ncurses library.
    return 0;
}
