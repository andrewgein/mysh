#include "lexer.h"

#include "editor.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMMAND_MAX_S 80

#define L_SYNTHAX_ERROR "lexer: Synthax error near -> %s\n"
#define L_USYNTHAX_ERROR "lexer: Synthax error"
#define L_MLLC_ERROR "lexer: Malloc error"
#define L_MISPRT_ERROR_MSG "lexer: Error: unclosed parenthesis"
#define L_REDIR_ERROR_MSG "lexer: TK_REDIRECT Error! TODO: add buf_shift"
#define L_FDN_ERROR_MSG "lexer: TK_FD_NUMBER Error! TODO: add buf_shift"
#define L_CMDSUB_ERROR_MSG                                                     \
  "lexer: TK_CMD_SUB_OPEN Error! (command substitution without parent "        \
  "command) TODO: add buf_shift"

const char whitespace[] = "\t\r\n\v ";
const char breaksymbols[] = "&|;()<>";

char *read_word(char *buf, int *shift);
char *read_special_symbol(char *buf, int *shift);
token_t *get_token(char *buf, int *shift);
token_list_t *merge_tokens(token_list_t *tklist);
#ifdef DEBUG
void print_tokens(token_list_t *tokens, int n);
#endif

void report_synthax_error(char *buf, int *shift) {
  if (*shift + 10 < COMMAND_MAX_S) {
    buf[*shift + 10] = '\0';
  }
  printf(L_SYNTHAX_ERROR, buf + *shift);
  exit(1);
}

int is_end_of_input(char *buf) {
  int buflen = strlen(buf);
  return !((buflen >= 2 && buf[buflen - 2] == '\\') ||
           (buflen >= 3 && buf[buflen - 2] == '&' && buf[buflen - 3] == '&') ||
           (buflen >= 3 && buf[buflen - 2] == '|' && buf[buflen - 3] == '|'));
}

token_list_t *get_tokens(char *buf) {
  token_t *token;
  char *bufp;
  int shift;
  token_list_t *tklist = NULL;
  shift = 0;
  bufp = buf;
  int bufsize = 80;

  while ((token = get_token(buf, &shift)) != NULL) {
    if (tklist == NULL) {
      tklist = init(token);
    } else {
      push_back(tklist, token);
    }
    bufp += shift;
  }
  tklist = merge_tokens(tklist);
#ifdef DEBUG
  puts("\n###########TOKENS###########");
  print_tokens(tklist, get_length(tklist));
#endif
  return tklist;
}

void skip_whitespaces(char *buf, int *shift) {
  char *bufp = buf + *shift;
  char *bufendp = buf + strlen(buf);
  while ((bufp < bufendp) && (strchr(whitespace, *bufp) != NULL))
    bufp++;
  *shift = bufp - buf;
}

int is_redirection(char *bufp) {
  if ((bufp != NULL) && isdigit(*bufp)) {
    bufp += 1;
  }
  if ((bufp != NULL) && (*bufp == '&')) {
    bufp += 1;
  }
  return (bufp != NULL) && (*bufp == '<' || *bufp == '>');
}

char *read_word(char *buf, int *shift) {
  char *word;
  char *tokenstartp;
  char *tokenendp;
  char *bufp = buf + *shift;
  char *bufendp = buf + strlen(buf);
  while ((bufp < bufendp) && (strchr(whitespace, *bufp) == NULL) &&
         (strchr(breaksymbols, *bufp) == NULL)) {
    bufp++;
  }
  tokenstartp = buf + *shift;
  tokenendp = bufp;
  if (tokenstartp == tokenendp) {
    return NULL;
  }
  word = malloc(sizeof(char) * (tokenendp - tokenstartp + 1));
  if (word == NULL) {
    puts(L_MLLC_ERROR);
    exit(1);
  }
  memcpy(word, tokenstartp, (tokenendp - tokenstartp));
  word[tokenendp - tokenstartp] = 0;
  *shift = bufp - buf;
  return word;
}

token_t *read_io_number(char *buf, int *shift) {
  token_t *token;
  char *cur = buf + *shift;
  char *next = buf + *shift + 1;
  if (!isdigit(*(cur))) {
    return NULL;
  }
  if (!strchr("<>&|", *next)) {
    return NULL;
  }
  token = malloc(sizeof(token_t));
  if (token == NULL) {
    puts(L_MLLC_ERROR);
    exit(1);
  }
  token->type = TK_FD_NUMBER;
  token->data.fd_num.fd = *cur - '0';
  (*shift)++;
  return token;
}

token_type_t peek_opened_token(list_node_t *opened_tokens) {
  if (get_length(opened_tokens) == 0) {
    puts(L_MISPRT_ERROR_MSG);
    exit(1);
  }
  token_type_t tk = *(token_type_t *)get_last(opened_tokens)->data;
  if (get_length(opened_tokens) == 1) {
    destroy(opened_tokens);
  } else {
    destroy_next(get_item(opened_tokens, get_length(opened_tokens) - 2));
  }
  return tk;
}

list_node_t *put_opened_token(token_type_t tk, list_node_t *opened_tokens) {
  token_type_t *tkp = malloc(sizeof(token_type_t));
  if (tkp == NULL) {
    puts(L_MLLC_ERROR);
    exit(1);
  }
  *tkp = tk;
  if (opened_tokens == NULL) {
    return init(tkp);
  }
  push_back(opened_tokens, tkp);
  return opened_tokens;
}

token_t *get_token(char *buf, int *shift) {
  static list_node_t *opened_tokens;

  token_t *token;
  char *cur;
  char *next;
  skip_whitespaces(buf, shift);
  cur = buf + *shift;
  next = buf + *shift + 1;
  if (*cur == '\0') {
    return NULL;
  }

  if ((token = read_io_number(buf, shift))) {
    return token;
  }

  token = malloc(sizeof(token_t));
  if (token == NULL) {
    puts(L_MLLC_ERROR);
    exit(1);
  }
  switch (*cur) {
  case '&':
    if (*next == '&') {
      token->type = TK_AND_IF;
      *shift += 2;
    } else if (*next == '>') {
      token->type = TK_REDIRECT;
      token->data.redir.type = RD_IN;
      token->data.redir.dup = 1;
      *shift += 2;
    } else {
      token->type = TK_WORD;
      *shift += 1;
    }
    break;

  case '|':
    if (*next == '|') {
      token->type = TK_OR_IF;
      *shift += 2;
    } else {
      token->type = TK_PIPE;
      *shift += 1;
    }
    break;

  case ';':
    token->type = TK_SEMI;
    *shift += 1;
    break;
  case '(':
    *shift += 1;
    token->type = TK_SUBSH_OPEN;
    opened_tokens = put_opened_token(TK_SUBSH_OPEN, opened_tokens);
    break;
  case ')':
    if (get_length(opened_tokens) == 0) {
      report_synthax_error(buf, shift);
    }
    *shift += 1;
    token_type_t tokent;
    switch (peek_opened_token(opened_tokens)) {
    case TK_SUBSH_OPEN:
      token->type = TK_SUBSH_CLOSE;
      break;
    case TK_CMD_SUB_OPEN:
      token->type = TK_CMD_SUB_CLOSE;
      break;
    default:
      report_synthax_error(buf, shift);
      break;
    }
    break;

  case '>':
    if (*next == '>') {
      token->type = TK_REDIRECT;
      token->data.redir.type = APP_OUT;
      *shift += 2;
    } else {
      token->type = TK_REDIRECT;
      token->data.redir.type = RD_OUT;
      *shift += 1;
    }
    break;

  case '<':
    if (*next == '>') {
      token->type = TK_REDIRECT;
      token->data.redir.type = RD_INOUT;
      *shift += 2;
    } else if (*next == '&') {
      token->type = TK_REDIRECT;
      token->data.redir.type = RD_IN;
      token->data.redir.dup = 1;
      *shift += 2;
    } else {
      token->type = TK_REDIRECT;
      token->data.redir.type = RD_IN;
      *shift += 1;
    }
    break;

  case '$':
    if (*next == '(') {
      token->type = TK_CMD_SUB_OPEN;
      *shift += 2;
      opened_tokens = put_opened_token(TK_CMD_SUB_OPEN, opened_tokens);
    } else {
      token->type = TK_ENV_VAR_START;
      *shift += 1;
    }
    break;

  default:
    token->type = TK_WORD;
    token->data.word.str = read_word(buf, shift);
  }
  return token;
}

token_list_t *merge_tokens(token_list_t *head) {
  token_t *curtk;
  token_t *nexttk;
  if (head == NULL) {
    return NULL;
  }
  curtk = head->data;
  if (head->next == NULL) {
    nexttk = NULL;
  } else {
    nexttk = head->next->data;
  }
  int fd;
  switch (curtk->type) {
  case TK_REDIRECT:
    if (nexttk == NULL || nexttk->type != TK_WORD) {
      puts(L_REDIR_ERROR_MSG);
      exit(1);
    }
    curtk->data.redir.fd = -1;
    curtk->data.redir.file = nexttk->data.word.str;
    remove_next(head);
    break;
  case TK_WORD:
    curtk->type = TK_CMD;
    curtk->data.cmd.head = curtk->data.word.str;
    token_t *first_parameter = malloc(sizeof(token_t));
    if (first_parameter == NULL) {
      puts(L_MLLC_ERROR);
      exit(1);
    }
    first_parameter->type = TK_WORD;
    first_parameter->data.word.str = malloc(strlen(curtk->data.cmd.head) + 1);
    if (first_parameter->data.word.str == NULL) {
      puts(L_MLLC_ERROR);
      exit(1);
    }
    strcpy(first_parameter->data.word.str, curtk->data.cmd.head);
    curtk->data.cmd.parameters = init(first_parameter);
    token_list_t *origin = head;
    for (int i = 1; nexttk != NULL; i++) {
      if (nexttk->type == TK_CMD_SUB_CLOSE) {
        return head->next;
      }
      if (nexttk->type != TK_WORD && nexttk->type != TK_CMD_SUB_OPEN) {
        break;
      }
      if (nexttk->type == TK_WORD) {
        push_back(curtk->data.cmd.parameters, nexttk);
        remove_next(head);
      } else if (nexttk->type == TK_CMD_SUB_OPEN) {
        head = merge_tokens(head->next->next);
        for (list_node_t *it = origin->next; it != head; it = it->next) {
          push_back(curtk->data.cmd.parameters, it->data);
        }
        push_back(curtk->data.cmd.parameters, head->data);
        for (list_node_t *it = origin->next; it != head; it = it->next) {
          remove_next(it);
        }
        remove_next(origin); // OPEN
        remove_next(origin); // CLOSE
        head = origin;       // head = CLOSE(deleted) -> head = CMD
      }

      if (head->next == NULL) {
        break;
      }
      nexttk = head->next->data;
    }

    break;
  case TK_FD_NUMBER:
    if (nexttk == NULL || nexttk->type != TK_REDIRECT) {
      puts(L_FDN_ERROR_MSG);
      exit(1);
    }
    fd = curtk->data.fd_num.fd;
    curtk->type = nexttk->type;
    curtk->data = nexttk->data;
    curtk->data.redir.fd = fd;
    remove_next(head);
    merge_tokens(head);
    break;

  case TK_CMD_SUB_CLOSE:
    return head;

  case TK_CMD_SUB_OPEN:
    puts(L_CMDSUB_ERROR_MSG);
    exit(1);

  default:
    break;
  }
  merge_tokens(head->next);
  return head;
}

void lexer_cleanup(token_list_t *tokens) {
  if (tokens == NULL) {
    return;
  }
  token_t *tokenp = tokens->data;
  token_list_t dummy;
  switch (tokenp->type) {
  case TK_WORD:
    free(tokenp->data.word.str);
    break;
  case TK_CMD:
    free(tokenp->data.cmd.head);
    lexer_cleanup(tokenp->data.cmd.parameters);
    break;
  case TK_PIPE:
    dummy.next = NULL;
    dummy.data = tokenp->data.pipe.left;
    lexer_cleanup(&dummy);
    dummy.data = tokenp->data.pipe.right;
    lexer_cleanup(&dummy);
    break;
  case TK_REDIRECT:
    free(tokenp->data.redir.file);
    break;
  default:
    break;
  }
  token_list_t *cur = tokens;
  tokens = cur->next;
  free(cur);
  lexer_cleanup(tokens);
}

void print_tokens(token_list_t *tokens, int n) {
  token_t *tokenp;
  cmd_token_t cmd;
  for (int i = 0; i < n; i++) {
    tokenp = tokens->data;
    tokens = tokens->next;
    printf("%d\n", i);
    switch (tokenp->type) {
    case TK_WORD:
      puts("type: WORD");
      printf("content: %s\n", tokenp->data.word.str);
      break;
    case TK_CMD:
      cmd = tokenp->data.cmd;
      puts("type: command");
      printf("head: %s\n", cmd.head);
      puts("/////////ARGUMENTS//////////");
      for (int j = 0; j < get_length(cmd.parameters); j++) {
        printf("%d", j);
        print_tokens(get_item(cmd.parameters, j), 1);
      }
      printf("NULL\n");
      puts("\\\\\\\\\\\\\\\\\\ARGUMENTS\\\\END\\\\\\\\\\\\\\\\\\\\");
      break;
    case TK_SEMI:
      puts("type: SEMI");
      break;
    case TK_OR_IF:
      puts("type: OR_IF");
      break;
    case TK_AND_IF:
      puts("type: AND_IF");
      break;
    case TK_SUBSH_OPEN:
      puts("type: SUBSH_OPEN");
      break;
    case TK_SUBSH_CLOSE:
      puts("type: SUBSH_CLOSE");
      break;
    case TK_CMD_SUB_OPEN:
      puts("type: CMD_SUB_OPEN");
      break;
    case TK_CMD_SUB_CLOSE:
      puts("type: CMD_SUB_CLOSE");
      break;
    case TK_PIPE:
      puts("type: PIPE");
      break;
    case TK_ENV_VAR_START:
      puts("type: ENV_VAR");
      break;

    case TK_REDIRECT:
      switch (tokenp->data.redir.type) {
      case RD_IN:
        printf("type: REDIRECT_IN, fd: %d, dup: %d\n", tokenp->data.redir.fd,
               tokenp->data.redir.dup);
        printf("file: %s\n", tokenp->data.redir.file);
        break;

      case RD_OUT:
        printf("type: REDIRECT_OUT, fd: %d, dup: %d\n", tokenp->data.redir.fd,
               tokenp->data.redir.dup);
        printf("file: %s\n", tokenp->data.redir.file);
        break;

      case RD_INOUT:
        puts("type: REDIRECT_IN_OUT");
        break;

      case APP_OUT:
        printf("type: APPEND_OUT, fd: %d, dup: %d\n", tokenp->data.redir.fd,
               tokenp->data.redir.dup);
        printf("file: %s\n", tokenp->data.redir.file);
        break;
      }
      break;

    case TK_HEREDOC:
      puts("type: HEREDOC");
      break;

    default:
      puts("type: UNKNOWN");
      break;
    }
    puts("");
  }
}
