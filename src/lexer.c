#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char whitespace[] = "\t\r\n\v ";
const char breaksymbols[] = "&|;";

char *read_next(char *buf, int *shift);
char *read_word(char *buf, int *shift);
char *read_special_symbol(char *buf, int *shift);
token_t *get_token(char *buf, int *shift);
#ifdef DEBUG
void print_tokens(token_t *tokens, int n);
#endif

int get_tokens(char *buf, int bufsize, token_t *tokens) {
  int n;
  token_t *token;
  char *bufp;
  int shift;
  shift = 0;
  memset(buf, 0, bufsize);
  fgets(buf, bufsize, stdin);
  if (buf[0] == 0)
    return -1;
  buf[strlen(buf) - 1] = 0;
  bufp = buf;
  n = 0;
  while ((token = get_token(buf, &shift)) != NULL) {
    bufp += shift;
    tokens[n++] = *token;
  }
#ifdef DEBUG
  print_tokens(tokens, n);
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

char *read_next(char *buf, int *shift) {
  char *bufp;
  char *bufendp;
  char *tokenstr;
  char *tokenstartp;
  char *tokenendp;

  skip_whitespaces(buf, shift);

  bufp = buf + *shift;
  bufendp = buf + strlen(buf);
  if(bufp == bufendp) {
    return NULL;
  }
  tokenstartp = bufp;
  *shift = bufp - buf;
  if (strchr(breaksymbols, *tokenstartp)) {
    return read_special_symbol(buf, shift);
  }
  return read_word(buf, shift);
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

char *read_special_symbol(char *buf, int *shift) {
  char *bufp = buf + *shift;
  char *bufendp = buf + strlen(buf);
  char *token;

  if (*bufp == ';') {
    *shift += 1;
    token = malloc(sizeof(char) * 2);
    token = ";";
    return token;
  }

  if (bufp + 1 == bufendp) {
    puts("Synthax error!");
    exit(1);
  }

  token = malloc(sizeof(char) * 3);
  if (*bufp == '&' && *(bufp + 1) == '&') {
    *shift += 2;
    token = "&&";
  } else if (*bufp == '|' && *(bufp + 1) == '|') {
    *shift += 2;
    token = "||";
  } else {
    puts("Synthax error!");
    exit(1);
  }
  return token;
}

token_t *get_token(char *buf, int *shift) {
  token_t *token = (token_t *)malloc(sizeof(token_t));
  char *tokenstr = read_next(buf, shift);
  //printf("tokenstr: %s\n", tokenstr);
  if (tokenstr == NULL) {
    return NULL;
  }

  if (strchr("|&;", tokenstr[0])) {
    switch (tokenstr[0]) {
    case '|':
      if (strlen(tokenstr) == 2) {
        token->type = OR_IF;
        return token;
      } else {
        puts("Synthax error!");
        exit(1);
      }
      break;
    case '&':
      if (strlen(tokenstr) == 2) {
        token->type = AND_IF;
        return token;
      } else {
        puts("Synthax error!");
        exit(1);
      }
      break;
    case ';':
      token->type = SEMI;
      break;
    }
  } else {
    int argc = 1;
    token = malloc(sizeof(token_t));
    token->type = CMD;
    token->data.cmd.head = tokenstr;
    token->data.cmd.parameters[0] = tokenstr;
    skip_whitespaces(buf, shift);
    while ((tokenstr = read_word(buf, shift)) != NULL) {
      token->data.cmd.parameters[argc++] = tokenstr;
      skip_whitespaces(buf, shift);
    }
    token->data.cmd.parameters[argc] = NULL;
  }
  return token;
}

void print_tokens(token_t *tokens, int n) {
  token_t *tokenp;
  cmd_token_t cmd;
  puts("\n###########TOKENS###########");
  for (int i = 0; i < n; i++) {
    tokenp = tokens + i;
    printf("%d\n", i);
    switch (tokenp->type) {
    case CMD:
      cmd = tokenp->data.cmd;
      puts("type: command");
      printf("head: %s\n", cmd.head);
      printf("argv: ");
      for (int i = 0; cmd.parameters[i] != NULL; i++) {
        printf("%s, ", cmd.parameters[i]);
      }
      printf("NULL\n");
      break;
    case SEMI:
      puts("type: SEMI");
      break;
    case OR_IF:
      puts("type: OR_IF");
      break;
    case AND_IF:
      puts("type: AND_IF");
      break;
    defaults:
      puts("type: UNKNOWN");
    }
    puts("");
  }
}
