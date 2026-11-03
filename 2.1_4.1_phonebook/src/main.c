#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contact.h"
#include "contact_service.h"

static void read_line(const char *prompt, char *buffer, size_t size)
{
    fputs(prompt, stdout);
    fflush(stdout);

    if (!fgets(buffer, (int)size, stdin))
    {
        buffer[0] = '\0';
        exit(0);
    }

    buffer[strcspn(buffer, "\r\n")] = '\0';
}

static void read_required(const char *prompt, char *buffer, size_t size)
{
    while (1)
    {
        read_line(prompt, buffer, size);

        if (buffer[0] != '\0')
        {
            break;
        }

        puts("Поле не может быть пустым.");
    }
}

static const char *optional(const char *value)
{
    if (value && value[0])
    {
        return value;
    }

    return NULL;
}

static Contact *input_contact(int id)
{
    char last_name[256];
    char first_name[256];
    char middle_name[256];

    char company[256];
    char position[256];

    char phone[256];
    char email[256];
    char social[256];
    char messenger[256];

    read_required("Фамилия: ", last_name, sizeof(last_name));
    read_required("Имя: ", first_name, sizeof(first_name));

    read_line("Отчество: ", middle_name, sizeof(middle_name));

    read_line("Место работы: ", company, sizeof(company));
    read_line("Должность: ", position, sizeof(position));

    read_line("Телефон: ", phone, sizeof(phone));
    read_line("E-mail: ", email, sizeof(email));
    read_line("Соцсети: ", social, sizeof(social));
    read_line("Мессенджеры: ", messenger, sizeof(messenger));

    return contact_new(
        id,
        last_name,
        first_name,
        optional(middle_name),
        optional(company),
        optional(position),
        optional(phone),
        optional(email),
        optional(social),
        optional(messenger));
}

static void print_menu(void)
{
    puts("");
    puts("1. Добавить контакт");
    puts("2. Редактировать контакт");
    puts("3. Удалить контакт");
    puts("4. Показать контакты");
    puts("0. Выход");
}

int main(void)
{
    const char *conninfo = getenv("DATABASE_URL");

    if (!conninfo)
    {
        conninfo =
            "host=localhost port=5432 dbname=contacts "
            "user=postgres password=postgres";
    }

    if (service_init(conninfo) != 0)
    {
        fprintf(stderr,
                "Не удалось инициализировать сервис контактов.\n"
                "Проверьте строку подключения и убедитесь, что применён скрипт schema.sql.\n");
        return 1;
    }

    int running = 1;

    while (running)
    {
        print_menu();

        char choice[32];
        read_line("Выбор: ", choice, sizeof(choice));

        switch (atoi(choice))
        {
        case 1:
        {
            Contact *contact = input_contact(0);

            if (service_add(contact) == 0)
            {
                puts("Контакт добавлен.");
            }
            else
            {
                puts("Ошибка добавления контакта.");
            }

            break;
        }

        case 2:
        {
            char id_text[32];
            read_line("ID контакта для редактирования: ",
                      id_text,
                      sizeof(id_text));

            int id = atoi(id_text);

            if (id <= 0)
            {
                puts("Некорректный ID.");
                break;
            }

            Contact *contact = input_contact(id);

            if (service_update(contact) == 0)
            {
                puts("Контакт обновлен.");
            }
            else
            {
                puts("Ошибка обновления контакта.");
            }

            break;
        }

        case 3:
        {
            char id_text[32];
            read_line("ID контакта для удаления: ",
                      id_text,
                      sizeof(id_text));

            int id = atoi(id_text);

            if (service_delete(id) == 0)
            {
                puts("Контакт удален.");
            }
            else
            {
                puts("Ошибка удаления контакта.");
            }

            break;
        }

        case 4:
        {
            int page_size = 10;
            int page = 0;
            int total = service_count();
            int total_pages = (total + page_size - 1) / page_size;

            if (total_pages == 0)
            {
                total_pages = 1;
            }

            int browsing = 1;

            while (browsing)
            {
                service_print_page(page, page_size);

                printf("Страница %d/%d. [n] следующая, [p] предыдущая, [q] в меню: ",
                       page + 1, total_pages);

                char cmd[32];
                read_line("", cmd, sizeof(cmd));

                if (cmd[0] == 'n' || cmd[0] == 'N')
                {
                    if (page + 1 < total_pages)
                    {
                        ++page;
                    }
                    else
                    {
                        puts("Это последняя страница.");
                    }
                }
                else if (cmd[0] == 'p' || cmd[0] == 'P')
                {
                    if (page > 0)
                    {
                        --page;
                    }
                    else
                    {
                        puts("Это первая страница.");
                    }
                }
                else if (cmd[0] == 'q' || cmd[0] == 'Q')
                {
                    browsing = 0;
                }
                else
                {
                    puts("Неизвестная команда. Используйте n/p/q.");
                }
            }

            break;
        }

        case 0:
        {
            running = 0;
            break;
        }

        default:
        {
            puts("Неизвестная команда.");
            break;
        }
        }
    }

    service_shutdown();

    return 0;
}