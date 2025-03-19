#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>


#define TAB_KEY 9
// Enable raw mode 

void enableRawMode();
void disableRawMode();
void completeCommand(char*, int*);
