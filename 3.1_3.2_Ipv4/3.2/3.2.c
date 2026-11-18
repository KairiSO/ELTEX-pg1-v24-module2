#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <locale.h>

// Преобразование строки "192.168.1.1" в 32-битное целое
uint32_t parse_ip(const char* str) {
    unsigned int b1, b2, b3, b4;
    if (sscanf(str, "%u.%u.%u.%u", &b1, &b2, &b3, &b4) != 4) {
        return 0; // Ошибка парсинга
    }
    return (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
}

// Преобразование 32-битного целого обратно в строку "192.168.1.1"
void print_ip(uint32_t ip) {
    printf("%u.%u.%u.%u",
           (ip >> 24) & 0xFF,
           (ip >> 16) & 0xFF,
           (ip >> 8) & 0xFF,
           ip & 0xFF);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");

    // Проверка аргументов командной строки
    if (argc != 4) {
        printf("Использование: %s <IP_шлюза> <Маска_подсети> <Кол-во_пакетов>\n", argv[0]);
        printf("Пример: %s 192.168.1.1 255.255.255.0 10000\n", argv[0]);
        return 1;
    }

    uint32_t gateway_ip = parse_ip(argv[1]);
    uint32_t subnet_mask = parse_ip(argv[2]);
    int packet_count = atoi(argv[3]);

    if (packet_count <= 0) {
        printf("Ошибка: количество пакетов должно быть > 0.\n");
        return 1;
    }

    // Вычисляем адрес "своей" подсети
    uint32_t local_network = gateway_ip & subnet_mask;

    srand(time(NULL));
    int local_packets = 0;
    int other_packets = 0;
    uint32_t host_bits = ~subnet_mask; // Битовая маска для хостовой части

    printf("Имитация обработки %d пакетов (целевое попадание: ~15%%)...\n\n", packet_count);

    for (int i = 0; i < packet_count; i++) {
        uint32_t dest_ip;

        // Генерируем число от 1 до 100 для определения вероятности
        int chance = (rand() % 100) + 1;

        if (chance <= 15)
        {
            uint32_t random_host = rand() & host_bits;

            // Избегаем сетевого адреса (host = 0) и широковещательного (host = все 1)
            if (random_host == 0) random_host = 1;
            if (random_host == host_bits) random_host--;

            dest_ip = local_network | random_host;
        } else {
            // 85% случаев: генерируем IP строго вне "своей" подсети
            do {
                dest_ip = ((uint32_t)(rand() % 256) << 24) |
                          ((uint32_t)(rand() % 256) << 16) |
                          ((uint32_t)(rand() % 256) << 8) |
                          ((uint32_t)(rand() % 256));
            } while ((dest_ip & subnet_mask) == local_network); // Перегенерируем при совпадении
        }

        // Определяем принадлежность подсети (для статистики)
        uint32_t dest_network = dest_ip & subnet_mask;

        if (dest_network == local_network) {
            local_packets++;
        } else {
            other_packets++;
        }
    }

    // Вывод статистики
    float local_percent = (float)local_packets / packet_count * 100.0f;
    float other_percent = (float)other_packets / packet_count * 100.0f;

    printf("Адрес локальной подсети: ");
    print_ip(local_network);
    printf("\n\n--- Статистика ---\n");
    printf("Своя подсеть: %d пакетов (%.2f%%)\n", local_packets, local_percent);
    printf("Другие сети:  %d пакетов (%.2f%%)\n", other_packets, other_percent);

    return 0;
}