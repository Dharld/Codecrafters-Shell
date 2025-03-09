#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>

#define MAX_TOKENS 64 // Maximum number of tokens

typedef enum {
  CMD_NONE = 0,
  CMD_EXIT,
  CMD_ECHO,
  CMD_TYPE,
  CMD_EXTERNAL
} CommandType;

typedef struct {
  CommandType type;
  char* name;
  char** args;
  int argc;
} Command;

Command parseCommand(char* input);
void executeCommand(Command cmd);
char* checkCommand(char* cmdName);

extern bool continueRPL;
extern int exitStatus;

#endif // COMMAND_H

