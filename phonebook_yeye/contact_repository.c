#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

#include "contact_repository.h"

static PGconn *db = NULL;

static const char *text_param(const char *value)
{
    if (value && value[0])
    {
        return value;
    }

    return NULL;
}

static const char *text_field(const PGresult *res, int row, int col)
{
    if (PQgetisnull(res, row, col))
    {
        return NULL;
    }

    const char *value = PQgetvalue(res, row, col);

    if (value && value[0])
    {
        return value;
    }

    return NULL;
}

int repo_init(const char *conninfo)
{
    db = PQconnectdb(conninfo ? conninfo : "");

    if (PQstatus(db) != CONNECTION_OK)
    {
        fprintf(stderr, "PostgreSQL connection error: %s\n",
                PQerrorMessage(db));

        PQfinish(db);
        db = NULL;

        return -1;
    }

    return 0;
}

void repo_close(void)
{
    if (db)
    {
        PQfinish(db);
        db = NULL;
    }
}

int repo_insert(Contact *contact)
{
    if (!db || !contact)
    {
        return -1;
    }

    static const char *sql =
        "INSERT INTO contacts("
        "last_name, first_name, middle_name, "
        "company, position, phone, email, social, messenger"
        ") VALUES($1,$2,$3,$4,$5,$6,$7,$8,$9) RETURNING id";

    const char *params[9] = {
        text_param(contact->last_name),
        text_param(contact->first_name),
        text_param(contact->middle_name),
        text_param(contact->company),
        text_param(contact->position),
        text_param(contact->phone),
        text_param(contact->email),
        text_param(contact->social),
        text_param(contact->messenger)};

    PGresult *res = PQexecParams(db, sql, 9, NULL, params, NULL, NULL, 0);

    if (!res || PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0)
    {
        fprintf(stderr, "PostgreSQL insert error: %s\n",
                PQerrorMessage(db));

        if (res)
        {
            PQclear(res);
        }

        return -1;
    }

    int new_id = atoi(PQgetvalue(res, 0, 0));

    PQclear(res);

    return new_id;
}

int repo_update(const Contact *contact)
{
    if (!db || !contact || contact->id <= 0)
    {
        return -1;
    }

    static const char *sql =
        "UPDATE contacts SET "
        "last_name=$1, "
        "first_name=$2, "
        "middle_name=$3, "
        "company=$4, "
        "position=$5, "
        "phone=$6, "
        "email=$7, "
        "social=$8, "
        "messenger=$9 "
        "WHERE id=$10";

    char id_text[32];
    snprintf(id_text, sizeof(id_text), "%d", contact->id);

    const char *params[10] = {
        text_param(contact->last_name),
        text_param(contact->first_name),
        text_param(contact->middle_name),
        text_param(contact->company),
        text_param(contact->position),
        text_param(contact->phone),
        text_param(contact->email),
        text_param(contact->social),
        text_param(contact->messenger),
        id_text};

    PGresult *res = PQexecParams(db, sql, 10, NULL, params, NULL, NULL, 0);

    int ok = res &&
             PQresultStatus(res) == PGRES_COMMAND_OK &&
             atoi(PQcmdTuples(res)) == 1;

    if (!ok)
    {
        fprintf(stderr, "PostgreSQL update error: %s\n",
                PQerrorMessage(db));

        if (res)
        {
            PQclear(res);
        }

        return -1;
    }

    PQclear(res);

    return 0;
}

int repo_delete(int id)
{
    if (!db || id <= 0)
    {
        return -1;
    }

    static const char *sql = "DELETE FROM contacts WHERE id=$1";

    char id_text[32];
    snprintf(id_text, sizeof(id_text), "%d", id);

    const char *params[1] = {
        id_text};

    PGresult *res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);

    int ok = res &&
             PQresultStatus(res) == PGRES_COMMAND_OK &&
             atoi(PQcmdTuples(res)) == 1;

    if (!ok)
    {
        fprintf(stderr, "PostgreSQL delete error: %s\n",
                PQerrorMessage(db));

        if (res)
        {
            PQclear(res);
        }

        return -1;
    }

    PQclear(res);

    return 0;
}

int repo_load_list(ContactNode **head)
{
    if (!db || !head)
    {
        return -1;
    }

    static const char *sql =
        "SELECT "
        "id, last_name, first_name, middle_name, "
        "company, position, phone, email, social, messenger "
        "FROM contacts "
        "ORDER BY last_name, first_name, middle_name, id";

    PGresult *res = PQexec(db, sql);

    if (!res || PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "PostgreSQL select error: %s\n",
                PQerrorMessage(db));

        if (res)
        {
            PQclear(res);
        }

        return -1;
    }

    int rows = PQntuples(res);

    for (int row = 0; row < rows; ++row)
    {
        int id = atoi(PQgetvalue(res, row, 0));

        Contact *contact = contact_new(
            id,
            text_field(res, row, 1),
            text_field(res, row, 2),
            text_field(res, row, 3),
            text_field(res, row, 4),
            text_field(res, row, 5),
            text_field(res, row, 6),
            text_field(res, row, 7),
            text_field(res, row, 8),
            text_field(res, row, 9));

        if (!contact)
        {
            PQclear(res);
            return -1;
        }

        list_insert_ordered(head, contact);
    }

    PQclear(res);

    return 0;
}