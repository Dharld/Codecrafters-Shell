#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   // for fork, execvp
#include <sys/wait.h> // for waitpid
#include <ctype.h>
#include <fcntl.h>
#include "terminal.h"

#define MAX_TOKENS 64 // Maximum number of tokens
#define PATH_MAX 1024

// Type definition
typedef enum {
  CMD_NONE = 0,
  CMD_EXIT,
  CMD_ECHO,
  CMD_TYPE,
  CMD_PWD,
  CMD_CD,
  CMD_EXTERNAL
} CommandType;

typedef struct {
  CommandType type;
  char* name;
  char** args;
  int argc;

  // For redirection
  bool hasOutputRedirection;
  bool hasErrorRedirection;
  bool appendOutput;
  bool appendError;

  char *outputFile;
  char *errorOutputFile;
} Command;

// Function headers
void freeCommand(Command* cmd);
Command parseCommand(char* input);
void executeCommand(Command cmd);
char* checkCommand(char* cmdName);
void printWorkingDirectory();
int changeDirectory(char* path);

extern bool continueRPL;
extern int exitStatus;

#endif // COMMAND_H

