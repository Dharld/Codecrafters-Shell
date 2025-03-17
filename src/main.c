#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"


int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);
  
  // Enable raw mode once at the beginning
  enableRawMode();
  atexit(disableRawMode);  // Make sure to restore terminal at exit
  
  printf("$ ");
  
  char input[100] = {0};
  int position = 0;
  
  while (continueRPL) {
    char c;
    if(read(STDIN_FILENO, &c, 1) == 1) {
      if (c == TAB_KEY) {
        // Handle TAB Completion
        completeCommand(input, &position);
      } 
      else if(c == '\n') {
        if (strcmp(input, "exit") != 0 && strncmp(input, "exit ", 5) != 0) {
          printf("\n"); // Only print newline if not exit
        }

        if (position > 0) {
          Command cmd = parseCommand(input);
          if (cmd.type == CMD_NONE) {
            printf("%s: command not found\n", input);
          } else {
            executeCommand(cmd);
          }
          freeCommand(&cmd);
        }        
        // Reset for next command
        position = 0;
        input[0] = '\0';
        
        if (continueRPL) {
          printf("$ ");
        }
      }
      else if(c == 127) { // Backspace
        if (position > 0) {
          position--;
          input[position] = '\0';
          printf("\b \b"); // Erase character
        }
      }
      else if (!iscntrl(c)) {  // Regular character
        if (position < sizeof(input) - 1) {
          input[position++] = c;
          input[position] = '\0';
          printf("%c", c);  // Echo the character
        }
      }
    }
  }
 
  return exitStatus;
}

