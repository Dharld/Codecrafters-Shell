#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_TOKENS 64 // Maximum number of tokens

bool continueRPL = true;
int exitStatus = 1;

typedef enum {
  CMD_NONE = 0,
  CMD_EXIT,
  CMD_ECHO,
  CMD_TYPE
} CommandType;



typedef struct {
  CommandType type;
  char* name;
  char** args;
  int argc;
} Command;

// Method to parse the command

Command parseCommand(char* input) {
  // Initialize command object
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

  // Tokenize the input
  char* token;
  char* tokens[MAX_TOKENS];  // Assume MAX_TOKENS is defined
  int tokenCount = 0;

  // Read the first token as the token name
  token = strtok(strCopy, " \t\n");
  //  printf("%s is the current token\n", token);

  if (token == NULL) {
    free(strCopy);
    return cmd;
  }

  cmd.name = token;
  tokens[tokenCount++] = token;

  while ((token = strtok(NULL, " \t\n")) != NULL &&  tokenCount < MAX_TOKENS) {
    tokens[tokenCount++] = token;
  }

  // Null terminate the token array
  tokens[tokenCount] = NULL;

  // Store all the arguments
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

  // Parse the input name
  if(cmd.name != NULL) {
    if (strcmp(cmd.name, "exit") == 0) {
      cmd.type = CMD_EXIT;
    } else if(strcmp(cmd.name, "echo") == 0) {
      cmd.type = CMD_ECHO;
    } else if(strcmp(cmd.name, "type") == 0) {
      cmd.type = CMD_TYPE;
    }
  }

  return cmd;
}

char* checkCommand(char* cmdName) {
  char* path = getenv("PATH");
  
  // Check if the path is present
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
void executeCommand(Command cmd) {
  // printf("This is the command   type: %d, name: %s\n", cmd.type, cmd.name);
  switch(cmd.type) {
    case CMD_EXIT:
      continueRPL = false;
      exitStatus = atoi(cmd.args[1]);
      // printf("Status code: %d\n", exitStatus)
      exit(exitStatus);
      break;

    case CMD_ECHO:
      if (cmd.argc == 1) {
        printf("Error: Not enough arguments -> exit --args\n");
        return;
      }

      // Print all the arguments of the command
      for(int i = 1; i < cmd.argc; i++) {
        printf("%s ", cmd.args[i]);
      }

      // Go to the line
      printf("\n");
      break;

    case CMD_TYPE:
      if (cmd.argc == 1) {
        printf("Error: Not enough arguments -> type --args\n");
        return;
      }
      
      char* cmdName = cmd.args[1];

      // Define the built-in
      char* builtIns[] = {"exit", "echo", "type", NULL};
      
      bool found = false;
      for (int i = 0; builtIns[i] != NULL; i++) {
        if (strcmp(cmdName, builtIns[i]) == 0) {
          found = true;
          break;
        }
      }


      if(found) {
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
  };
}

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);
  
  printf("$ ");

  // Wait for user input
  char input[100];

  while (continueRPL && fgets(input, 100, stdin)) {
    // Remove the new trailing new line
    size_t len = strlen(input);
    input[len - 1] = '\0';

    Command cmd = parseCommand(input);

    if(cmd.type == CMD_NONE) {
      printf("%s: command not found\n", input);
    } else {
      executeCommand(cmd);
    }
    
    if (continueRPL)
      printf("$ ");
  }
 

  return exitStatus;
}
