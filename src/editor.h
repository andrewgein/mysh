#ifndef __EDITOR_H__
#define __EDITOR_H__

#include <stdlib.h>

typedef struct {
  char *buf;
  int length;
  int cursor;
  char *prompt;

} input_editor_t;

input_editor_t editor_init(char *prmopt);
void editor_cleanup(input_editor_t *editor);
char *readline(input_editor_t *editor);
void append(input_editor_t *editor, char c);
void refresh(input_editor_t *editor);
void move_left(input_editor_t *e);
void move_right(input_editor_t *e);
void move_begin(input_editor_t *e);
void move_end(input_editor_t *e);
void backspace(input_editor_t *e);

#endif
