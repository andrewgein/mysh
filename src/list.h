#ifndef __LIST_H__
#define __LIST_H__

typedef struct list_node {
  void *data;
  struct list_node *next;
} list_node_t;

list_node_t *init(void *data);
void insert(list_node_t *head, void *data);
void push_back(list_node_t *head, void *data);
void remove_next(list_node_t *item);
void destroy_all(list_node_t *item);
void destroy_next(list_node_t *item);
void destroy(list_node_t *item);
int get_length(list_node_t *head);
list_node_t *get_item(list_node_t *head, int n);
list_node_t *get_last(list_node_t *head);

#endif
