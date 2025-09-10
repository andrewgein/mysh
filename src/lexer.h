#ifndef __LEXER_H__
#define __LEXER_H__

#include "list.h"

#define MAX_PARAMS 80 // TODO: move all to one file

typedef enum {
  TK_WORD,
  TK_CMD,
  TK_PIPE,
  TK_AND_IF,
  TK_OR_IF,
  TK_SEMI,
  TK_FD_NUMBER,
  TK_SUBSH_OPEN,
  TK_SUBSH_CLOSE,
  TK_CMD_SUB_OPEN,
  TK_CMD_SUB_CLOSE,
  TK_VAR_EXP_START,
  TK_REDIRECT,
  TK_HEREDOC
} token_type_t;

typedef struct token_t token_t;
typedef list_node_t token_list_t;

typedef struct {
} token_base_t;

typedef struct {
  char *str;
} word_token_t;

typedef struct {
  char *head;
  token_list_t *parameters;
  token_t *parameters_array; // TODO remove this mf
  int argc;
} cmd_token_t;

typedef struct {
  token_t *left;
  token_t *right;
} pipe_token_t;

typedef struct {
  int fd;
} fd_num_token_t;

typedef enum {
  RD_IN,
  RD_OUT,
  RD_INOUT,
  APP_OUT
} redir_type_t;

typedef struct {
  redir_type_t type;
  int fd;
  int dup;
  char *file;
} redir_token_t;

struct token_t {
  token_type_t type;
  int buf_shift;
  union {
    token_base_t base;
    word_token_t word;
    cmd_token_t cmd;
    pipe_token_t pipe;
    redir_token_t redir;
    fd_num_token_t fd_num;
  } data;
};


int get_tokens(char *buf, int bufsize, token_t *tokens);

#endif
