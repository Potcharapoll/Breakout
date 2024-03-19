#include "linked_list.h"

#include <malloc.h>
#include <string.h>

struct LinkedList linked_list_init(size_t data_size, void (*free_node_callback)(void*,void**)) {
    struct LinkedList new_list = {
        .data_size = data_size,
        .free_node_callback = free_node_callback,
        .head = NULL,
        .length = 0
    };
    return new_list;
}

void linked_list_destroy(struct LinkedList *self) {
    while (self->head != NULL) {
        struct Node *node = self->head;
        self->head = self->head->next;
        free(node->data); 
        free(node);
    }
    self->head = NULL;
}

void linked_list_append(struct LinkedList *self, void *data) {
    struct Node *node = malloc(sizeof(*node));
    node->data = malloc(self->data_size);
    node->id = self->length;
    self->length++;
    memcpy(node->data, data, self->data_size);

    node->next = NULL;
    node->prev = NULL;

    if (self->head == NULL) {
        self->head = node;
        return;
    }

    struct Node *curr = self->head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = node;
    node->prev = curr;
}

