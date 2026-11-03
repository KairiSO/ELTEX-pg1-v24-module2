#include <stdio.h>
#include <stdlib.h>

#include "contact_list.h"

ContactNode *list_insert_ordered(ContactNode **head, Contact *contact)
{
    if (!head || !contact)
    {
        return NULL;
    }

    ContactNode *node = malloc(sizeof(*node));

    if (!node)
    {
        return NULL;
    }

    node->contact = contact;
    node->prev = NULL;
    node->next = NULL;

    ContactNode *prev = NULL;
    ContactNode *current = *head;

    while (current && contact_compare(current->contact, contact) <= 0)
    {
        prev = current;
        current = current->next;
    }

    node->prev = prev;
    node->next = current;

    if (prev)
    {
        prev->next = node;
    }
    else
    {
        *head = node;
    }

    if (current)
    {
        current->prev = node;
    }

    return node;
}

ContactNode *list_find(ContactNode *head, int id)
{
    for (ContactNode *node = head; node; node = node->next)
    {
        if (node->contact && node->contact->id == id)
        {
            return node;
        }
    }

    return NULL;
}

void list_remove(ContactNode **head, int id, int free_contact)
{
    if (!head)
    {
        return;
    }

    ContactNode *node = list_find(*head, id);

    if (!node)
    {
        return;
    }

    if (node->prev)
    {
        node->prev->next = node->next;
    }
    else
    {
        *head = node->next;
    }

    if (node->next)
    {
        node->next->prev = node->prev;
    }

    if (free_contact)
    {
        contact_free(node->contact);
    }

    free(node);
}

void list_clear(ContactNode *head)
{
    while (head)
    {
        ContactNode *next = head->next;

        contact_free(head->contact);
        free(head);

        head = next;
    }
}

void print_optional_field(const char *label, const char *value)
{
    if (value && value[0])
    {
        printf("  %s: %s\n", label, value);
    }
}

void list_print(const ContactNode *head)
{
    if (!head)
    {
        puts("Список контактов пуст.");
        return;
    }

    for (const ContactNode *node = head; node; node = node->next)
    {
        const Contact *contact = node->contact;

        printf("ID %d: %s %s",
               contact->id,
               contact->last_name ? contact->last_name : "",
               contact->first_name ? contact->first_name : "");

        if (contact->middle_name && contact->middle_name[0])
        {
            printf(" %s", contact->middle_name);
        }

        putchar('\n');

        print_optional_field("Место работы", contact->company);
        print_optional_field("Должность", contact->position);
        print_optional_field("Телефон", contact->phone);
        print_optional_field("E-mail", contact->email);
        print_optional_field("Соцсети", contact->social);
        print_optional_field("Мессенджеры", contact->messenger);

        putchar('\n');
    }
}

int list_count(const ContactNode *head)
{
    int count = 0;
    for (const ContactNode *node = head; node; node = node->next)
    {
        ++count;
    }
    return count;
}

ContactNode *list_get_at(const ContactNode *head, int index)
{
    if (index < 0)
    {
        return NULL;
    }

    const ContactNode *node = head;
    for (int i = 0; i < index && node; ++i)
    {
        node = node->next;
    }

    return (ContactNode *)node;
}