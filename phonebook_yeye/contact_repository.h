#ifndef CONTACT_REPOSITORY_H
#define CONTACT_REPOSITORY_H

#include "contact_list.h"

int repo_init(const char *conninfo);

void repo_close(void);

/*
Возвращает id нового контакта или -1 при ошибке.
*/
int repo_insert(Contact *contact);

/*
Возвращает 0 при успехе, -1 при ошибке.
*/
int repo_update(const Contact *contact);

/*
Возвращает 0 при успехе, -1 при ошибке.
*/
int repo_delete(int id);

/*
Загружает все контакты из БД в двусвязный упорядоченный список.
*/
int repo_load_list(ContactNode **head);

#endif