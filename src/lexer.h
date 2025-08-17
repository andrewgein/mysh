#ifndef __LEXER_H__
#define __LEXER_H__

#define MAX_PARAMS 80 // TODO: move all to one file

typedef enum {
  TK_CMD,
  TK_PIPE,
  TK_AND_IF,
  TK_OR_IF,
  TK_SEMI,
  TK_SUBSH_OPEN,
  TK_SUBSH_CLOSE
} token_type_t;


typedef struct token_t token_t;

typedef struct {
} token_base_t;

typedef struct {
  char *head;
  char *parameters[MAX_PARAMS];
} cmd_token_t;

typedef struct {
  token_t *left;
  token_t *right;
} pipe_token_t;

struct token_t {
  token_type_t type;
  union {
    token_base_t base;
    cmd_token_t cmd;
    pipe_token_t pipe;
  } data;
};

int get_tokens(char *buf, int bufsize, token_t *tokens);

#endif
