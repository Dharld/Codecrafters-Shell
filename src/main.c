#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);
  
  printf("$ ");

  // Wait for user input
  char input[100];
  while (fgets(input, 100, stdin)) {
    // Remove the new trailing new line
    size_t len = strlen(input);
    input[len - 1] = '\0';

    printf("%s: command not found\n", input);

    printf("$ ");
  }
 

  return 0;
}
