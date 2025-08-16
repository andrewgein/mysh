#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

//#define DEBUG

#define COMMAND_MAX_S     80
#define MAX_PARAMS        20
#define PARAM_MAX_S       80

typedef struct{
  char *head;
  char *parameters[MAX_PARAMS];
} cmd_t;

const char whitespace[] = "\t\r\n\v ";

void type_prompt() {
  char *prompt;
  prompt = getenv("PS1");
  #ifdef DEBUG
    printf("[test] ");
  #endif
  if(prompt != NULL)
    printf("%s", prompt);
  else
    if(getuid() == 0)
      printf("# ");
    else
      printf("$ ");
}

void execute(cmd_t *command) {
  execvp(command->head, command->parameters);
}

char *get_token(char *buf, int *shift) {
  char *bufp;
  char *bufendp;
  char *token;
  char *tokenstartp;
  char *tokenendp;
  bufp = buf;
  bufendp = buf + strlen(buf);
  while((bufp < bufendp) && (strchr(whitespace, *bufp) != NULL))
    bufp++;
  tokenstartp = bufp;
  while((bufp < bufendp) && (strchr(whitespace, *bufp) == NULL))
    bufp++;
  tokenendp = bufp;
  if(tokenstartp == tokenendp)
    return NULL;
  token = malloc(sizeof(char) * (tokenendp - tokenstartp + 1));
  memcpy(token, tokenstartp, (tokenendp - tokenstartp));
  token[tokenendp - tokenstartp] = 0;
  *shift = bufp - buf;
  return token;
}

int read_command(char *buf, int bufsize, cmd_t *command) {
  int n;
  char *token;
  char *bufp;
  int shift;
  command->head = "";
  memset(buf, 0, bufsize);
  fgets(buf, bufsize, stdin);
  if(buf[0] == 0)
    return -1;
  buf[strlen(buf) - 1] = 0;
  bufp = buf;
  n = 0;
  while((token = get_token(bufp, &shift)) != NULL) {
    if(n == 0)
      command->head = token;
    command->parameters[n++] = token;
    bufp += shift;
  }
  command->parameters[n] = NULL;
  return n;
}

int main(int argc, char **argv) {
  int status;
  static cmd_t command;
  static char buf[MAX_PARAMS*(PARAM_MAX_S + 1)];
  type_prompt();
  while(read_command(buf, sizeof(buf), &command) >= 0) {
    if(strcmp(command.head, "exit") == 0) {
      return 0;
    }
    if(strcmp(command.head, "cd") == 0) {
      chdir(command.parameters[1]);
    }

    #ifdef DEBUG
      printf("\"%s\" argv: ", command.head);
      for(int i = 0; command.parameters[i] != NULL; i++) {
        printf("%s, ", command.parameters[i]);
      }
      printf("NULL\n");
    #endif

    if(fork() == 0) {
      execute(&command);
      if(errno == ENOENT) {
        printf("mysh: Command not found: %s\n", command.head);
        return 1;
      }
    }

    waitpid(-1, &status, 0);
    type_prompt();
  }
  return 0;
}
