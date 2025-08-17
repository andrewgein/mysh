#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "lexer.h"
#include "parser.h"
#include "runner.h"

#define COMMAND_MAX_S     80
#define PARAM_MAX_S       80

#define TOKEN_MAX_S      100

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

int main(int argc, char **argv) {
  int status;
  static char buf[MAX_PARAMS*(PARAM_MAX_S + 1)];
  type_prompt();
  token_t *tokens = calloc(sizeof(token_t),  TOKEN_MAX_S);
  while(get_tokens(buf, sizeof(buf), tokens)) {
    run(parse_tokens(tokens));
    memset(tokens, 0, TOKEN_MAX_S * sizeof(token_t));
    type_prompt();
  }
  return status;
}
