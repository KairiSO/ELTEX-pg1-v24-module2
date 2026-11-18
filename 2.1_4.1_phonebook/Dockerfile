FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    gcc \
    make \
    libpq-dev \
    postgresql \
    postgresql-client \
    bash \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -s /bin/bash appuser

WORKDIR /app
COPY include/ ./include/
COPY src/ ./src/
COPY Makefile ./Makefile
COPY schema.sql ./schema.sql
COPY data.sql ./data.sql
COPY docker-entrypoint.sh ./docker-entrypoint.sh

# Конвертируем CRLF в LF на всякий случай
RUN sed -i 's/\r$//' ./docker-entrypoint.sh
RUN chmod +x ./docker-entrypoint.sh

RUN make

RUN chown appuser:appuser /app/contact_book

EXPOSE 5432

# Явно указываем bash
ENTRYPOINT ["/bin/bash", "./docker-entrypoint.sh"]