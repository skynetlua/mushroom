#ifndef mr_slist_h
#define mr_slist_h

#include <stdlib.h>
#include <string.h>
#include "mr_config.h"

struct mr_slist_node
{
    struct mr_slist_node* next;
};

struct mr_slist 
{
    struct mr_slist_node head;
    struct mr_slist_node *tail;
};

#define mr_slist_is_empty(list) ((list)->head.next == NULL)

#define mr_slist_free(list) if((list) != NULL){FREE(list);}

#define mr_slist_link(list, node) \
    (list)->tail->next = (node);\
    (list)->tail = (node);\
    (node)->next = NULL

// static inline void mr_slist_free(struct mr_slist* list){
//     if (list != NULL) FREE(list);
// }

// static inline void mr_slist_link(struct mr_slist *list, struct mr_slist_node *node) {
//     list->tail->next = node;
//     list->tail = node;
//     node->next = NULL;
// }

static inline struct mr_slist* mr_slist_create(void) {
    struct mr_slist* list = (struct mr_slist*)MALLOC(sizeof(struct mr_slist));
    memset(list, 0, sizeof(struct mr_slist));
    return list;
}

static inline struct mr_slist_node* mr_slist_clear(struct mr_slist *list) {
    struct mr_slist_node * ret = list->head.next;
    list->head.next = NULL;
    list->tail = &(list->head);
    return ret;
}


#endif

