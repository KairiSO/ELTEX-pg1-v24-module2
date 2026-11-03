#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "loader.h"

/* Каталог, в котором лежат .so плагины.
   Путь считается относительно места запуска программы. */
#define PLUGINS_DIR "./plugins"


/* Печатает меню: список всех доступных команд с их номерами. */
static void print_menu(const CommandList *list)
{
    printf("\n=== Калькулятор ===\n");
    if (list->count == 0) {
        printf("  (команды не найдены — проверьте каталог %s)\n", PLUGINS_DIR);
        return;
    }
    for (int i = 0; i < list->count; i++) {
        printf("  %d) %s\n", i + 1, list->commands[i].name);
    }
    printf("  0) Выход\n");
}


/* Читает из stdin одно целое число. Возвращает 0 при успехе, -1 при ошибке. */
static int read_int(int *out)
{
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin) == NULL) return -1;
    char *end = NULL;
    long v = strtol(buf, &end, 10);
    if (end == buf) return -1;      /* ничего не распарсилось */
    *out = (int)v;
    return 0;
}


/* Читает из stdin одно число с плавающей точкой. */
static int read_double(double *out)
{
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin) == NULL) return -1;
    char *end = NULL;
    *out = strtod(buf, &end);
    if (end == buf) return -1;
    return 0;
}


int main(void)
{
    CommandList list;

    /* === Шаг 1. Загружаем все плагины из каталога PLUGINS_DIR. === */
    if (load_plugins(&list, PLUGINS_DIR) != 0) {
        fprintf(stderr, "Ошибка при загрузке плагинов.\n");
        return 1;
    }
    printf("Загружено команд: %d\n", list.count);

    /* === Шаг 2. Главный цикл работы калькулятора. === */
    while (1) {
        print_menu(&list);

        printf("Выберите действие: ");
        int choice;
        if (read_int(&choice) != 0) {
            printf("Некорректный ввод.\n");
            continue;
        }

        if (choice == 0) break;                     /* выход */
        if (choice < 1 || choice > list.count) {
            printf("Нет такой команды.\n");
            continue;
        }

        /* Запрашиваем два аргумента. */
        printf("Введите первый аргумент:  ");
        double a;
        if (read_double(&a) != 0) {
            printf("Некорректное число.\n");
            continue;
        }

        printf("Введите второй аргумент:  ");
        double b;
        if (read_double(&b) != 0) {
            printf("Некорректное число.\n");
            continue;
        }

        /* Вызываем функцию через указатель.
           list.commands[choice-1].func — это и есть указатель на execute(). */
        Command *cmd = &list.commands[choice - 1];
        double result = cmd->func(a, b);

        /* Обрабатываем особые значения (деление на ноль и т.п.). */
        if (isinf(result)) {
            printf("Результат: ошибка (бесконечность)\n");
        } else if (isnan(result)) {
            printf("Результат: ошибка (не число)\n");
        } else {
            printf("Результат: %g\n", result);
        }
    }

    /* === Шаг 3. Освобождаем ресурсы. === */
    free_plugins(&list);
    printf("Выход. Всего было выполнено команд в сессии.\n");
    return 0;
}