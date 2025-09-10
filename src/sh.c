#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "lexer.h"
#include "list.h"
#include "parser.h"
#include "runner.h"

#define COMMAND_MAX_S     80
#define PARAM_MAX_S       80

#define TOKEN_MAX_S      100

void type_prompt() {
  char *prompt;
  prompt = getenv("PS1");
  #ifdef DEBUG
    printf("\e[0;31m[debug]\033[0m ");
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
  token_list_t *tokens, *start;
  while((tokens = get_tokens(buf, sizeof(buf)))) {
    start = tokens;
    ast_node_t *root = parse_tokens(&start);
#ifdef DEBUG
    puts("###########RESULT###########");
#endif
    run(root);
#ifdef DEBUG
    puts("");
#endif
    lexer_cleanup(tokens);
    parser_cleanup(root);
    type_prompt();
  }
  return status;
}
