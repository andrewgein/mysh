#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"

#define MAX_PARAMS 80 // TODO: move all to one file

typedef enum {
  AST_CMD,
  AST_PIPE,
  AST_SUBSH,
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

struct ast_node_t { 
  ast_type_t type;
  union {
    ast_cmd_t     cmd;
    ast_log_t     log;
    ast_subsh_t subsh;
    ast_pipe_t   pipe;
    ast_redir_t redir;
  } data;
};

ast_node_t *parse_tokens(token_t *tokens);

#endif
