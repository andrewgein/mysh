#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
void print_ast_tree(ast_node_t *root);
#endif

ast_node_t *parse_cmd(token_t *tokens, int *shift) {
  token_t *cur = tokens + *shift;
  if (cur == NULL) {
    return NULL;
  }
  ast_node_t *node = malloc(sizeof(ast_node_t));
  node->type = CMD;
  node->data.cmd.head = cur->data.cmd.head;
  memcpy(node->data.cmd.parameters, cur->data.cmd.parameters,
         sizeof(cur->data.cmd.parameters));
  *shift += 1;
  return node;
}

int is_logical(token_t *tokenp) {
  return (tokenp != NULL && (tokenp->type == AND_IF || tokenp->type == OR_IF ||
                             tokenp->type == SEMI));
}

ast_node_t *parse_log(token_t *tokens, int *shift) {
  ast_node_t *left = parse_cmd(tokens, shift);

  while (is_logical(tokens + *shift)) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    node->type = (tokens + *shift)->type;
    *shift += 1;
    node->data.log.right = parse_cmd(tokens, shift);
    node->data.log.left = left;
    left = node;
  }

  return left;
}

ast_node_t *parse_tokens(token_t *tokens) {
  int *shift = malloc(sizeof(int));
  ast_node_t *root = parse_log(tokens, shift);
#ifdef DEBUG
  puts("###########ORDER OF EXECUTION###########");
  print_ast_tree(root);
#endif
  return root;
}

void print_ast_node(ast_node_t *node) {
  switch (node->type) {
  case AND_IF:
    puts("AND");
    break;
  case OR_IF:
    puts("OR");
    break;
  case SEMI:
    puts("SEMI");
    break;
  case CMD:
    printf("CMD ");
    for (int i = 0; node->data.cmd.parameters[i] != NULL; i++) {
      printf("%s, ", node->data.cmd.parameters[i]);
    }
    printf("NULL\n");
    break;
  default:
    puts("UNSUPPORTED!");
  }
}

void print_ast_tree(ast_node_t *root) {
  if (root == NULL) {
    return;
  }
  if (root->type == AND_IF || root->type == OR_IF || root->type == SEMI) {
    print_ast_tree(root->data.log.left);
    print_ast_node(root);
    print_ast_tree(root->data.log.right);
  } else {
    print_ast_node(root);
  }
}
