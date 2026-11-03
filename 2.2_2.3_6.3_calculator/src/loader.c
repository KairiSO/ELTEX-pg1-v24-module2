#include "loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>  /* dlopen, dlsym, dlclose, dlerror */
#include <dirent.h> /* opendir, readdir                */

/* Начальная ёмкость массива команд. При переполнении будет удваиваться. */
#define INITIAL_CAPACITY 8

/* Суффикс, по которому мы отличаем разделяемые библиотеки от других файлов. */
#define PLUGIN_EXT ".so"

/* Внутренняя функция: добавляет одну команду в список.
   При необходимости расширяет массив (удвоением). */
static int add_command(CommandList *list, const char *name,
                       operation_func func, void *handle)
{
    if (list->count >= list->capacity)
    {
        int new_capacity = list->capacity * 2;
        Command *tmp = realloc(list->commands, new_capacity * sizeof(Command));
        if (tmp == NULL)
        {
            return -1;
        }
        list->commands = tmp;
        list->capacity = new_capacity;
    }

    /* Копируем имя и сохраняем указатели. */
    strncpy(list->commands[list->count].name, name,
            sizeof(list->commands[0].name) - 1);
    list->commands[list->count].name[sizeof(list->commands[0].name) - 1] = '\0';
    list->commands[list->count].func = func;
    list->commands[list->count].handle = handle;
    list->count++;
    return 0;
}

/* Пытается загрузить один файл как плагин.
   Возвращает 0, если файл успешно загружен и содержит нужные символы.
   Возвращает -1, если файл не является плагином (например, нет нужных функций). */
static int load_one_plugin(CommandList *list, const char *path)
{
    /* RTLD_NOW — разрешить все символы сразу, чтобы сразу узнать об ошибках. */
    void *handle = dlopen(path, RTLD_NOW);
    if (handle == NULL)
    {
        fprintf(stderr, "[loader] Не удалось загрузить %s: %s\n", path, dlerror());
        return -1;
    }

    /* Ищем две обязательные функции: get_name и execute.
       Если хотя бы одной нет — библиотека не является плагином,
       закрываем её и возвращаем ошибку (не считаем это фатальной ошибкой). */
    const char *(*get_name)(void) = (const char *(*)(void))dlsym(handle, "get_name");
    operation_func execute = (operation_func)dlsym(handle, "execute");

    if (get_name == NULL || execute == NULL)
    {
        /* Это не плагин калькулятора — молча закрываем. */
        dlclose(handle);
        return -1;
    }

    /* Всё в порядке: добавляем команду в список. */
    if (add_command(list, get_name(), execute, handle) != 0)
    {
        dlclose(handle);
        return -1;
    }
    return 0;
}

int load_plugins(CommandList *list, const char *directory)
{
    DIR *dir = opendir(directory);
    if (dir == NULL)
    {
        perror("[loader] opendir");
        return -1;
    }

    /* Инициализируем список команд пустым массивом. */
    list->commands = malloc(INITIAL_CAPACITY * sizeof(Command));
    list->count = 0;
    list->capacity = INITIAL_CAPACITY;
    if (list->commands == NULL)
    {
        closedir(dir);
        return -1;
    }

    /* Проходим по всем файлам в каталоге. */
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        /* Нас интересуют только обычные файлы с расширением .so */
        size_t name_len = strlen(entry->d_name);
        size_t ext_len = strlen(PLUGIN_EXT);
        if (name_len <= ext_len)
            continue;
        if (strcmp(entry->d_name + name_len - ext_len, PLUGIN_EXT) != 0)
            continue;

        /* Собираем полный путь: directory/filename */
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        /* Пытаемся загрузить. Если не получилось — просто пропускаем. */
        load_one_plugin(list, path);
    }

    closedir(dir);
    return 0;
}

void free_plugins(CommandList *list)
{
    if (list == NULL)
        return;

    /* Сначала выгружаем все библиотеки — это освобождает память,
       которую занял код плагина в адресном пространстве процесса. */
    for (int i = 0; i < list->count; i++)
    {
        if (list->commands[i].handle != NULL)
        {
            dlclose(list->commands[i].handle);
        }
    }

    /* Затем освобождаем сам массив команд. */
    free(list->commands);
    list->commands = NULL;
    list->count = 0;
    list->capacity = 0;
}