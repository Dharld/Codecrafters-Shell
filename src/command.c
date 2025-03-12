#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>   // for fork, execvp
#include <sys/wait.h> // for waitpid
#include <ctype.h>
#include <fcntl.h>
#include "command.h"

bool continueRPL = true;
int exitStatus = 1;

CommandType getCommandType(char* path) {
  if (path == NULL) {
    return CMD_NONE;
  }  
  if (strcmp("exit", path) == 0) {
    return CMD_EXIT;
  } else if (strcmp("echo", path) == 0) {
    return CMD_ECHO;
  } else if (strcmp("type", path) == 0) {
    return CMD_TYPE;
  } else if (strcmp("pwd", path) == 0) {
    return CMD_PWD;
  } else if (strcmp("cd", path) == 0) {
    return CMD_CD;
  } else {
  return CMD_EXTERNAL;
  }
}

void handleSingleQuote(int* index, char* input, char** tokens, int* tokenCount) {
    // Skip the start character
    (*index)++;  // Need parentheses around *index
    
    // Collect the word
    char token[1024];
    int i = 0;
    
    while(*index < strlen(input)) {  // Dereferencing index
        token[i++] = input[*index];  // Dereferencing index
        (*index)++;  // Increment after using
        
        // Case of 'hello''world' -> helloworld
        if (*index < strlen(input) && input[*index] == '\'') {
            if (*index + 1 < strlen(input) && input[*index + 1] == '\'') {
                *index = *index + 2;
                continue;
            }
            
            break;
        }     
    }
    token[i] = '\0';
    
    // Skip the closing quote if present
    if (*index < strlen(input) && input[*index] == '\'') {
        (*index)++;
    }
    
    // You need to add this to actually save the token
    tokens[*tokenCount] = strdup(token);
    (*tokenCount)++;
}

void handleDoubleQuote(int* index, char* input, char** tokens, int* tokenCount) {
    // Skip the start character
    (*index)++;
    
    // Collect the word
    char token[1024];
    int i = 0;
    
    while(*index < strlen(input)) {
        // Check for closing quote
        if (input[*index] == '"') {
            if (*index + 1 < strlen(input) && input[*index + 1] == '"') {
                *index = *index + 2;
                continue;
            }
            
            // Check if the next character is a literal character: "hello\"insidequotes"script\"
            if (*index + 1 < strlen(input) && isalnum(input[*index + 1])) {
              // Skip the current character and continue
              (*index)++;
              continue;
            }
            // Skip the current character
            (*index)++;
            break;      
        }
        // Handle escape sequences
        else if (input[*index] == '\\' && *index + 1 < strlen(input)) {
            // Check for special escape characters
            if (input[*index + 1] == '"' || input[*index + 1] == '$' || input[*index + 1] == '\\') {
                token[i++] = input[*index + 1];
                *index += 2;
            } else {
                // For other escapes, include both the backslash and the character
                token[i++] = '\\';
                token[i++] = input[*index + 1];
                *index += 2;
            }
        }
        // Regular character
        else {
            token[i++] = input[*index];
            (*index)++;
        } 
    }
    
    token[i] = '\0';
    tokens[*tokenCount] = strdup(token);
    (*tokenCount)++;
}

Command parseCommand(char* input) {
  Command cmd;
  cmd.type = CMD_NONE;
  cmd.name = NULL;
  cmd.args = NULL;
  cmd.argc = 0;
  cmd.hasOutputRedirection = false;
  cmd.outputFile = NULL;
  
  char* tokens[MAX_TOKENS];
  int tokenCount = 0;
  int index = 0;
  
  while(index < strlen(input)) {
    char ch = input[index];
    
    
    if(ch == '\'') {
      handleSingleQuote(&index, input, tokens, &tokenCount);
    }
    else if(ch == '"') {
      handleDoubleQuote(&index, input, tokens, &tokenCount);
    } 
    else if (isspace(ch)) {
      // Skip whitespace
      index++;
    }
    // In your  function, modify the regular token collection:
    else {
        // Collect a regular token (non-whitespace characters)
        char token[1024];
        int i = 0;
        
        while (index < strlen(input)) {
            // If we hit space or quote without a backslash, end the token
            if (isspace(input[index]) || input[index] == '\'' || input[index] == '"') {
                break;
            }
            
            // Handle backslash - escape the next character
            if (input[index] == '\\' && index + 1 < strlen(input)) {
                index++; // Skip the backslash
                token[i++] = input[index++]; // Add escaped character
                continue;
            }
            
            // Regular character
            token[i++] = input[index++];
        }
        
        token[i] = '\0';
        tokens[tokenCount++] = strdup(token);
    } 
  }
 
  tokens[tokenCount] = NULL;
  
  cmd.name = tokens[0];
  cmd.type = getCommandType(cmd.name);
  
  // Check if there's a token for redirection
  int i = 0;
  for(; i < tokenCount; i++) {
    if (strcmp(">", tokens[i]) == 0 || strcmp("1>", tokens[i]) == 0) {
      cmd.hasOutputRedirection = true;

      // The next element is the output file
      if (i + 1 < tokenCount) {
        cmd.outputFile = strdup(tokens[i + 1]); 
      }
      
      break;
    }
  }

  cmd.argc = i;

  cmd.args = malloc(cmd.argc * sizeof(char*));
  for(int i = 0; i < cmd.argc; i++) {
    cmd.args[i] = tokens[i];
  }


  return cmd;
}

char* checkCommand(char* cmdName) {
  char* path = getenv("PATH");
  
  if (path == NULL) {
    return NULL;
  }

  char* pathCopy = strdup(path);
  char* dir = strtok(pathCopy, ":");

  while (dir != NULL) {
    char fullPath[1024];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", dir, cmdName);
    FILE* file = fopen(fullPath, "r");

    if (file != NULL) {
      fclose(file);
      free(pathCopy);
      char* result = strdup(fullPath);
      return result;
    }

    dir = strtok(NULL, ":");
  }

  free(pathCopy);
  return NULL;
}

void printWorkingDirectory() {
  char currentDir[PATH_MAX];

  if(getcwd(currentDir, sizeof(currentDir)) != NULL) {
    printf("%s\n", currentDir);
  } else {
    fprintf(stderr, "Impossible to print the current working directory.\n");
  }
}


int changeDirectory(char* path) { 
  if (path == NULL || strcmp(path, "~") == 0) { // No path provided, go to HOME
    path = getenv("HOME");
    if (path == NULL) {
      return -1; // Error: HOME not set
    }
  }
    
  return chdir(path); // Returns 0 on success, -1 on error
}

void executeWithRedirection(Command cmd, void (*executeFunc)(Command)) {
  if (cmd.hasOutputRedirection) {
      // Open the file for redirection
      int fd = open(cmd.outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd < 0) {
          perror("open");
          return;
      }
      
      // Save original stdout
      int savedStdOut = dup(STDOUT_FILENO);
      if (savedStdOut < 0) {
          perror("dup");
          close(fd);
          return;
      }
      
      // Redirect stdout to the file
      if (dup2(fd, STDOUT_FILENO) < 0) {
          perror("dup2");
          close(fd);
          close(savedStdOut);
          return;
      }
      close(fd);  // fd is no longer needed as it's duplicated
      
      // Execute the command (output goes to the file)
      executeFunc(cmd);
      
      // Restore original stdout
      fflush(stdout);  // Ensure all output is written before switching
      if (dup2(savedStdOut, STDOUT_FILENO) < 0) {
          perror("dup2 restore");
      }
      close(savedStdOut);
  } else {
      // Execute normally without redirection
      executeFunc(cmd);
  }
}

// Function to execute echo command
void executeEcho(Command cmd) {
  if (cmd.argc == 1) {
      printf("\n");  // Echo with no args just prints a newline
      return;
  }
  
  for(int i = 1; i < cmd.argc; i++) {
      printf("%s", cmd.args[i]);
      if (i < cmd.argc - 1) {
          printf(" ");  // Add space only between arguments
      }
  }
  printf("\n");
}

// For external commands
void executeExternal(Command cmd) {
    pid_t pid = fork();
    
    if (pid == -1) {
        fprintf(stderr, "Fork failed.\n");
    } else if (pid == 0) {
        // Child process
        execvp(cmd.name, cmd.args);
        
        // If execvp returns, it failed
        fprintf(stderr, "%s: command not found\n", cmd.name);
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

// Function to execute type command
void executeType(Command cmd) {
    if (cmd.argc == 1) {
        printf("Error: Not enough arguments -> type --args\n");
        return;
    }
    
    char* cmdName = cmd.args[1];
    char* builtIns[] = {"exit", "echo", "type", "pwd", "cd", NULL};
    bool found = false;
    
    for (int i = 0; builtIns[i] != NULL; i++) {
        if (strcmp(cmdName, builtIns[i]) == 0) {
            found = true;
            break;
        }
    }

    if (found) {
        printf("%s is a shell builtin\n", cmdName);
    } else {
        char* path = checkCommand(cmdName);
        if (path == NULL) {
            printf("%s: not found\n", cmdName);
        } else {
            printf("%s is %s\n", cmdName, path);
            free(path); // Don't forget to free the allocated path
        }
    }
}

// Function to execute pwd command
void executePwd(Command cmd) {
    char currentDir[PATH_MAX];
    if(getcwd(currentDir, sizeof(currentDir)) != NULL) {
        printf("%s\n", currentDir);
    } else {
        fprintf(stderr, "Impossible to print the current working directory.\n");
    }
}

// Function to execute cd command
void executeCd(Command cmd) {
    if(cmd.argc == 1) {
        printf("Error: Not enough arguments -> cd --path\n");
        return;
    }

    char* path = cmd.args[1];
    if(changeDirectory(path) < 0) {
        fprintf(stderr, "cd: %s: No such file or directory\n", path);
    }
}

void executeCommand(Command cmd) {
  switch(cmd.type) {
    case CMD_EXIT:
      continueRPL = false;
      exitStatus = atoi(cmd.args[1]);
      exit(exitStatus);
      break;

    case CMD_ECHO:
      executeWithRedirection(cmd, executeEcho);
      break;

    case CMD_TYPE:
    executeWithRedirection(cmd, executeType);
    break;
    
    case CMD_PWD:
        executeWithRedirection(cmd, executePwd);
        break;
        
    case CMD_CD:
        executeWithRedirection(cmd, executeCd);
        break;

    case CMD_EXTERNAL:
      executeWithRedirection(cmd, executeExternal);
      break;
  }
}

