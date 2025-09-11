#include "parser.h"
#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define P_MISPRT_ERROR_MSG "parser: Synthax error - closing parenthesis missing!"
#define P_UNSPTYPE_ERROR_MSG "parser: Unsupported logical type!"
#define P_MLLC_ERROR_MSG "parser: Malloc error"

#ifdef DEBUG
void print_ast_tree(ast_node_t *root);
#endif

ast_node_t *parse_subcmd(token_list_t **it);
ast_node_t *parse_cmd(token_list_t **it);
ast_node_t *parse_subshell(token_list_t **it);
ast_node_t *parse_redirection(token_list_t **it);
ast_node_t *parse_pipes(token_list_t **it);
ast_node_t *parse_logical(token_list_t **it);

ast_type_t to_ast_logical_type(token_type_t type) {
  switch (type) {
  case TK_AND_IF:
    return AST_AND_IF;
  case TK_OR_IF:
    return AST_OR_IF;
  case TK_SEMI:
    return AST_SEMI;
  default:
    puts(P_UNSPTYPE_ERROR_MSG);
    exit(1);
  }
}

void next(token_list_t **it) {
  if (*it == NULL) {
    return;
  }
  *it = (*it)->next;
}

token_t *get(token_list_t **it) {
  if (*it == NULL) {
    return NULL;
  }
  return (*it)->data;
}

token_type_t get_type(token_list_t **it) {
  if (*it == NULL) {
    return -1;
  }
  return ((token_t *)(*it)->data)->type;
}

ast_node_t *parse_subcmd(token_list_t **it) {
  if (get_type(it) == TK_CMD_SUB_OPEN) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    if (node == NULL) {
      puts(P_MLLC_ERROR_MSG);
      exit(1);
    }
    next(it);
    node->type = AST_CMDSUB;
    node->data.cmdsub.cmd = parse_logical(it);
    if (get_type(it) != TK_CMD_SUB_CLOSE) {
      puts(P_MISPRT_ERROR_MSG);
      exit(1);
    }
    next(it);
    return node;
  }
  return NULL;
}

ast_node_t *parse_cmd(token_list_t **it) {
  if (*it == NULL) {
    return NULL;
  }
  ast_node_t *result = malloc(sizeof(ast_node_t));
  if (result == NULL) {
    puts(P_MLLC_ERROR_MSG);
    exit(1);
  }
  ast_node_t *cmdnode = result;
  token_list_t *arg = get(it)->data.cmd.parameters;
  cmdnode->type = AST_CMD;
  cmdnode->data.cmd.head = malloc(strlen(get(it)->data.cmd.head) + 1);
  if (cmdnode->data.cmd.head == NULL) {
    puts(P_MLLC_ERROR_MSG);
    exit(1);
  }
  strcpy(cmdnode->data.cmd.head, get(it)->data.cmd.head);
  int argn = 0;
  while (arg != NULL) {
    cmdnode->data.cmd.parameters[argn] = malloc(MAX_PARAM_LEN);
    if (cmdnode->data.cmd.parameters[argn] == NULL) {
      puts(P_MLLC_ERROR_MSG);
      exit(1);
    }
    if (get_type(&arg) == TK_WORD) {
      strcpy(cmdnode->data.cmd.parameters[argn], get(&arg)->data.word.str);
      arg = arg->next;
    } else if (get_type(&arg) == TK_CMD_SUB_OPEN) {
      ast_node_t *upper = malloc(sizeof(ast_node_t));
      upper = parse_subcmd(&arg);
      upper->data.cmdsub.result = cmdnode->data.cmd.parameters[argn];
      upper->data.cmdsub.next = result;
      result = upper;
    } else {
      puts(P_UNSPTYPE_ERROR_MSG);
      exit(1);
    }
    argn++;
  }
  next(it);
  return result;
}

ast_node_t *parse_subshell(token_list_t **it) {
  if (get_type(it) == TK_SUBSH_OPEN) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    if (node == NULL) {
      puts(P_MLLC_ERROR_MSG);
      exit(1);
    }
    next(it);
    node->type = AST_SUBSH;
    node->data.subsh.content = parse_logical(it);
    if (get_type(it) != TK_SUBSH_CLOSE) {
      puts(P_MISPRT_ERROR_MSG);
      exit(1);
    }
    next(it);
    return node;
  }
  return parse_cmd(it);
}

ast_node_t *parse_redirection(token_list_t **it) {
  ast_node_t *left = parse_subshell(it);
  if (get_type(it) == TK_REDIRECT) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    if (node == NULL) {
      puts(P_MLLC_ERROR_MSG);
      exit(1);
    }
    node->type = AST_REDIRECT;
    node->data.redir.left = left;
    node->data.redir.rdinfo = get(it)->data.redir;
    next(it);
    return node;
  }
  return left;
}

ast_node_t *parse_pipe(token_list_t **it) {
  ast_node_t *left = parse_redirection(it);
  while (get_type(it) == TK_PIPE) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    if (node == NULL) {
      puts(P_MLLC_ERROR_MSG);
      exit(1);
    }
    next(it);
    node->type = AST_PIPE;
    node->data.pipe.right = parse_redirection(it);
    node->data.pipe.left = left;
    left = node;
  }

  return left;
}

int is_logical(token_list_t **it) {
  if (*it == NULL) {
    return 0;
  }
  token_t *tokenp = (*it)->data;
  return (tokenp != NULL &&
          (tokenp->type == TK_AND_IF || tokenp->type == TK_OR_IF ||
           tokenp->type == TK_SEMI));
}

ast_node_t *parse_logical(token_list_t **it) {
  ast_node_t *left = parse_pipe(it);
  while (is_logical(it)) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    if (node == NULL) {
      puts(P_MLLC_ERROR_MSG);
      exit(1);
    }
    node->type = to_ast_logical_type(get_type(it));
    next(it);
    node->data.log.right = parse_pipe(it);
    node->data.log.left = left;
    left = node;
  }

  return left;
}

ast_node_t *parse_tokens(token_list_t **tokens) {
  ast_node_t *root = parse_logical(tokens);
#ifdef DEBUG
  puts("###########AST###########");
  print_ast_tree(root);
  puts("");
#endif
  return root;
}

void parser_cleanup(ast_node_t *root) {
  switch (root->type) {
  case AST_CMD:
    free(root->data.cmd.head);
    for (int i = 0; i < MAX_PARAMS; i++) {
      if (root->data.cmd.parameters[i] == NULL) {
        break;
      }
      free(root->data.cmd.parameters[i]);
    }
    break;
  case AST_OR_IF:
  case AST_AND_IF:
  case AST_SEMI:
    parser_cleanup(root->data.log.left);
    parser_cleanup(root->data.log.right);
    break;
  case AST_SUBSH:
    parser_cleanup(root->data.subsh.content);
    break;
  case AST_REDIRECT:
    parser_cleanup(root->data.redir.left);
    // free(root->data.redir.rdinfo.file); ALREADY FREED
    break;
  case AST_PIPE:
    parser_cleanup(root->data.pipe.left);
    parser_cleanup(root->data.pipe.right);
    break;
  case AST_CMDSUB:
    parser_cleanup(root->data.cmdsub.cmd);
    parser_cleanup(root->data.cmdsub.next);
    // free(root->data.cmdsub.result); ALREADY FREED
    break;
  }
  free(root);
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
  case AST_CMDSUB:
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
