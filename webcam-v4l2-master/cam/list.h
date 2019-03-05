#ifndef _LIST_H_
#define _LIST_H_

struct list_head
{
    struct list_head *next;
    struct list_head *prev;
};

void list_head_init(struct list_head *list);
void list_add_tail(struct list_head *_new, struct list_head *head);
void list_del(struct list_head *entry);

#ifndef offsetof
#define offsetof(TYPE, MEMBER) \
    ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
#define container_of(ptr, type, member) \
    ((type *)((char *)ptr - offsetof(type,member)))
#endif

/**
 * list_entry - get the struct for this entry
 * @ptr:    the &struct list_head pointer.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:    the &struct list_head to use as a loop cursor.
 * @n:        another &struct list_head to use as temporary storage
 * @head:    the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)

#endif //_LIST_H_
