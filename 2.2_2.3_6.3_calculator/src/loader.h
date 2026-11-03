#ifndef LOADER_H
#define LOADER_H

/* Тип указателя на функцию операции: принимает два double, возвращает double.
   Все плагины обязаны экспортировать функцию именно такого вида. */
typedef double (*operation_func)(double, double);

/* Одна команда: имя (для меню) и указатель на её реализацию. */
typedef struct
{
    char name[64];       /* Название операции, полученное из get_name() */
    operation_func func; /* Указатель на функцию execute()              */
    void *handle;        /* Дескриптор .so — нужен для последующего dlclose */
} Command;

/* Динамический массив команд. */
typedef struct
{
    Command *commands; /* Указатель на массив структур Command        */
    int count;         /* Текущее количество загруженных команд       */
    int capacity;      /* Выделенная ёмкость массива                  */
} CommandList;

/* Сканирует каталог directory, загружает все .so файлы и
   добавляет найденные команды в список list.
   Возвращает 0 при успехе, -1 при ошибке. */
int load_plugins(CommandList *list, const char *directory);

/* Освобождает все ресурсы: выгружает библиотеки, освобождает память. */
void free_plugins(CommandList *list);

#endif