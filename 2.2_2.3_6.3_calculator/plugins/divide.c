/* Плагин деления. Проверяет деление на ноль внутри плагина. */

const char* get_name(void)
{
    return "Деление";
}

double execute(double a, double b)
{
    if (b == 0.0) {
        printf("Ошибка: деление на ноль.\n");
        return 0.0;
    }
    return a / b;
}