#ifndef CONTACT_SERVICE_H
#define CONTACT_SERVICE_H

#include "contact.h"

int service_init(const char *conninfo);

void service_shutdown(void);

/*
Передает владение контактом сервису.
При ошибке сервис сам освобождает память.
*/
int service_add(Contact *contact);

int service_update(Contact *contact);

int service_delete(int id);

void service_print(void);

int  service_count(void);
void service_print_page(int page, int page_size);

#endif