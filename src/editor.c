#include "editor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "history.h"

#define E_MLLC_ERROR_MSG "editor: Malloc error"

#define BUF_INITIAL_CAP 80

#define KEY_ENTER 10
#define KEY_ESCAPE 27
#define KEY_ARROW_LEFT 37
#define KEY_ARROW_UP 38
#define KEY_ARROW_RIGHT 39
#define KEY_ARROW_DOWN 40
#define KEY_BACKSPACE 127

static struct termios term, oterm;

input_editor_t editor_init(char *prompt) {
  input_editor_t editor;
  editor.buf = calloc(BUF_INITIAL_CAP, sizeof(char *));
  if(editor.buf == NULL) {
    puts(E_MLLC_ERROR_MSG);
    exit(1);
  }
  editor.length = BUF_INITIAL_CAP;
  editor.cursor = 0;
  editor.prompt = prompt;
  return editor;
}

void editor_cleanup(input_editor_t *editor) {
  free(editor->buf);
  free(editor->prompt);
}

void reset(input_editor_t *editor) {
  memset(editor->buf, 0, BUF_INITIAL_CAP * sizeof(char *));
  editor->cursor = 0;
}

char get_char() {
  char c = 0;
  tcgetattr(0, &oterm);
  memcpy(&term, &oterm, sizeof(term));
  term.c_lflag &= ~(ICANON | ECHO);
  term.c_cc[VMIN] = 1;
  term.c_cc[VTIME] = 0;
  tcsetattr(0, TCSANOW, &term);
  c = getchar();
  tcsetattr(0, TCSANOW, &oterm);
  return c;
}

char is_key_pressed() {
  tcgetattr(0, &oterm);
  memcpy(&term, &oterm, sizeof(term));
  term.c_lflag &= ~(ICANON | ECHO);
  term.c_cc[VMIN] = 0;
  term.c_cc[VTIME] = 1;
  tcsetattr(0, TCSANOW, &term);
  char c = getchar();
  tcsetattr(0, TCSANOW, &oterm);
  if (c != -1)
    ungetc(c, stdin);
  return ((c != -1) ? 1 : 0);
}

char get_key() {
  char c = get_char();
  if (c != KEY_ESCAPE) {
    return c;
  }
  if (!is_key_pressed()) {
    return KEY_ESCAPE;
  }
  c = get_char();
  if (c == '[') {
    switch (get_char()) {
    case 'A':
      c = KEY_ARROW_UP;
      break;
    case 'B':
      c = KEY_ARROW_DOWN;
      break;
    case 'C':
      c = KEY_ARROW_RIGHT;
      break;
    case 'D':
      c = KEY_ARROW_LEFT;
      break;
    default:
      c = 0;
      break;
    }
  } else {
    c = 0;
  }
  if (c == 0) {
    while (is_key_pressed()) {
      get_char();
    }
  }
  return c;
}

char *readline(input_editor_t *editor) {
  char c;
  char *cmd;
  cmd_history_t hist = history_init();
  printf("%s", editor->prompt);
  for (;;) {
    switch (c = get_key()) {
    case KEY_ENTER:
      add_command(&hist, editor->buf);
      save_history(&hist);
      cmd = strdup(editor->buf);
      reset(editor);
      return cmd;
    case KEY_BACKSPACE:
      backspace(editor);
      break;
    case KEY_ARROW_LEFT:
      move_left(editor);
      break;
    case KEY_ARROW_RIGHT:
      move_right(editor);
      break;
    case KEY_ARROW_UP:
      set_temp_command(&hist, editor->buf);
      cmd = prev_command(&hist);
      if (cmd != NULL) {
        move_begin(editor);
        printf("\033[K");
        strcpy(editor->buf, cmd);
        printf("%s", editor->buf);
        editor->cursor = strlen(cmd);
      }
      break;
    case KEY_ARROW_DOWN:
      cmd = next_command(&hist);
      if (cmd != NULL) {
        move_begin(editor);
        printf("\033[K");
        strcpy(editor->buf, cmd);
        printf("%s", editor->buf);
        editor->cursor = strlen(cmd);
      }
      break;
    default:
      append(editor, c);
      break;
    }
  }
  return NULL;
}

void append(input_editor_t *editor, char c) {
  if (strlen(editor->buf) + 1 >= editor->length) {
    editor->length *= 2;
    char *res = realloc(editor->buf, editor->length);
    if(res == NULL) {
      free(editor->buf);
      puts(E_MLLC_ERROR_MSG);
      exit(1);
    }
    editor->buf = res;
  }
  memmove(editor->buf + editor->cursor + 1, editor->buf + editor->cursor,
          strlen(editor->buf) - editor->cursor + 1);
  editor->buf[editor->cursor] = c;
  int len = strlen(editor->buf);
  printf("\033[K");
  printf("%s", editor->buf + editor->cursor);
  editor->cursor++;
  printf("\033[%dD", len - editor->cursor - (editor->cursor == len));
}

void refresh(input_editor_t *editor) {
  printf("\r\x1b[K");
  printf("%s%s", editor->prompt, editor->buf);
  if (editor->cursor < editor->length) {
    printf("\r\x1b[%luC", strlen(editor->prompt) + editor->cursor);
  }
}

void move_left(input_editor_t *editor) {
  if (editor->cursor > 0) {
    editor->cursor--;
    printf("\033[%dD", 1);
  }
}

void move_right(input_editor_t *editor) {
  if (editor->cursor < strlen(editor->buf)) {
    editor->cursor++;
    printf("\033[%dC", 1);
  }
}

void move_begin(input_editor_t *editor) {
  if (editor->cursor != 0) {
    printf("\033[%dD", editor->cursor);
    editor->cursor = 0;
  }
}

void move_end(input_editor_t *editor) {
  printf("\033[%luC", editor->cursor - strlen(editor->buf));
  editor->cursor = strlen(editor->buf);
}

void backspace(input_editor_t *editor) {
  if (editor->cursor == 0) {
    return;
  }
  memmove(editor->buf + editor->cursor - 1, editor->buf + editor->cursor,
          editor->length - editor->cursor + 1);
  editor->cursor--;
  int len = strlen(editor->buf);
  printf("\033[%dD", 1);
  printf("\033[K");
  printf("%s", editor->buf + editor->cursor);
  printf("\033[%dD", len - editor->cursor - (editor->cursor == len));
}
