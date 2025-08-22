#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMMAND_MAX_S 80

const char whitespace[] = "\t\r\n\v ";
const char breaksymbols[] = "&|;()<>";

char *read_word(char *buf, int *shift);
char *read_special_symbol(char *buf, int *shift);
token_t *get_token(char *buf, int *shift);
token_list_t *merge_tokens(token_list_t *tklist);
#ifdef DEBUG
void print_tokens(token_t *tokens, int n);
#endif

void report_synthax_error(char *buf, int *shift) {
  if (*shift + 10 < COMMAND_MAX_S) {
    buf[*shift + 10] = '\0';
  }
  printf("Synthax error near -> %s\n", buf + *shift);
  exit(1);
}

int is_end_of_input(char *buf) {
  int buflen = strlen(buf);
  return !((buflen >= 2 && buf[buflen - 2] == '\\') ||
           (buflen >= 3 && buf[buflen - 2] == '&' && buf[buflen - 3] == '&') ||
           (buflen >= 3 && buf[buflen - 2] == '|' && buf[buflen - 3] == '|'));
}

int get_tokens(char *buf, int bufsize, token_t *tokens) {
  int n;
  token_t *token;
  char *bufp;
  int shift;
  int read;
  token_list_t *tklist = NULL;
  token_list_t *head;
  shift = 0;
  memset(buf, 0, bufsize);
  bufp = buf;

  for (;;) {
    fgets(bufp, bufsize, stdin);
    if (is_end_of_input(bufp)) {
      break;
    }
    read = strlen(bufp);
    if (bufp[read - 2] == '\\') {
      bufp[read - 1] = '\0';
      bufp[read - 2] = '\0';
      read -= 2;
    } else if (bufp[read - 2] == '&') {
      bufp[read - 1] = ' ';
    }
    bufp += read;
    bufsize -= read;
    printf("> ");
  }

  buf[strlen(buf) - 1] = 0;
  bufp = buf;
  n = 0;
  while ((token = get_token(buf, &shift)) != NULL) {
    if (tklist == NULL) {
      tklist = init(token);
      head = tklist;
    } else {
      add_item(head, token);
      head = head->next;
    }
    bufp += shift;
    n++;
  }
  tklist = merge_tokens(tklist);
  head = tklist;
  for (int i = 0; head != NULL; i++) {
    tokens[i] = *(token_t *)head->data;
    head = head->next;
  }
#ifdef DEBUG
  print_tokens(tokens, get_length(tklist));
#endif
  return n;
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
  token->type = TK_FD_NUMBER;
  token->data.fd_num.fd = *cur - '0';
  (*shift)++;
  return token;
}

token_t *get_token(char *buf, int *shift) {
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
    token->type = TK_SUBSH_OPEN;
    *shift += 1;
    break;
  case ')':
    token->type = TK_SUBSH_CLOSE;
    *shift += 1;
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
      puts("TK_REDIRECT Error! TODO: add buf_shift");
      exit(1);
    }
    curtk->data.redir.file = nexttk->data.word.str;
    remove_next(head);
    break;
  case TK_WORD:
    curtk->type = TK_CMD;
    curtk->data.cmd.head = curtk->data.word.str;
    curtk->data.cmd.parameters[0] = curtk->data.cmd.head;
    for (int i = 1; nexttk != NULL && nexttk->type == TK_WORD; i++) {
      curtk->data.cmd.parameters[i] = nexttk->data.word.str;
      remove_next(head);
      if (head->next == NULL) {
        break;
      }
      nexttk = head->next->data;
    }
    break;
  case TK_FD_NUMBER:
    if (nexttk == NULL || nexttk->type != TK_REDIRECT) {
      puts("TK_FD_NUMBER Error! TODO: add buf_shift");
      exit(1);
    }
    fd = curtk->data.fd_num.fd;
    curtk->type = nexttk->type;
    curtk->data = nexttk->data;
    curtk->data.redir.fd = fd;
    remove_next(head);
    merge_tokens(head);
    break;

  default:
    break;
  }
  merge_tokens(head->next);
  return head;
}

void print_tokens(token_t *tokens, int n) {
  token_t *tokenp;
  cmd_token_t cmd;
  puts("\n###########TOKENS###########");
  for (int i = 0; i < n; i++) {
    tokenp = tokens + i;
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
      printf("argv: ");
      for (int i = 0; cmd.parameters[i] != NULL; i++) {
        printf("%s, ", cmd.parameters[i]);
      }
      printf("NULL\n");
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
    case TK_PIPE:
      puts("type: PIPE");
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
