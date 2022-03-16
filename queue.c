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

/* Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));

    if (head) {
        INIT_LIST_HEAD(head);
    }

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *node, *safe;
    list_for_each_entry_safe (node, safe, l, list)
        q_release_element(node);

    free(l);
}

/* Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    // q is NULL or could not allocate space
    // if (!head || !node)
    //    return false;

    // see make test trace-10-robust.cmd

    if (!head)
        return false;

    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return false;

    // store string
    int len = strlen(s);
    node->value = malloc((len + 1) * sizeof(char));  // plus 1 because of '\0'
    if (!node->value) {
        free(node);
        return false;
    }
    strncpy(node->value, s, len);
    *(node->value + len) = '\0';  // add '\0' at last

    list_add(&node->list, head);

    return true;
}

/* Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    // q is NULL or could not allocate space
    // if (!head || !node)
    //    return false;

    if (!head)
        return false;

    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return false;

    // store string
    int len = strlen(s);
    node->value = malloc((len + 1) * sizeof(char));  // plus 1 because of '\0'
    if (!node->value) {
        free(node);
        return false;
    }
    strncpy(node->value, s, len);
    *(node->value + len) = '\0';  // add '\0' at last

    list_add_tail(&node->list, head);

    return true;
}

/* Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * Reference:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    // if queue is NULL or empty
    if (!head || list_empty(head))
        return NULL;

    element_t *node = list_entry(head->next, element_t, list);
    // copy the removed string to *sp
    if (sp) {
        strncpy(sp, node->value, bufsize - 1);
        *(sp + bufsize - 1) = '\0';  // sp[bufsize-1] = '\0';
    }

    list_del(head->next);

    return node;
}

/* Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    // if queue is NULL or empty
    if (!head || list_empty(head))
        return NULL;

    element_t *node = list_entry(head->prev, element_t, list);
    // copy the removed string to *sp
    if (sp) {
        strncpy(sp, node->value, bufsize - 1);
        *(sp + bufsize - 1) = '\0';  // sp[bufsize-1] = '\0';
    }

    list_del(head->prev);

    return node;
}

/* WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/* Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    struct list_head *node;
    int size = 0;

    if (head && !list_empty(head)) {
        list_for_each (node, head)
            size++;
    }

    return size;
}

/* Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    // if q is NULL or empty
    if (!head || list_empty(head))
        return false;

    int pos = q_size(head) / 2;

    struct list_head *node, *safe;
    int count = 0;
    list_for_each_safe (node, safe, head) {
        if (count == pos) {
            list_del(node);
            q_release_element(list_entry(node, element_t, list));
            break;
        }

        count++;
    }

    return true;
}

/* Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    // if q is NULL
    if (!head)
        return false;
    // if q is empty or has only one node
    if (list_empty(head) || list_is_singular(head))
        return true;

    struct list_head *node, *safe;
    bool dup = false;
    list_for_each_safe (node, safe, head) {
        element_t *node_val = list_entry(node, element_t, list);
        element_t *next_val = list_entry(node->next, element_t, list);

        if (node->next != head &&
            strcmp(node_val->value, next_val->value) == 0) {
            list_del(node);
            q_release_element(node_val);
            dup = true;
        } else if (dup) {
            list_del(node);
            q_release_element(node_val);
            dup = false;
        }
    }

    return true;
}

/* Attempt to swap every two adjacent nodes. */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/

    // if q is NULL or empty
    if (!head || list_empty(head))
        return;

    struct list_head *node;
    for (node = head->next; node != head && node->next != head;
         node = node->next) {
        struct list_head *next = node->next;

        list_del(node->next);
        list_add_tail(next, node);
    }
}

/* Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    // if q is NULL or empty
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        list_move(node, head);
    }
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    // if q is NULL, empty or has only one node
    if (!head || list_empty(head) || list_is_singular(head))
        return;
}