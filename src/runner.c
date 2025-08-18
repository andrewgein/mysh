#include "runner.h"
#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int is_builtin(char *cmd) {
  char *builtins[] = {"cd", "exit", "exec"};
  for (int i = 0; i < sizeof(builtins) / sizeof(builtins[0]); i++) {
    if (strcmp(cmd, builtins[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

int run_builtin(ast_cmd_t cmd) {
  if (strcmp(cmd.head, "exit") == 0) {
    exit(0);
  } else if (strcmp(cmd.head, "cd") == 0) {
    if (cmd.parameters[2] != NULL) {
      puts("cd: too many arguments");
      return 1;
    }
    return chdir(cmd.parameters[1]);
  } else if (strcmp(cmd.head, "exec") == 0) {
    cmd.head = cmd.parameters[1];
    for (int i = 0; cmd.parameters[i] != NULL; ++i) {
      cmd.parameters[i] = cmd.parameters[i + 1];
    }
    execvp(cmd.head, cmd.parameters);
  }
}

int run(ast_node_t *root) {
  if (root == NULL) {
    exit(0);
  }

  int lstatus, rstatus;
  int pipefd[2];
  pid_t pid;
  switch (root->type) {

  case AST_CMD:
    if (is_builtin(root->data.cmd.head)) {
      return run_builtin(root->data.cmd);
    }
    if (fork() == 0) {
      execvp(root->data.cmd.head, root->data.cmd.parameters);
    }
    waitpid(-1, &lstatus, 0);
    return lstatus;

  case AST_AND_IF:
    lstatus = run(root->data.log.left);
    if (lstatus == 0) {
      rstatus = run(root->data.log.right);
      return rstatus;
    }
    return lstatus;

  case AST_OR_IF:
    lstatus = run(root->data.log.left);
    if (lstatus != 0) {
      rstatus = run(root->data.log.right);
      return rstatus;
    }
    return lstatus;

  case AST_SEMI:
    lstatus = run(root->data.log.left);
    rstatus = run(root->data.log.right);
    return rstatus;

  case AST_SUBSH:
    if (fork() == 0) {
      run(root->data.subsh.content);
      exit(0);
    }
    waitpid(-1, &lstatus, 0);
    return lstatus;

  case AST_PIPE:
    pipe(pipefd);
    if (fork() == 0) {
      if(fork() == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        lstatus = run(root->data.pipe.left);
        exit(0);
      } else {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        rstatus = run(root->data.pipe.right);
        exit(0);
      }
    }
    waitpid(-1, &lstatus, 0);
    break;

  default:
    printf("runner: Unsupported operation %d\n", root->type);
    return 1;
  }
}
