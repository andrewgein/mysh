#ifndef __HISTORY_H__
#define __HISTORY_H__

#define HIST_FILE ".mysh_history"
#define HIST_SIZE 80
#define HIST_LINE_MAX_LENGTH 80

typedef struct {
  char **lines;
  int cur;
  int length;
  int count;
  char *temp;
} cmd_history_t;

cmd_history_t history_init();
void history_cleanup(cmd_history_t *hist);
void load_history(cmd_history_t *hist);
void save_history(cmd_history_t *hist);
void add_command(cmd_history_t *hist, char *line);
void set_temp_command(cmd_history_t *hist, char *line);
char *next_command(cmd_history_t *hist);
char *prev_command(cmd_history_t *hist);

#endif
