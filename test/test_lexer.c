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

#define get_tp(tklist,i) ((token_t*)(get_item(tklist,i)->data))

TEST("basic command") {
  char cmd[80] = "echo foo\n";
  int pipefd[2];

  pipe(pipefd);
  if (fork() == 0) {
    dup2(pipefd[0], STDIN_FILENO);

    int n;
    char buf[80];
    token_list_t *tklist = get_tokens();
    token_t *cmd = get_tp(tklist, 0);
    token_list_t *parameters = cmd->data.cmd.parameters;
    ASSERT(cmd->type == TK_CMD);
    ASSERT(strcmp(get_tp(tklist, 0)->data.cmd.head, "echo") == 0);
    ASSERT(strcmp(get_tp(parameters, 0)->data.word.str, "echo") == 0);
    ASSERT(strcmp(get_tp(parameters, 1)->data.word.str, "foo") == 0);
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

    char buf[80];
    token_list_t *tklist = get_tokens();
    ASSERT(get_length(tklist) == 5);
    ASSERT(get_tp(tklist, 0)->type == TK_CMD);
    ASSERT(get_tp(tklist, 1)->type == TK_AND_IF);
    ASSERT(get_tp(tklist, 2)->type == TK_CMD);
    ASSERT(get_tp(tklist, 3)->type == TK_OR_IF);
    ASSERT(get_tp(tklist, 4)->type == TK_CMD);
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

    char buf[80];
    token_t *tokens = calloc(80, sizeof(token_t));
    token_list_t *tklist = get_tokens();
    ASSERT(get_length(tklist) == 7);
    ASSERT(get_tp(tklist, 0)->type == TK_SUBSH_OPEN);
    ASSERT(get_tp(tklist, 1)->type == TK_CMD);
    ASSERT(get_tp(tklist, 2)->type == TK_AND_IF);
    ASSERT(get_tp(tklist, 3)->type == TK_CMD);
    ASSERT(get_tp(tklist, 4)->type == TK_SUBSH_CLOSE);
    ASSERT(get_tp(tklist, 5)->type == TK_SEMI);
    ASSERT(get_tp(tklist, 6)->type == TK_CMD);
    exit(0);
  }

  close(pipefd[0]);
  write(pipefd[1], cmd, strlen(cmd) + 1);
  close(pipefd[1]);
  waitpid(-1, NULL, 0);
}

TEST("basic cmdsub") {
  char cmd[80] = "echo $(echo $(echo foo))\n";
  int pipefd[2];

  pipe(pipefd);
  if (fork() == 0) {
    dup2(pipefd[0], STDIN_FILENO);

    char buf[80];
    token_t *tokens = calloc(80, sizeof(token_t));
    token_list_t *tklist = get_tokens();
    ASSERT(get_length(tklist) == 1);

    ASSERT(get_tp(tklist, 0)->type == TK_CMD);
    token_list_t *parameters = get_tp(tklist, 0)->data.cmd.parameters;
    ASSERT(get_tp(parameters, 0)->type == TK_WORD);
    ASSERT(get_tp(parameters, 1)->type == TK_CMD_SUB_OPEN);
    ASSERT(get_tp(parameters, 2)->type == TK_CMD);
    ASSERT(get_tp(parameters, 3)->type == TK_CMD_SUB_CLOSE);

    parameters = get_tp(parameters, 2)->data.cmd.parameters;
    ASSERT(get_tp(parameters, 0)->type == TK_WORD);
    ASSERT(get_tp(parameters, 1)->type == TK_CMD_SUB_OPEN);
    ASSERT(get_tp(parameters, 2)->type == TK_CMD);
    ASSERT(get_tp(parameters, 3)->type == TK_CMD_SUB_CLOSE);
    exit(0);
  }

  close(pipefd[0]);
  write(pipefd[1], cmd, strlen(cmd) + 1);
  close(pipefd[1]);
  waitpid(-1, NULL, 0);
}

#endif
