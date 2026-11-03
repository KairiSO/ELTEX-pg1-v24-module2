#include <stdio.h>
#include <stddef.h>

#include "contact_service.h"
#include "contact_repository.h"
#include "contact_list.h"

static ContactNode *head = NULL;

static int contact_is_valid(const Contact *contact)
{
    if (!contact)
    {
        return 0;
    }

    if (!contact->last_name || !contact->last_name[0])
    {
        return 0;
    }

    if (!contact->first_name || !contact->first_name[0])
    {
        return 0;
    }

    return 1;
}

int service_init(const char *conninfo)
{
    if (repo_init(conninfo) != 0)
    {
        return -1;
    }

    if (repo_load_list(&head) != 0)
    {
        list_clear(head);
        head = NULL;

        repo_close();

        return -1;
    }

    return 0;
}

void service_shutdown(void)
{
    list_clear(head);
    head = NULL;

    repo_close();
}

int service_add(Contact *contact)
{
    if (!contact_is_valid(contact))
    {
        contact_free(contact);
        return -1;
    }

    int new_id = repo_insert(contact);

    if (new_id < 0)
    {
        contact_free(contact);
        return -1;
    }

    contact->id = new_id;

    list_insert_ordered(&head, contact);

    return 0;
}

int service_update(Contact *contact)
{
    if (!contact_is_valid(contact) || contact->id <= 0)
    {
        contact_free(contact);
        return -1;
    }

    if (repo_update(contact) != 0)
    {
        contact_free(contact);
        return -1;
    }

    list_remove(&head, contact->id, 1);
    list_insert_ordered(&head, contact);

    return 0;
}

int service_delete(int id)
{
    if (id <= 0)
    {
        return -1;
    }

    if (repo_delete(id) != 0)
    {
        return -1;
    }

    list_remove(&head, id, 1);

    return 0;
}

void service_print(void)
{
    list_print(head);
}

int service_count(void)
{
    return list_count(head);
}

void service_print_page(int page, int page_size)
{
    int total = list_count(head);

    if (total == 0)
    {
        puts("Список контактов пуст.");
        return;
    }

    int total_pages = (total + page_size - 1) / page_size;
    int start = page * page_size;
    int end = start + page_size;

    if (end > total)
    {
        end = total;
    }

    printf("Страница %d из %d (всего контактов: %d)\n\n",
           page + 1, total_pages, total);

    for (int i = start; i < end; ++i)
    {
        ContactNode *node = list_get_at(head, i);
        if (!node)
        {
            continue;
        }

        const Contact *c = node->contact;

        printf("[%d] %s %s",
               c->id,
               c->last_name ? c->last_name : "",
               c->first_name ? c->first_name : "");

        if (c->middle_name && c->middle_name[0])
        {
            printf(" %s", c->middle_name);
        }

        putchar('\n');

        print_optional_field("Место работы", c->company);
        print_optional_field("Должность", c->position);
        print_optional_field("Телефон", c->phone);
        print_optional_field("E-mail", c->email);
        print_optional_field("Соцсети", c->social);
        print_optional_field("Мессенджеры", c->messenger);

        putchar('\n');
    }
}