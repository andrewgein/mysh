#include "list.h"

#include <stdlib.h>

list_node_t *init(void *data) {
  list_node_t *head = malloc(sizeof(data));
  head->data = data;
  head->next = NULL;
  return head;
}

void add_item(list_node_t *head, void *data) {
  list_node_t *next = init(data);
  head->next = next;
}

void destroy_next(list_node_t *item) {
  list_node_t *l = item->next;
  item->next = item->next->next; 
  free(l);
}

void remove_next(list_node_t *item) {
  item->next = item->next->next; 
}

int get_length(list_node_t *head) {
  int i;
  for (i = 0; head != NULL; i++) {
    head = head->next;
  }
  return i;
}
