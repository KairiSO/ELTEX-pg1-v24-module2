FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Устанавливаем PostgreSQL, libpq и компилятор
RUN apt-get update && apt-get install -y \
    gcc \
    make \
    libpq-dev \
    postgresql \
    postgresql-client \
    && rm -rf /var/lib/apt/lists/*

# Создаём системного пользователя для запуска приложения
RUN useradd -m -s /bin/bash appuser

# Копируем исходники и SQL-скрипты
WORKDIR /app
COPY include/ ./include/
COPY src/ ./src/
COPY schema.sql ./schema.sql
COPY data.sql ./data.sql
COPY docker-entrypoint.sh ./docker-entrypoint.sh
RUN chmod +x ./docker-entrypoint.sh

# Компилируем программу
RUN cc -Wall -Wextra -std=c11 -Iinclude \
    -I/usr/include/postgresql \
    src/*.c -lpq -o contact_book

# Даём appuser права на запуск бинарника
RUN chown appuser:appuser /app/contact_book

EXPOSE 5432

ENTRYPOINT ["./docker-entrypoint.sh"]