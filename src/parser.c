#include "parser.h"
#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MISPRT_ERROR_MSG "parser: Synthax error - closing parenthesis missing!"
#define UNSPTYPE_ERROR_MSG "parser: Unsupported logical type!"

#ifdef DEBUG
void print_ast_tree(ast_node_t *root);
#endif

ast_node_t *parse_cmd(token_t *token, int *shift);
ast_node_t *parse_subshell(token_t *token, int *shift);
ast_node_t *parse_redirection(token_t *token, int *shift);
ast_node_t *parse_pipes(token_t *token, int *shift);
ast_node_t *parse_logical(token_t *token, int *shift);

ast_type_t to_ast_logical_type(token_type_t type) {
  switch (type) {
  case TK_AND_IF:
    return AST_AND_IF;
  case TK_OR_IF:
    return AST_OR_IF;
  case TK_SEMI:
    return AST_SEMI;
  default:
    puts(UNSPTYPE_ERROR_MSG);
    exit(1);
  }
}

ast_node_t *parse_subcmd(token_t *tokens, int *shift) {
  if ((tokens + *shift)->type == TK_CMD_SUB_OPEN) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    *shift += 1;
    node->type = AST_SUBCMD;
    node->data.cmdsub.cmd = parse_logical(tokens, shift);
    if ((tokens + *shift)->type != TK_CMD_SUB_CLOSE) {
      puts(MISPRT_ERROR_MSG);
      exit(1);
    }
    *shift += 1;
    return node;
  }
  return NULL;
}

ast_node_t *parse_cmd(token_t *tokens, int *shift) {
  token_t *cur = tokens + *shift;
  if (cur == NULL) {
    return NULL;
  }
  ast_node_t *result = malloc(sizeof(ast_node_t));
  ast_node_t *cmdnode = result;
  token_t arg;
  cmdnode->type = AST_CMD;
  cmdnode->data.cmd.head = cur->data.cmd.head;
  int argn, i;
  argn = 0;
  for (i = 0; i < cur->data.cmd.argc; i++) {
    arg = cur->data.cmd.parameters_array[i];
    cmdnode->data.cmd.parameters[argn] = malloc(MAX_PARAM_LEN);
    if (arg.type == TK_WORD) {
      strcpy(cmdnode->data.cmd.parameters[argn], arg.data.word.str);
    } else if (arg.type == TK_CMD_SUB_OPEN) {
      ast_node_t *upper = malloc(sizeof(ast_node_t));
      upper = parse_subcmd(cur->data.cmd.parameters_array, &i);
      i--;
      upper->data.cmdsub.result = cmdnode->data.cmd.parameters[argn];
      upper->data.cmdsub.next = result;
      result = upper;
    } else {
      puts(UNSPTYPE_ERROR_MSG);
      exit(1);
    }
    argn++;
  }
  *shift += 1;
  return result;
}

ast_node_t *parse_subshell(token_t *tokens, int *shift) {
  if ((tokens + *shift)->type == TK_SUBSH_OPEN) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    *shift += 1;
    node->type = AST_SUBSH;
    node->data.subsh.content = parse_logical(tokens, shift);
    if ((tokens + *shift)->type != TK_SUBSH_CLOSE) {
      puts(MISPRT_ERROR_MSG);
      exit(1);
    }
    *shift += 1;
    return node;
  }
  return parse_cmd(tokens, shift);
}

ast_node_t *parse_redirection(token_t *tokens, int *shift) {
  ast_node_t *left = parse_subshell(tokens, shift);
  if ((tokens + *shift)->type == TK_REDIRECT) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    node->type = AST_REDIRECT;
    node->data.redir.left = left;
    node->data.redir.rdinfo = (tokens + *shift)->data.redir;
    *shift += 1;
    return node;
  }
  return left;
}

ast_node_t *parse_pipe(token_t *tokens, int *shift) {
  ast_node_t *left = parse_redirection(tokens, shift);
  while ((tokens + *shift)->type == TK_PIPE) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    *shift += 1;
    node->type = AST_PIPE;
    node->data.pipe.right = parse_redirection(tokens, shift);
    node->data.pipe.left = left;
    left = node;
  }

  return left;
}

int is_logical(token_t *tokenp) {
  return (tokenp != NULL &&
          (tokenp->type == TK_AND_IF || tokenp->type == TK_OR_IF ||
           tokenp->type == TK_SEMI));
}

ast_node_t *parse_logical(token_t *tokens, int *shift) {
  ast_node_t *left = parse_pipe(tokens, shift);
  while (is_logical(tokens + *shift)) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    node->type = to_ast_logical_type((tokens + *shift)->type);
    *shift += 1;
    node->data.log.right = parse_pipe(tokens, shift);
    node->data.log.left = left;
    left = node;
  }

  return left;
}

ast_node_t *parse_tokens(token_t *tokens) {
  int *shift = malloc(sizeof(int));
  ast_node_t *root = parse_logical(tokens, shift);
#ifdef DEBUG
  puts("###########AST###########");
  print_ast_tree(root);
  puts("");
#endif
  return root;
}

void print_ast_node(ast_node_t *node) {
  switch (node->type) {
  case AST_AND_IF:
    puts("AND");
    break;
  case AST_OR_IF:
    puts("OR");
    break;
  case AST_SEMI:
    puts("SEMI");
    break;
  case AST_CMD:
    printf("CMD ");
    for (int i = 0; node->data.cmd.parameters[i] != NULL; i++) {
      printf("%s, ", node->data.cmd.parameters[i]);
    }
    printf("NULL\n");
    break;
  case AST_SUBSH:
    puts("SUBSH");
    break;
  case AST_PIPE:
    puts("PIPE");
    break;
  case AST_REDIRECT:
    switch (node->data.redir.rdinfo.type) {
    case RD_IN:
      printf("REDIRECT_IN, fd: %d, dup: %d, file: %s\n",
             node->data.redir.rdinfo.fd, node->data.redir.rdinfo.dup,
             node->data.redir.rdinfo.file);
      break;
    case RD_OUT:
      printf("REDIRECT_OUT, fd: %d, dup: %d, file: %s\n",
             node->data.redir.rdinfo.fd, node->data.redir.rdinfo.dup,
             node->data.redir.rdinfo.file);
      break;
    case RD_INOUT:
      puts("REDIRECT_IN_OUT");
      break;
    case APP_OUT:
      printf("APPEND_OUT, fd: %d, dup: %d, file: %s\n",
             node->data.redir.rdinfo.fd, node->data.redir.rdinfo.dup,
             node->data.redir.rdinfo.file);
      break;
    }
    break;
  case AST_SUBCMD:
    printf("SUBCMD\n");
    print_ast_node(node->data.cmdsub.cmd);
    print_ast_node(node->data.cmdsub.next);
    break;

  default:
    printf("%d UNSUPPORTED!", node->type);
  }
}

void print_ast_tree(ast_node_t *root) {
  if (root == NULL) {
    return;
  }
  if (root->type == AST_SUBSH) {
    print_ast_node(root);
    puts("(");
    print_ast_tree(root->data.subsh.content);
    puts(")");
  } else if (root->type == AST_AND_IF || root->type == AST_OR_IF ||
             root->type == AST_SEMI || root->type == AST_PIPE) {
    print_ast_tree(root->data.log.left);
    print_ast_node(root);
    print_ast_tree(root->data.log.right);
  } else if (root->type == AST_REDIRECT) {
    print_ast_tree(root->data.redir.left);
    print_ast_node(root);
  } else {
    print_ast_node(root);
  }
}
