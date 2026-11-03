#!/usr/bin/env bash
set -e

PG_BIN="/usr/lib/postgresql/14/bin"
PG_DATA="/var/lib/postgresql/data"
PG_LOG="/var/log/postgresql"
DB_NAME="contacts"
APP_USER="appuser"
APP_PASS="apppass"

# 1. Создаём необходимые директории
mkdir -p "$PG_DATA" "$PG_LOG"
chown -R postgres:postgres "$PG_DATA" "$PG_LOG"

# 2. Инициализируем кластер, если его ещё нет
if [ ! -f "$PG_DATA/PG_VERSION" ]; then
    echo ">>> Инициализация PostgreSQL..."
    su -s /bin/bash postgres -c "$PG_BIN/initdb -D $PG_DATA"
fi

# 3. Настраиваем pg_hba.conf для локального входа по паролю
#    (по умолчанию в контейнере может стоять peer, который не работает)
echo ">>> Настройка pg_hba.conf..."
cat > "$PG_DATA/pg_hba.conf" <<EOF
local   all   all                 trust
host    all   all   127.0.0.1/32  md5
host    all   all   ::1/128       md5
EOF

# 4. Запускаем PostgreSQL
echo ">>> Запуск PostgreSQL..."
su -s /bin/bash postgres -c "$PG_BIN/pg_ctl -D $PG_DATA -l $PG_LOG/pg.log start"

# Ждём готовности
echo ">>> Ожидание готовности PostgreSQL..."
for i in $(seq 1 30); do
    if su -s /bin/bash postgres -c "$PG_BIN/pg_isready" > /dev/null 2>&1; then
        echo ">>> PostgreSQL готов."
        break
    fi
    if [ "$i" -eq 30 ]; then
        echo ">>> ОШИБКА: PostgreSQL не запустился. Логи:"
        cat "$PG_LOG/pg.log"
        exit 1
    fi
    sleep 1
done

# 5. Создаём пользователя и базу
echo ">>> Создание пользователя и базы данных..."
su -s /bin/bash postgres -c "$PG_BIN/psql -c \"CREATE USER $APP_USER WITH PASSWORD '$APP_PASS';\"" 2>/dev/null || true
su -s /bin/bash postgres -c "$PG_BIN/psql -c \"CREATE DATABASE $DB_NAME OWNER $APP_USER;\"" 2>/dev/null || true

# 6. Применяем схему
echo ">>> Применение schema.sql..."
su -s /bin/bash postgres -c "$PG_BIN/psql -d $DB_NAME -f /app/schema.sql"

# 7. Выдаём права пользователю appuser
echo ">>> Выдача прав..."
su -s /bin/bash postgres -c "$PG_BIN/psql -d $DB_NAME -c \"GRANT ALL ON SCHEMA public TO $APP_USER; GRANT ALL ON ALL TABLES IN SCHEMA public TO $APP_USER; GRANT ALL ON ALL SEQUENCES IN SCHEMA public TO $APP_USER; ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO $APP_USER; ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO $APP_USER;\""

# 8. Заполняем тестовыми данными (если таблица пустая)
echo ">>> Заполнение тестовыми данными..."
COUNT=$(su -s /bin/bash postgres -c "$PG_BIN/psql -d $DB_NAME -t -c 'SELECT count(*) FROM contacts;'")
if [ "$COUNT" -eq 0 ] 2>/dev/null; then
    su -s /bin/bash postgres -c "$PG_BIN/psql -d $DB_NAME -f /app/data.sql"
    echo ">>> Добавлено 20 тестовых контактов."
else
    echo ">>> Таблица уже содержит $COUNT контактов, пропускаем заполнение."
fi

# 9. Запускаем телефонную книгу
echo ">>> Запуск телефонной книги..."
echo ""
export DATABASE_URL="host=127.0.0.1 port=5432 dbname=$DB_NAME user=$APP_USER password=$APP_PASS"
exec su -s /bin/bash appuser -c "cd /app && DATABASE_URL='$DATABASE_URL' ./contact_book"