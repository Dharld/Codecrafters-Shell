#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>   // for fork, execvp
#include <sys/wait.h> // for waitpid
#include "command.h"

bool continueRPL = true;
int exitStatus = 1;

Command parseCommand(char* input) {
  Command cmd;
  cmd.type = CMD_NONE;
  cmd.name = NULL;
  cmd.args = NULL;
  cmd.argc = 0;

  char* strCopy = strdup(input);
  if (strCopy == NULL) {
    fprintf(stderr, "Memory allocation failed!\n");
    return cmd;
  }

  char* token;
  char* tokens[MAX_TOKENS];
  int tokenCount = 0;

  token = strtok(strCopy, " \t\n");

  if (token == NULL) {
    free(strCopy);
    return cmd;
  }

  cmd.name = token;
  tokens[tokenCount++] = token;

  while ((token = strtok(NULL, " \t\n")) != NULL && tokenCount < MAX_TOKENS) {
    tokens[tokenCount++] = token;
  }

  tokens[tokenCount] = NULL;
  cmd.args = malloc(tokenCount * sizeof(char*));

  if(cmd.args == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    free(strCopy);
    return cmd;
  }

  for (int i = 0; i < tokenCount; i++) {
    cmd.args[i] = tokens[i];
  }

  cmd.argc = tokenCount;

  if (cmd.name != NULL) {
    if (strcmp(cmd.name, "exit") == 0) {
      cmd.type = CMD_EXIT;
    } else if(strcmp(cmd.name, "echo") == 0) {
      cmd.type = CMD_ECHO;
    } else if(strcmp(cmd.name, "type") == 0) {
      cmd.type = CMD_TYPE;
    } 
    else if(strcmp(cmd.name, "pwd") == 0) {
      cmd.type = CMD_PWD;
    }
    else if(strcmp(cmd.name, "cd") == 0) {
      cmd.type = CMD_CD;
    }
    else {
      cmd.type = CMD_EXTERNAL;
    }
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
  if (path == NULL) {
        // No path provided, go to HOME
        path = getenv("HOME");
        if (path == NULL) {
            return -1; // Error: HOME not set
        }
    }
    
    return chdir(path); // Returns 0 on success, -1 on error
}

void executeCommand(Command cmd) {
  switch(cmd.type) {
    case CMD_EXIT:
      continueRPL = false;
      exitStatus = atoi(cmd.args[1]);
      exit(exitStatus);
      break;

    case CMD_ECHO:
      if (cmd.argc == 1) {
        printf("Error: Not enough arguments -> exit --args\n");
        return;
      }

      for(int i = 1; i < cmd.argc; i++) {
        printf("%s ", cmd.args[i]);
      }
      printf("\n");
      break;

    case CMD_TYPE:
      if (cmd.argc == 1) {
        printf("Error: Not enough arguments -> type --args\n");
        return;
      }
      
      char* cmdName = cmd.args[1];

      char* builtIns[] = {"exit", "echo", "type", "pwd", NULL};
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
        }
      }
      break;
    
    case CMD_PWD:
      printWorkingDirectory();
      break;
    
    case CMD_CD:
      if(cmd.argc == 1) {
        printf("Error: Not enough arguments -> cd --path\n");
        return;
      }

      char* path = cmd.args[1];
      if(changeDirectory(path) < 0) {
        fprintf(stderr, "cd: %s No such file or directory\n", path);
        return;
      }
      break;


    case CMD_EXTERNAL:
      pid_t pid = fork();
      
      if (pid == -1) {
        // Fork failed
        fprintf(stderr, "Fork failed.");
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
      break;
  }
}

