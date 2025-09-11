#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"

#define MAX_PARAMS 80 // TODO: move all to one file
#define MAX_PARAM_LEN 80

typedef enum {
  AST_CMD,
  AST_PIPE,
  AST_SUBSH,
  AST_CMDSUB,
  AST_SEMI,
  AST_AND_IF,
  AST_OR_IF,
  AST_REDIRECT,
} ast_type_t;

typedef struct ast_node_t ast_node_t;

typedef struct {
  char *head;
  char *parameters[MAX_PARAMS];
} ast_cmd_t;

typedef struct {
  ast_node_t *left;
  ast_node_t *right;
} ast_log_t;

typedef struct {
  ast_node_t *content;
} ast_subsh_t;

typedef struct {
  ast_node_t *left;
  ast_node_t *right;
} ast_pipe_t;

typedef struct {
  redir_token_t rdinfo;  
  ast_node_t *left;
} ast_redir_t;

typedef struct {
  ast_node_t *cmd;
  char *result;
  ast_node_t *next;
} ast_cmd_sub_t;

struct ast_node_t { 
  ast_type_t type;
  union {
    ast_cmd_t     cmd;
    ast_log_t     log;
    ast_subsh_t subsh;
    ast_pipe_t   pipe;
    ast_redir_t redir;
    ast_cmd_sub_t cmdsub;
  } data;
};

ast_node_t *parse_tokens(token_list_t **);
void parser_cleanup(ast_node_t *root);

#endif
