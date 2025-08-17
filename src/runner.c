#include "runner.h"
#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    for(int i = 0; cmd.parameters[i] != NULL; ++i) {
      cmd.parameters[i] = cmd.parameters[i + 1];
    }
    execvp(cmd.head, cmd.parameters);
  }
}

int run(ast_node_t *root) {
  if (root == NULL) {
    exit(0);
  }

  switch (root->type) {
    int lstatus, rstatus;
  case CMD:
    if (is_builtin(root->data.cmd.head)) {
      return run_builtin(root->data.cmd);
    }
    if (fork() == 0) {
      execvp(root->data.cmd.head, root->data.cmd.parameters);
    }
    waitpid(-1, &lstatus, 0);
    return lstatus;

  case AND_IF:
    lstatus = run(root->data.log.left);
    rstatus = run(root->data.log.right);
    return !(!lstatus && !rstatus);

  case OR_IF:
    if (fork() == 0) {
      run(root->data.log.left);
    }
    waitpid(-1, &lstatus, 0);
    if (lstatus != 0 && fork() == 0) {
      run(root->data.log.right);
    }
    waitpid(-1, &lstatus, 0);
    return !(!lstatus || !rstatus);

  default:
    puts("UNSUPPORTED !!");
    return 1;

  case SEMI:
    if (fork() == 0) {
      run(root->data.log.left);
    }
    waitpid(-1, &lstatus, 0);
    if (fork() == 0) {
      run(root->data.log.right);
    }
    waitpid(-1, &rstatus, 0);
    // TODO return??
  }
}
