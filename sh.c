#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "lexer.c"

#define COMMAND_MAX_S     80
#define PARAM_MAX_S       80

void type_prompt() {
  char *prompt;
  prompt = getenv("PS1");
  #ifdef DEBUG
    printf("[test] ");
  #endif
  if(prompt != NULL)
    printf("%s", prompt);
  else
    if(getuid() == 0)
      printf("# ");
    else
      printf("$ ");
}

void execute(cmd_t *command) {
  execvp(command->head, command->parameters);
}


int main(int argc, char **argv) {
  int status;
  static cmd_t command;
  static char buf[MAX_PARAMS*(PARAM_MAX_S + 1)];
  type_prompt();
  token_t *tokens = malloc(sizeof(token_t) * 100);
  get_tokens(buf, sizeof(buf), tokens);
  return 0;
}
