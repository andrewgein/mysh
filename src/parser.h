#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"

#define MAX_PARAMS 80 // TODO: move all to one file

typedef struct ast_node_t ast_node_t;

typedef struct {
  char *head;
  char *parameters[MAX_PARAMS];
} ast_cmd_t;

typedef struct {
  int type;
  ast_node_t *left;
  ast_node_t *right;
} ast_log_t;

struct ast_node_t { 
  int type;  
  union {
    ast_cmd_t cmd;
    ast_log_t log;
  } data;
};

ast_node_t *parse_tokens(token_t *tokens);

#endif
