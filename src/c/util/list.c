#include <stdlib.h>
#include <stdarg.h>

#include "list.h"
#include "list_node.h"

struct list {
    struct list_node *front_sentinel;
    struct list_node *last;
    void (*free_element)(void*);
    struct list_node *cursor;
    size_t size;
    unsigned int curpos;
    unsigned int flags;
};

static struct list_node *list_get_node(const struct list *list, unsigned int n)
{
    unsigned int k = n;
    struct list_node *node = list->front_sentinel;
    if (list->curpos <= k) {
	k -= list->curpos;
	node = list->cursor;
    }
    for (unsigned int i = 0; i < k; i++)
	node = node_get_next(node);
    ((struct list*)list)->cursor = node;
    ((struct list*)list)->curpos = n;
    return node;
}

size_t list_size(const struct list *list)
{
    return list->size;
}

struct list *list_new(int flags, ...)
{
    struct list *list = calloc(sizeof(*list), 1);
    list->front_sentinel = node_new(NULL, SENTINEL_NODE);
    node_set_next(list->front_sentinel, node_new(NULL, 1));
    list->flags = flags;
    list->cursor = list->front_sentinel;
    list->curpos = 0;
    list->size = 0;

    va_list ap;
    va_start(ap, flags);
    if ((flags & LI_FREE) != 0)
	list->free_element = va_arg(ap, void(*)(void*));
    if ((flags & LI_ELEM) != 0) {
	void *arg;
	do {
	    arg = va_arg(ap, void*);
	    if (arg != NULL) list_append(list, arg);
	} while (NULL != arg);
    }

    return list;
}

void list_free(struct list *list)
{
    struct list_node *node = node_get_next(list->front_sentinel);
    struct list_node *tmp = NULL;
    while (!node_is_sentinel(node)) {
	tmp = node_get_next(node);
	if ((list->flags & LI_FREE) != 0)
	    list->free_element((void*) node_get_data(node));
	node_free(node);
	node = tmp;
    }
    node_free(node); //the back sentinel
    node_free(list->front_sentinel);
    free(list);
}

void *list_get(const struct list *list, unsigned int n)
{
    return (void*) node_get_data(list_get_node(list, n));
}

void list_add(struct list *list, const void *element)
{
    struct list_node *tmp = node_new(element, 0);
    node_set_next(tmp, node_get_next(list->front_sentinel));
    node_set_next(list->front_sentinel, tmp);
    list->size ++;
}

void list_append(struct list *list, const void *element)
{
    int n = list_size(list) + 1;
    list_insert(list, n, element);
}

void list_append_list(struct list *l1, const struct list *l2)
{
    for (unsigned int i = 1; i <= list_size(l2); ++i)
	list_append(l1, list_get(l2, i));
}

void list_insert(struct list *list, unsigned int n, const void *element)
{
    struct list_node *previous = list_get_node(list, n-1);
    struct list_node *tmp = node_new(element, 0);
    node_set_next(tmp, node_get_next(previous));
    node_set_next(previous, tmp);
    list->size ++;
}

void list_remove(struct list *list, unsigned int n)
{
    struct list_node *previous = list_get_node(list, n-1);
    struct list_node *tmp = node_get_next(previous);
    node_set_next(previous, node_get_next(tmp));
    if ((list->flags & LI_FREE) != 0)
	list->free_element((void*)node_get_data(tmp));
    node_free(tmp);
    list->size --;
}

struct list *list_copy(const struct list *l)
{
    struct list *n = list_new(0);
    list_append_list(n, l);
    return n;
}

void *list_to_array(const struct list *l)
{
    void **array = malloc(sizeof(*array) * l->size);
    for (int i = 0; i < l->size; ++i)
	array[i] = list_get(l, i);

    return array;
}

struct hash_table *list_to_hashtable(const struct list *l,
				     const char *(*element_keyname) (void *))
{
    struct hash_table *ht = ht_create(2 * l->size, NULL);

    for (int i = 0; i < l->size; ++i) {
	void *el = list_get(l, i);
	ht_add_entry(ht, element_keyname(el), el);
    }
    return ht;
}
