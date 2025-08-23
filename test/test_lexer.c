#if defined HEADERS
#include "lexer.c"
#include "list.c"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#elif defined TESTS
TEST("basic command") {
  char cmd[80] = "echo foo\n";
  int pipefd[2];

  pipe(pipefd);
  if (fork() == 0) {
    dup2(pipefd[0], STDIN_FILENO);

    int n;
    char buf[80];
    token_t *tokens = calloc(80, sizeof(token_t));
    n = get_tokens(buf, 80, tokens);
    ASSERT(n == 1);
    ASSERT(tokens[0].type == TK_CMD);
    ASSERT(strcmp(tokens[0].data.cmd.head, "echo") == 0);
    ASSERT(strcmp(tokens[0].data.cmd.parameters[0], "echo") == 0);
    ASSERT(strcmp(tokens[0].data.cmd.parameters[1], "foo") == 0);
    exit(0);
  }

  close(pipefd[0]);
  write(pipefd[1], cmd, strlen(cmd) + 1);
  close(pipefd[1]);
  waitpid(-1, NULL, 0);
}

TEST("basic log") {
  char cmd[80] = "false && echo foo || echo bar\n";
  int pipefd[2];

  pipe(pipefd);
  if (fork() == 0) {
    dup2(pipefd[0], STDIN_FILENO);

    int n;
    char buf[80];
    token_t *tokens = calloc(80, sizeof(token_t));
    n = get_tokens(buf, 80, tokens);
    ASSERT(n == 5);
    ASSERT(tokens[0].type == TK_CMD);
    ASSERT(tokens[1].type == TK_AND_IF);
    ASSERT(tokens[2].type == TK_CMD);
    ASSERT(tokens[3].type == TK_OR_IF);
    ASSERT(tokens[4].type == TK_CMD);
    exit(0);
  }

  close(pipefd[0]);
  write(pipefd[1], cmd, strlen(cmd) + 1);
  close(pipefd[1]);
  waitpid(-1, NULL, 0);
}

TEST("basic subshell") {
  char cmd[80] = "(cd /tmp && pwd); pwd\n";
  int pipefd[2];

  pipe(pipefd);
  if (fork() == 0) {
    dup2(pipefd[0], STDIN_FILENO);

    int n;
    char buf[80];
    token_t *tokens = calloc(80, sizeof(token_t));
    n = get_tokens(buf, 80, tokens);
    ASSERT(n == 7);
    ASSERT(tokens[0].type == TK_SUBSH_OPEN);
    ASSERT(tokens[1].type == TK_CMD);
    ASSERT(tokens[2].type == TK_AND_IF);
    ASSERT(tokens[3].type == TK_CMD);
    ASSERT(tokens[4].type == TK_SUBSH_CLOSE);
    ASSERT(tokens[5].type == TK_SEMI);
    ASSERT(tokens[6].type == TK_CMD);
    exit(0);
  }

  close(pipefd[0]);
  write(pipefd[1], cmd, strlen(cmd) + 1);
  close(pipefd[1]);
  waitpid(-1, NULL, 0);
}


#endif
