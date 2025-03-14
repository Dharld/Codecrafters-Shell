#include "terminal.h"

struct termios original_termios;

// Enable raw mode 
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
      // Move cursor to beginning of line and clear it
      printf("\r");        // Carriage return moves to beginning of line
      printf("\033[K");    // Clear line from cursor position to end
      
      // Replace buffer with completed command
      strcpy(buffer, builtins[i]);
      strcat(buffer, " ");  // Add space after command
      *position = strlen(buffer);
      
      // Print prompt and completed command
      printf("$ %s", buffer);
      
      break;
    }
  }
}

