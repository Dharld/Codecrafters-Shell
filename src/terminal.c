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

void complete(char* buffer, char* text, char* args, int* position) {
   // move cursor to beginning of line and clear it
  printf("\r");        // carriage return moves to beginning of line
  printf("\033[K");    // clear line from cursor position to end
  
  // replace buffer with completed command
  strcpy(buffer, text);
  strcat(buffer, args);
 
  // printf("this is your buffer: %s\n", buffer);

  // if there's no arguments
  if (args[0] == '\0') {
    strcat(buffer, " ");  // add space after command
  }

  *position = strlen(buffer);
  
  // print prompt and completed command
  printf("$ %s", buffer); 
}

void completeCommand(char* buffer, int* position) {
  char* builtins[] = {"echo", "exit", NULL};
  bool foundMatch = false; // To check for a match
  
  // Create a copy of the buffer
  char fullBuffer[1024];
  strcpy(fullBuffer, buffer);

  // Find the first space for the arguments
  char* space = strchr(buffer, ' ');
  int cmdLength = space ? space - buffer : *position;

  char args[1024] = {'\0'};
  if (space) {
    strcpy(args, space); // Include all the arguments including the space
  }
  
  // printf("This is space:%s\n", space);

  // Iterate through every builtin command
  for (int i = 0; builtins[i] != NULL; i++) {
    if (strncmp(builtins[i], buffer, cmdLength) == 0) {
      foundMatch = true;
      complete(buffer, builtins[i], args, position);
      break;
    }
  }
  
  // Check the executable for the command
  char* path = getenv("PATH");
  
  if (path == NULL) {
    if(!foundMatch) {
      printf("\a");
      fflush(stdout);
    }
    return;
  }
  
  char* pathCopy = strdup(path);
  char* directoryPath = strtok(pathCopy, ":");
  
  while (directoryPath != NULL && !foundMatch) {
    DIR* dir = opendir(directoryPath);
    if (dir == NULL) {
      directoryPath = strtok(NULL, ":");
      continue;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
      if (strncmp(entry->d_name, buffer, cmdLength) == 0) {
        foundMatch = true;
        complete(buffer, entry->d_name, args, position);
        break;
      }
    }
    
    closedir(dir);
    if (!foundMatch) {
      directoryPath = strtok(NULL, ":");
    }
  }
  
  free(pathCopy);
  
  if(!foundMatch) {
    printf("\a");
    fflush(stdout);
  }}

