#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return; /* queue is NULL */
    element_t *del, *next;
    /* del, the element to be free. next, the next element of del */
    list_for_each_entry_safe (del, next, l, list) {
        list_del(&del->list);
        q_release_element(del);
    }
    free(l);
    return;
}
/* Return an element that contains string s.
 * Return NULL if could not allocate space.
 */
element_t *gen_element(char *s)
{
    element_t *e = malloc(sizeof(element_t));
    if (!e)
        return NULL; /* could not allocate space */
    int len = strlen(s) + 1;
    e->value = malloc(sizeof(char) * len);
    if (!e->value) {
        free(e);
        return NULL; /* could not allocate space */
    }
    memcpy(e->value, s, len);
    INIT_LIST_HEAD(&e->list);
    return e;
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false; /* queue is NULL or s is NULL*/
    element_t *e = gen_element(s);
    if (!e)
        return false;
    list_add(&e->list, head);
    return true;
}
/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)
        return false; /* queue is NULL or s is NULL*/
    element_t *e = gen_element(s);
    if (!e)
        return false;
    list_add_tail(&e->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *e = list_first_entry(head, element_t, list);
    list_del_init(head->next);
    if (sp)
        memcpy(sp, e->value, strlen(e->value) + 1);
    return e;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    return q_remove_head(head->prev->prev, sp, bufsize);
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    struct list_head *node;
    int count = 0;
    list_for_each (node, head)
        ++count;
    return count;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 * https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
 */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    struct list_head **indir = &head->next, *fast = head->next;
    for (; fast != head && fast->next != head; fast = fast->next->next)
        indir = &(*indir)->next;
    struct list_head *del_node = *indir;
    list_del_init(del_node);
    element_t *del_e = list_entry(del_node, element_t, list);
    q_release_element(del_e);
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 * https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
 */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;
    element_t *first_dup, *node, *next;
    char *str = malloc(sizeof(char) * 1024);
    list_for_each_entry_safe (node, next, head, list) {
        if (strcmp(str, node->value) == 0) {
            list_del_init(&node->list);
            q_release_element(node);
            if (strcmp(str, next->value) != 0) {
                /* delete the frist node that have duplicate string */
                first_dup = list_entry(next->list.prev, element_t, list);
                list_del_init(&first_dup->list);
                q_release_element(first_dup);
            }
        } else {
            memcpy(str, node->value, strlen(node->value) + 1);
        }
    }
    free(str);
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 * https://leetcode.com/problems/swap-nodes-in-pairs/
 */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *node, *safe;
    for (node = head->next, safe = node->next; node != head && safe != head;
         node = safe, safe = node->next) {
        safe = safe->next;
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->prev = node->next;
        node->next->next = node;
        safe->prev = node;
        node->next = safe;
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *node, *next;
    list_for_each_safe (node, next, head) {
        node->next = node->prev;
        node->prev = next;
    }
    node->next = node->prev;
    node->prev = next;
    return;
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head) {}
