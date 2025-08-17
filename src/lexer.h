#ifndef __LEXER_H__
#define __LEXER_H__

#define CMD 1
#define PIPE 2
#define AND_IF 3
#define OR_IF 4
#define SEMI 5

#define MAX_PARAMS 80

typedef struct token_t token_t;

typedef struct {
} token_base_t;

typedef struct {
  int type;
  char *head;
  char *parameters[MAX_PARAMS];
} cmd_token_t;

typedef struct {
  int type;
  token_t *left;
  token_t *right;
} pipe_token_t;

struct token_t {
  int type;
  union {
    token_base_t base;
    cmd_token_t cmd;
    pipe_token_t pipe;
  } data;
};

int get_tokens(char *buf, int bufsize, token_t *tokens);

#endif
