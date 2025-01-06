// Implement tetris on the command line under linux using ncurses library.
//
// Inspired by:
//   https://www.youtube.com/watch?v=8OK8_tHeCIA
//   
//  Controls are Arrow keys Left, Right & Down. Use 'Z' to rotate the piece. 'Q' to quit. 
//  You score 25pts per tetronimo, and 2^(number of lines)*100 when you get lines.
//
//  Build: 
//	  gcc -Werror -Wall -Wextra -Wconversion -g -c rolling_log.c	
//    gcc -o tetris tetris.c -lncurses rolling_log.o -lm   
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

extern char g_szLogFileName[MAX_LOG_FILE_NAME];

// Seven tetrominos of 4x4 characters, '.'=clear, 'X'=set
//
char *tetromino[7] = {"..X...X...X...X.",   // 0  A----------------------------------A
                      "..X..XX..X......",   // 1  B-----------------------------B    A
                      ".X...XX...X.....",   // 2  C-----------------------C    BB    A
                      ".....XX..XX.....",   // 3  D------------------DD   CC   B     A
                      "..X..XX...X.....",   // 4  E-------------E    DD    C            
                      ".....XX...X...X.",   // 5  F-------FF   EE
                      ".....XX..X...X.."};  // 6  G--GG    F    E
                                            //       G     F    
#define BORDER          9                   //       G
#define nFieldWidth     12                  // 
#define nFieldHeight    18                  //
uint8_t cField[nFieldWidth * nFieldHeight] = {
    9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,
    9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,
    9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,9,9,9,9,9,9,9,9,9,9,9,9,9};
                                 
// Rotate tetromino (4x4 matrix) by providing asset index from x and y in non-rotated space.
//
int Rotate(int x, int y, int r)
{
    switch (r % 4) {
        case 0: // 0 degrees 
            return ((y*4)+x);
        case 1: // 90 degrees           
            return (12+y-(x*4));
        case 2: // 180 degrees          
            return (15-(y*4)-x);
        case 3: // 270 degrees          
            return (3-y+(x*4));
    }                               
}

// Collision detection against other Tetrominos and field boundery. 
//
bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
    // All Field cells >0 are occupied
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            // Get index into piece
            int pi = Rotate(px, py, nRotation);

            // Get index into field
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

            // Check that test is in bounds. Note out of bounds does
            // not necessarily mean a fail, as the long vertical piece
            // can have cells that lie outside the boundary, so we'll
            // just ignore them
            if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
                    // In Bounds so do collision check
                    if (tetromino[nTetromino][pi] != '.' && cField[fi] != 0)
                        return false; // fail on first hit
                }
            }
        }
	}
    return true;
}

// Draw the 12 x 18 playing field with a border.
//
void draw_field(WINDOW *field_win, int nCurPiece, int nCurRot, int nPosX, int nPosY)
{
    // Render to the field window from the cField. 'A'-'G': tetrominos, 
    // '=': completed line, '#': border.
    //
    for (int x = 0; x < nFieldWidth; x++)
        for (int y = 0; y < nFieldHeight; y++)      // 0123456789
            mvwaddch(field_win, y, x, " ABCDEFG=#"[cField[(y * nFieldWidth) + x]]);

    // Draw Current Piece
    for (int px = 0; px < 4; px++)
        for (int py = 0; py < 4; py++)
            if (tetromino[nCurPiece][Rotate(px, py, nCurRot)] != '.')
                mvwaddch(field_win, (nPosY + py), (nPosX + px), nCurPiece + 65);


#ifdef fool
    // Animate Line Completion
    if (!vLines.empty())
    {
        // Display Frame (cheekily to draw lines)
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
        this_thread::sleep_for(400ms); // Delay a bit

        for (auto &v : vLines)
            for (int px = 1; px < nFieldWidth - 1; px++)
            {
                for (int py = v; py > 0; py--)
                    pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                pField[px] = 0;
            }

        vLines.clear();
    }
#endif
    // Update display Frame
    wrefresh(field_win);
}
  
int main(int argc, char **argv)
{
    WINDOW *field_win;
    int c;

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
    mvprintw(0, 0, "Controls are Arrow keys Left, Right & Down.");
    mvprintw(1, 0, "Use 'Z' to rotate the piece. 'Q' to quit.");
    refresh();

    // Game Logic
    int  nCurrentPiece = rand() % 7;
    int  nCurrentRotation = 0;
    int  nCurrentX = (nFieldWidth / 2) - 2;
    int  nCurrentY = 0;
    int  nSpeed = 20;
    int  nSpeedCount = 0;
    bool bForceDown = false;
    bool bRotateHold = true;
    int  nPieceCount = 0;
    int  nScore = 0;
//  vector<int> vLines;
    bool bGameOver = false;

    while(!bGameOver) { 

        // Timing =======================
        //
        static struct timespec ts = {0, 50000000};	// Blocks the execution of the current thread for 50ms	
        nanosleep(&ts, NULL);
        nSpeedCount++;
        bForceDown = (nSpeedCount == nSpeed);

        // Input ========================
		//
        c = wgetch(stdscr);  // non-blocking
        if (c != ERR) {

	        // Handle player commands =======
	        // 
	        switch(c) { 
	            case KEY_LEFT:  // Move piece left.
        			nCurrentX -= (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;      
	                break;
	                
	            case KEY_RIGHT: // Move piece right.
					nCurrentX += (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
	                break;
	                
	            case KEY_DOWN:  // Move piece down.
					nCurrentY += (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
	                break;
	                
	            case 'Z':       // Rotate the piece.
	            case 'z':
					nCurrentRotation += (bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
					bRotateHold = false;
	                break;
	
	            case 'Q':        // Quit.
	            case 'q':
	                bGameOver = true;
	                log_printf("key: %c\n", c);
	                break;
	        }
		}      
        bRotateHold = true;

        // Force the piece down the playfield if it's time
        if (bForceDown) {
			
            // Update difficulty every 50 pieces
            nSpeedCount = 0;
            nPieceCount++;
            if ((nPieceCount % 50 == 0) && (nSpeed >= 10)) nSpeed--;
            
            // Test if piece can be moved down
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) nCurrentY++; // It can, so do it!
            else {
                // It can't! Lock the piece in place
                for (int px = 0; px < 4; px++)
                    for (int py = 0; py < 4; py++)
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
                            cField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

                // Check for lines
                for (int py = 0; py < 4; py++) {
                    if(nCurrentY + py < nFieldHeight - 1) {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth - 1; px++)
                            bLine &= (cField[(nCurrentY + py) * nFieldWidth + px]) != 0;

                        if (bLine) {
                            // Remove Line, set to =
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                cField[(nCurrentY + py) * nFieldWidth + px] = 8;
//                            vLines.push_back(nCurrentY + py);
                        }                       
                    }
				}
                nScore += 25;
//                if(!vLines.empty()) nScore += (1 << vLines.size()) * 100;

                // Pick New Piece
                nCurrentX = (nFieldWidth / 2) - 2;
                nCurrentY = 0;
                nCurrentRotation = 0;
                nCurrentPiece = rand() % 7;
                 
                // If piece does not fit straight away, game over!
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }
        }

		// Display ======================
		//
        draw_field(field_win, nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

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
