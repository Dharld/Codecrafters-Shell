#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"

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
    
    if (continueRPL) {
      printf("$ ");
      fflush(stdout); // make sure it's displayed immediately
    }

  }
 

  return exitStatus;
}


