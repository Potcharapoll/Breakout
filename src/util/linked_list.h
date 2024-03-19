#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include <stdio.h>
#include "types.h"

struct Node {
    u32 id;
    void *data;
    struct Node *prev;
    struct Node *next;
};

struct LinkedList {
    size_t data_size;
    size_t length;

    void (*free_node_callback)(void*, void**);
    struct Node *head;
};

struct LinkedList linked_list_init(size_t data_size, void (*free_node_callback)(void*,void**));
void linked_list_destroy(struct LinkedList *self);
void linked_list_append(struct LinkedList *self, void *data);
#endif
