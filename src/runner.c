#include "runner.h"
#include "lexer.h"
#include "parser.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
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
    for (int i = 0; cmd.parameters[i] != NULL; ++i) {
      cmd.parameters[i] = cmd.parameters[i + 1];
    }
    execvp(cmd.head, cmd.parameters);
  }
  return 0;
}

int run(ast_node_t *root) {
  if (root == NULL) {
    exit(0);
  }

  int lstatus, rstatus;
  int pipefd[2];
  int inputfd, outputfd;
  int stdinfd, stdoutfd;
  pid_t pid;
  redir_token_t info;
  FILE *memstream;
  size_t streamsz = MAX_PARAM_LEN;
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
      if (fork() == 0) {
        close(STDOUT_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        lstatus = run(root->data.pipe.left);
        exit(0);
      } else {
        close(STDIN_FILENO);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        rstatus = run(root->data.pipe.right);
        exit(0);
      }
    }
    waitpid(-1, &lstatus, 0);
    break;

  case AST_REDIRECT:
    info = root->data.redir.rdinfo;
    switch (info.type) {
    case RD_IN:
      if ((inputfd = open(info.file, O_RDONLY)) < 0) {
        printf("Cannot open file %s\n", info.file);
        exit(1);
      }
      if (info.fd == -1) {
        info.fd = STDIN_FILENO;
      }
      stdinfd = dup(info.fd);
      dup2(inputfd, info.fd);
      close(inputfd);
      lstatus = run(root->data.redir.left);
      dup2(stdinfd, info.fd);
      close(stdinfd);
      return lstatus;

    case RD_OUT:
      if ((outputfd = open(info.file, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) {
        printf("Cannot open file %s\n", info.file);
        exit(1);
      }
      if (info.fd == -1) {
        info.fd = STDOUT_FILENO;
      }
      stdoutfd = dup(info.fd);
      dup2(outputfd, info.fd);
      close(outputfd);
      lstatus = run(root->data.redir.left);
      dup2(stdoutfd, info.fd);
      close(stdoutfd);
      return lstatus;

    case APP_OUT:
      if ((outputfd = open(info.file, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) {
        printf("Cannot open file %s\n", info.file);
        exit(1);
      }
      if (info.fd == -1) {
        info.fd = STDOUT_FILENO;
      }
      stdoutfd = dup(info.fd);
      dup2(outputfd, info.fd);
      close(outputfd);
      lstatus = run(root->data.redir.left);
      dup2(stdoutfd, info.fd);
      close(stdoutfd);
      return lstatus;

    case RD_INOUT:
      if ((inputfd = open(info.file, O_CREAT | O_RDWR, 644)) < 0) {
        printf("Cannot open file %s\n", info.file);
        exit(1);
      }
      if (info.fd == -1) {
        info.fd = STDIN_FILENO;
      }
      stdinfd = dup(info.fd);
      dup2(inputfd, info.fd);
      close(inputfd);
      lstatus = run(root->data.redir.left);
      dup2(stdinfd, info.fd);
      close(stdinfd);
      return lstatus;
    }
    break;

  case AST_CMDSUB:
    pipe(pipefd);
    if (fork() == 0) {
      close(STDOUT_FILENO);
      close(pipefd[0]);
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[1]);
      lstatus = run(root->data.cmdsub.cmd);
      exit(0);
    }
    close(pipefd[1]);
    size_t size = 0;
    char chunk[1024];
    waitpid(-1, &lstatus, 0);
    size = read(pipefd[0], root->data.cmdsub.result, streamsz);
    close(pipefd[0]);
    root->data.cmdsub.result[size - 1] = '\0';
    rstatus = run(root->data.cmdsub.next);
    return lstatus;

  default:
    printf("runner: Unsupported operation %d\n", root->type);
    return 1;
  }
}
