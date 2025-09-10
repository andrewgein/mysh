#include "list.h"

#include <stdlib.h>

list_node_t *init(void *data) {
  list_node_t *head = malloc(sizeof(data));
  head->data = data;
  head->next = NULL;
  return head;
}

void insert(list_node_t *head, void *data) {
  list_node_t *next = init(data);
  head->next = next;
}

void push_back(list_node_t *head, void *data) { insert(get_last(head), data); }

void destroy_next(list_node_t *item) {
  list_node_t *l = item->next;
  item->next = item->next->next;
  destroy(l);
}

void destroy(list_node_t *item) {
  free(item->data);
  free(item);
}

void destroy_all(list_node_t *item) {
  while(item->next != NULL) {
    destroy_next(item);
  }
  destroy(item);
}

void remove_next(list_node_t *item) {
  if (item->next != NULL) {
    item->next = item->next->next;
  }
}

int get_length(list_node_t *head) {
  int i = 0;
  for (; head != NULL; i++) {
    head = head->next;
  }
  return i;
}

list_node_t *get_item(list_node_t *head, int n) {
  for (int i = 0; i < n; i++) {
    if (head != NULL) {
      head = head->next;
    } else {
      return NULL;
    }
  }
  return head;
}

list_node_t *get_last(list_node_t *head) {
  return get_item(head, get_length(head) - 1);
}
