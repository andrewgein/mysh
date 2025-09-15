#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "editor.h"
#include "lexer.h"
#include "list.h"
#include "parser.h"
#include "runner.h"

char *get_pompt() {
  char *prompt;
  prompt = getenv("PS1");
#ifdef DEBUG
  return strdup("\e[0;31m[debug]\033[0m ");
#endif
  if (prompt != NULL)
    return prompt;
  else if (getuid() == 0)
    return strdup("# ");
  else
    return strdup("$ ");
}

int main(int argc, char **argv) {
  token_list_t *tokens, *start;

  input_editor_t ed = editor_init(get_pompt());
  char *line;
  while ((line = readline(&ed)) && (tokens = get_tokens(line))) {
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
    // type_prompt();
  }
  editor_cleanup(&ed);
  return 0;
}
