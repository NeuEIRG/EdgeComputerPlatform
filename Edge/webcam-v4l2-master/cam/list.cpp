#include "list.h"
#include <stdio.h>

static void __list_add(struct list_head *_new, struct list_head *prev, struct list_head *next)
{
    next->prev = _new;
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}

static void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

void list_head_init(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

/**
 * list_add_tail - insert a new entry before the specified head
 * @_new: new entry to be added
 * @head: list head to add it before
 */
void list_add_tail(struct list_head *_new, struct list_head *head)
{
    __list_add(_new, head->prev, head);
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 */
void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}
