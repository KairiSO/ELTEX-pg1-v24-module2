#ifndef CONTACT_LIST_H
#define CONTACT_LIST_H

#include "contact.h"

typedef struct ContactNode
{
    Contact *contact;

    struct ContactNode *prev;
    struct ContactNode *next;
} ContactNode;

ContactNode *list_insert_ordered(ContactNode **head, Contact *contact);

ContactNode *list_find(ContactNode *head, int id);

/* Возвращает количество узлов в списке. */
int list_count(const ContactNode *head);

/* Возвращает узел с индексом index (0-based) или NULL, если index вне диапазона. */
ContactNode *list_get_at(const ContactNode *head, int index);

void list_remove(ContactNode **head, int id, int free_contact);

void list_clear(ContactNode *head);

void list_print(const ContactNode *head);

void print_optional_field(const char *label, const char *value);

#endif