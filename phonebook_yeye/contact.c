#include <stdlib.h>
#include <string.h>

#include "contact.h"

static char *copy_string(const char *value)
{
    if (!value)
    {
        return NULL;
    }

    size_t size = strlen(value) + 1;
    char *copy = malloc(size);

    if (copy)
    {
        memcpy(copy, value, size);
    }

    return copy;
}

Contact *contact_new(int id,
                     const char *last_name,
                     const char *first_name,
                     const char *middle_name,
                     const char *company,
                     const char *position,
                     const char *phone,
                     const char *email,
                     const char *social,
                     const char *messenger)
{
    Contact *contact = malloc(sizeof(*contact));

    if (!contact)
    {
        return NULL;
    }

    contact->id = id;

    contact->last_name = copy_string(last_name);
    contact->first_name = copy_string(first_name);
    contact->middle_name = copy_string(middle_name);

    contact->company = copy_string(company);
    contact->position = copy_string(position);

    contact->phone = copy_string(phone);
    contact->email = copy_string(email);
    contact->social = copy_string(social);
    contact->messenger = copy_string(messenger);

    return contact;
}

void contact_free(Contact *contact)
{
    if (!contact)
    {
        return;
    }

    free(contact->last_name);
    free(contact->first_name);
    free(contact->middle_name);

    free(contact->company);
    free(contact->position);

    free(contact->phone);
    free(contact->email);
    free(contact->social);
    free(contact->messenger);

    free(contact);
}

static int compare_nullable_strings(const char *a, const char *b)
{
    if (!a && !b)
    {
        return 0;
    }

    if (!a)
    {
        return -1;
    }

    if (!b)
    {
        return 1;
    }

    return strcmp(a, b);
}

int contact_compare(const Contact *a, const Contact *b)
{
    if (!a && !b)
    {
        return 0;
    }

    if (!a)
    {
        return -1;
    }

    if (!b)
    {
        return 1;
    }

    int result = compare_nullable_strings(a->last_name, b->last_name);
    if (result != 0)
    {
        return result;
    }

    result = compare_nullable_strings(a->first_name, b->first_name);
    if (result != 0)
    {
        return result;
    }

    result = compare_nullable_strings(a->middle_name, b->middle_name);
    if (result != 0)
    {
        return result;
    }

    if (a->id < b->id)
    {
        return -1;
    }

    if (a->id > b->id)
    {
        return 1;
    }

    return 0;
}