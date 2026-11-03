CREATE TABLE IF NOT EXISTS public.contacts (
    id          SERIAL PRIMARY KEY,
    last_name   TEXT NOT NULL,
    first_name  TEXT NOT NULL,
    middle_name TEXT,
    company     TEXT,
    position    TEXT,
    phone       TEXT,
    email       TEXT,
    social      TEXT,
    messenger   TEXT
);