#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <locale.h>

// Функция для вывода прав доступа во всех трех форматах
void print_permissions(mode_t mode) {
    char symbolic[10];
    const char* chars = "rwxrwxrwx";
    
    // Формируем буквенное представление
    for (int i = 0; i < 9; i++) {
        if (mode & (1 << (8 - i))) {
            symbolic[i] = chars[i];
        } else {
            symbolic[i] = '-';
        }
    }
    symbolic[9] = '\0';

    printf("  Буквенное: %s\n", symbolic);
    printf("  Цифровое:  %o\n", mode & 0777);
    printf("  Битовое:   ");
    for (int i = 8; i >= 0; i--) {
        printf("%d", (mode >> i) & 1);
        if (i % 3 == 0 && i != 0) printf(" "); // Разделитель для групп rwx
    }
    printf("\n");
}

// Парсинг ввода: поддержка числового (755) и буквенного (rwxr-xr-x) форматов
mode_t parse_input(const char* input) {
    mode_t mode = 0;
    
    // Если начинается с цифры, считаем числовым (октал)
    if (input[0] >= '0' && input[0] <= '7') {
        mode = strtol(input, NULL, 8);
    } 
    // Иначе считаем буквенным
    else if (strlen(input) == 9) {
        const char* map = "rwxrwxrwx";
        for (int i = 0; i < 9; i++) {
            if (input[i] == map[i]) {
                mode |= (1 << (8 - i));
            }
        }
    } else {
        printf("Ошибка: неверный формат ввода.\n");
        return -1;
    }
    return mode & 0777;
}

// Имитация команды chmod (например, u+x, go-w, a=r)
mode_t apply_chmod(mode_t current_mode, const char* cmd) {
    int targets[3] = {0}; // 0: user, 1: group, 2: other
    char op = 0;
    int perms = 0;
    int i = 0;

    // 1. Определяем цели (u, g, o, a)
    while (cmd[i] && strchr("ugoa", cmd[i])) {
        if (cmd[i] == 'u' || cmd[i] == 'a') targets[0] = 1;
        if (cmd[i] == 'g' || cmd[i] == 'a') targets[1] = 1;
        if (cmd[i] == 'o' || cmd[i] == 'a') targets[2] = 1;
        i++;
    }
    
    // 2. Определяем операцию (+, -, =)
    if (cmd[i] == '+' || cmd[i] == '-' || cmd[i] == '=') {
        op = cmd[i++];
    } else {
        printf("Ошибка: неизвестная операция.\n");
        return current_mode;
    }

    // 3. Определяем права (r, w, x)
    while (cmd[i]) {
        if (cmd[i] == 'r') perms |= 4;
        else if (cmd[i] == 'w') perms |= 2;
        else if (cmd[i] == 'x') perms |= 1;
        i++;
    }

    // 4. Применяем изменения к маске
    for (int t = 0; t < 3; t++) {
        if (targets[t]) {
            int shift = (2 - t) * 3; // Сдвиг для user(6), group(3), other(0)
            if (op == '+') {
                current_mode |= (perms << shift);
            } else if (op == '-') {
                current_mode &= ~(perms << shift);
            } else if (op == '=') {
                current_mode &= ~(7 << shift); // Сбрасываем биты
                current_mode |= (perms << shift); // Устанавливаем новые
            }
        }
    }
    return current_mode & 0777;
}

int main() {
    setlocale(LC_ALL, "Russian");
    int choice;
    mode_t current_mode = 0;

    while (1) {
        printf("\n--- Меню прав доступа ---\n");
        printf("1. Ввести права (буквенно/цифрово)\n");
        printf("2. Получить права файла (stat)\n");
        printf("3. Изменить права (имитация chmod)\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");
        
        if (scanf("%d", &choice) != 1) break;
        if (choice == 0) break;

        if (choice == 1) {
            char input[20];
            printf("Введите права (напр. 755 или rwxr-xr-x): ");
            scanf("%s", input);
            current_mode = parse_input(input);
            if (current_mode != (mode_t)-1) {
                printf("Текущие права:\n");
                print_permissions(current_mode);
            }
        } 
        else if (choice == 2) {
            char filename[256];
            struct stat buf;
            printf("Введите имя файла: ");
            scanf("%s", filename);
            
            if (stat(filename, &buf) == 0) {
                current_mode = buf.st_mode & 0777;
                printf("Права файла %s:\n", filename);
                print_permissions(current_mode);
            } else {
                perror("Ошибка stat");
            }
        } 
        else if (choice == 3) {
            char cmd[20];
            printf("Введите команду модификации (напр. u+x, go-w): ");
            scanf("%s", cmd);
            current_mode = apply_chmod(current_mode, cmd);
            printf("Новые права:\n");
            print_permissions(current_mode);
        }
    }
    return 0;
}