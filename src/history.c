#include "history.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define H_MLLC_ERROR_MSG "history: Malloc error"

cmd_history_t history_init() {
  cmd_history_t hist;
  hist.lines = malloc(sizeof(char *) * HIST_SIZE);
  if (hist.lines == NULL) {
    puts(H_MLLC_ERROR_MSG);
    exit(1);
  }
  hist.cur = 0;
  hist.length = HIST_SIZE;
  hist.count = 0;
  hist.temp = NULL;
  load_history(&hist);
  return hist;
}

void history_cleanup(cmd_history_t *hist) {
  free(hist->lines);
  if(hist->temp != NULL) {
    free(hist->temp);
  }
}

void load_history(cmd_history_t *hist) {
  FILE *file = fopen(HIST_FILE, "r");
  if (!file) {
    puts("Failed to load history");
    return;
  }
  char line[HIST_LINE_MAX_LENGTH];
  int len = 0;
  while (fgets(line, sizeof(line), file)) {
    len = strlen(line);
    if (len > 0 && len[line - 1] == '\n') {
      line[len - 1] = '\0';
    }
    if (strlen(line) > 0) {
      add_command(hist, line);
    }
    hist->cur++;
  }
  fclose(file);
}

void save_history(cmd_history_t *hist) {
  FILE *file = fopen(HIST_FILE, "w");
  // TODO limit hist file size??
  for (int i = 0; i < hist->count; i++) {
    fprintf(file, "%s\n", hist->lines[i]);
  }
  fclose(file);
}

void add_command(cmd_history_t *hist, char *line) {
  if (hist->count >= hist->length) {
    hist->length *= 2;
    char **res = realloc(hist->lines, hist->length * sizeof(char *));
    if (res == NULL) {
      free(hist->lines);
      puts(H_MLLC_ERROR_MSG);
      exit(1);
    }
    hist->lines = res;
  }
  hist->cur = hist->count;
  hist->lines[hist->count++] = strdup(line);
}

void set_temp_command(cmd_history_t *hist, char *line) {
  if (hist->temp == NULL) {
    hist->temp = strdup(line);
  }
}

char *next_command(cmd_history_t *hist) {
  if (hist->count == 0) {
    return NULL;
  }
  if (hist->cur < hist->count - 1) {
    hist->cur++;
    return hist->lines[hist->cur];
  }
  hist->cur = hist->count;
  if (hist->temp != NULL) {
    return hist->temp;
  }
  return strdup("");
}

char *prev_command(cmd_history_t *hist) {
  if (hist->count == 0) {
    return NULL;
  }
  if (hist->cur == 0) {
    return hist->lines[hist->cur];
  }
  return hist->lines[--hist->cur];
}
