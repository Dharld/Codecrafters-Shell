#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  // Uncomment this block to pass the first stage
  printf("$ ");

  // Wait for user input
  char input[100];
  if (fgets(input, 100, stdin) != NULL) {
    // Remove the new trailing new line
    size_t len = strlen(input);
    input[len - 1] = '\0';

    printf("%s: command not found\n", input);
  } else {
    printf("Error reading input\n");
  }; 


  return 0;
}
