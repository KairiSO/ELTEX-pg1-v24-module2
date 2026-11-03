#ifndef CONTACT_H
#define CONTACT_H

typedef struct Contact
{
    int id;

    char *last_name;
    char *first_name;
    char *middle_name;

    char *company;
    char *position;

    char *phone;
    char *email;
    char *social;
    char *messenger;
} Contact;

Contact *contact_new(int id,
                     const char *last_name,
                     const char *first_name,
                     const char *middle_name,
                     const char *company,
                     const char *position,
                     const char *phone,
                     const char *email,
                     const char *social,
                     const char *messenger);

void contact_free(Contact *contact);

int contact_compare(const Contact *a, const Contact *b);

#endif