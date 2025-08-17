#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
void print_ast_tree(ast_node_t *root);
#endif

ast_type_t to_ast_logical_type(token_type_t type) {
  switch (type) {
  case TK_AND_IF:
    return AST_AND_IF;
  case TK_OR_IF:
    return AST_OR_IF;
  case TK_SEMI:
    return AST_SEMI;
  default:
    puts("parser: Unsupported logical type!");
    exit(1);
  }
}

ast_node_t *parse_cmd(token_t *token, int *shift);
ast_node_t *parse_subshell(token_t *token, int *shift);
ast_node_t *parse_logical(token_t *token, int *shift);

ast_node_t *parse_cmd(token_t *tokens, int *shift) {
  token_t *cur = tokens + *shift;
  if (cur == NULL) {
    return NULL;
  }
  ast_node_t *node = malloc(sizeof(ast_node_t));
  node->type = AST_CMD;
  node->data.cmd.head = cur->data.cmd.head;
  memcpy(node->data.cmd.parameters, cur->data.cmd.parameters,
         sizeof(cur->data.cmd.parameters));
  *shift += 1;
  return node;
}

ast_node_t *parse_subshell(token_t *tokens, int *shift) {
  if ((tokens + *shift)->type == TK_SUBSH_OPEN) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    *shift += 1;
    node->type = AST_SUBSH;
    node->data.subsh.content = parse_logical(tokens, shift);
    if ((tokens + *shift)->type != TK_SUBSH_CLOSE) {
      puts("parser: Synthax error - closing parenthesis missing");
      exit(1);
    }
    *shift += 1;
    return node;
  }
  return parse_cmd(tokens, shift);
}

int is_logical(token_t *tokenp) {
  return (tokenp != NULL &&
          (tokenp->type == TK_AND_IF || tokenp->type == TK_OR_IF ||
           tokenp->type == TK_SEMI));
}

ast_node_t *parse_logical(token_t *tokens, int *shift) {
  ast_node_t *left = parse_subshell(tokens, shift);

  while (is_logical(tokens + *shift)) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    node->type = to_ast_logical_type((tokens + *shift)->type);
    *shift += 1;
    node->data.log.right = parse_subshell(tokens, shift);
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
  default:
    printf("%d UNSUPPORTED!", node->type);
  }
}

void print_ast_tree(ast_node_t *root) {
  if (root == NULL) {
    return;
  }
  if(root->type == AST_SUBSH) {
    print_ast_node(root);
    puts("(");
    print_ast_tree(root->data.subsh.content);
    puts(")");
  }
  else if (root->type == AST_AND_IF || root->type == AST_OR_IF ||
      root->type == AST_SEMI) {
    print_ast_tree(root->data.log.left);
    print_ast_node(root);
    print_ast_tree(root->data.log.right);
  } else {
    print_ast_node(root);
  }
}
