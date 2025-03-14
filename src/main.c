#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include "command.h"

#define TAB_KEY 9

// Enable raw mode 
struct termios original_termios;

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &original_termios);
  struct termios raw = original_termios;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void completeCommand(char* buffer, int* position) {
  char* builtins[] = {"echo", "exit", NULL};

  // Iterate through every builtin command
  for (int i = 0; builtins[i] != NULL; i++) {
    if (strncmp(builtins[i], buffer, *position) == 0) {
      // Clear the previous element
      printf("\n");
      printf("$ ");

      strcpy(buffer, builtins[i]);
      strcat(buffer, " ");
      *position = strlen(buffer);
    
      // Print the completed command
      printf("%s", buffer);
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);
  
  tcgetattr(STDIN_FILENO, &original_termios);

  printf("$ ");

  // Wait for user input
  char input[100];
  int position = 0; // Handle the position of the cursor

  while (continueRPL) {
    // Enable raw mode for each character
    enableRawMode();
    
    char c;
    if(read(STDIN_FILENO, &c, 1) == 1) {
      if (c == TAB_KEY) {
        // Handle TAB Completion
        completeCommand(input, &position);
      } 
      else if(c == '\n') {
        // Process the command when Enter is pressed
        disableRawMode();
        printf("\n");

        input[position] = '\0';

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
          fflush(stdout);
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

    disableRawMode();
  }
 

  return exitStatus;
}


