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
  int matchCount = 0;
  char* matches[256];  // Array to store matches
  static bool tabPressed = false; // To share it for multiple executions
  
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
  
  // Check builtin commands
  for (int i = 0; builtins[i] != NULL; i++) {
    if (strncmp(builtins[i], buffer, cmdLength) == 0) {
      matches[matchCount++] = strdup(builtins[i]);
      if (matchCount >= 256) break;  // Safety check
    }
  }
  
  // Check the executable for the command
  char* path = getenv("PATH");
  
  if (path != NULL) {
    char* pathCopy = strdup(path);
    char* directoryPath = strtok(pathCopy, ":");
    
    while (directoryPath != NULL) {
      DIR* dir = opendir(directoryPath);
      if (dir == NULL) {
        directoryPath = strtok(NULL, ":");
        continue;
      }
      
      struct dirent* entry;
      while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, buffer, cmdLength) == 0) {
          // Check if the file is executable
          char fullPath[2048];
          snprintf(fullPath, sizeof(fullPath), "%s/%s", directoryPath, entry->d_name);
          
          if (access(fullPath, X_OK) == 0) {
            bool isDuplicate = false;

            for (int i = 0; i < matchCount; i++) {
              if (strcmp(matches[i], entry->d_name) == 0) {
                isDuplicate = true;
                break;
              }
            }

            if (!isDuplicate) {
              matches[matchCount++] = strdup(entry->d_name);
              if (matchCount >= 256) break;
            }
          }
        }
      }
      
      closedir(dir);
      directoryPath = strtok(NULL, ":");
    }
    
    free(pathCopy);
  }
 
  // Comparison function for qsort
  int compare_strings(const void* a, const void* b) {
      return strcmp(*(const char**)a, *(const char**)b);
  }

  // In your completeCommand function, after collecting all matches:
  if (matchCount > 1) {
      // Sort matches alphabetically
      qsort(matches, matchCount, sizeof(char*), compare_strings);
  }

  if (matchCount == 0) {
    // No matches found, ring the bell
    printf("\a");
    fflush(stdout);
    tabPressed = false;  // Reset tab state
  } 
  else if (matchCount == 1) {
    // Exactly one match, complete it
    complete(buffer, matches[0], args, position);
    tabPressed = false;  // Reset tab state
  }
  else {
    // Multiple matches
    if (!tabPressed) {
      // First tab press with multiple matches - just ring the bell
      printf("\a");
      fflush(stdout);
      tabPressed = true;  // Remember tab was pressed
    } else {
      // Second tab press - show all possible completions
      printf("\n");
      
      // Display all matches with double spaces between them
      for (int i = 0; i < matchCount; i++) {
        printf("%s  ", matches[i]);
      }
      
      // Return to prompt with original input
      printf("\n$ %s", buffer);
      tabPressed = false;  // Reset tab state
    }
  }
  
  // Free allocated memory for matches
  for (int i = 0; i < matchCount; i++) {
    free(matches[i]);
  }}
