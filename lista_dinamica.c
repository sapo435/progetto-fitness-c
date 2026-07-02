/*
 * lista_dinamica.c
 * Lista collegata e array dinamico per la GUI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lista_dinamica.h"
#include "sqlite3.h"
#include "app_db.h"   /* per g_db, il database globale */

/* LISTA COLLEGATA - storico pesi */
NodoPeso *carica_storico_dinamico(int id_cliente)
{
    NodoPeso *testa = NULL, *coda = NULL;

    const char *sql =
        "SELECT DATA, PESO_REGISTRATO FROM STORICO"
        " WHERE ID_CLIENTE = ? ORDER BY DATA;";
    sqlite3_stmt *st;
    if (sqlite3_prepare_v2(g_db, sql, -1, &st, NULL) != SQLITE_OK)
        return NULL;
    sqlite3_bind_int(st, 1, id_cliente);

    while (sqlite3_step(st) == SQLITE_ROW) {
        NodoPeso *nodo = (NodoPeso *)malloc(sizeof(NodoPeso));
        if (!nodo) break;  /* memoria esaurita: ci fermiamo con quanto raccolto */

        const char *data = (const char *)sqlite3_column_text(st, 0);
        strncpy(nodo->data, data ? data : "", sizeof(nodo->data) - 1);
        nodo->data[sizeof(nodo->data) - 1] = '\0';
        nodo->peso_kg = (float)sqlite3_column_double(st, 1);
        nodo->next = NULL;

        if (!testa) {
            testa = coda = nodo;
        } else {
            coda->next = nodo;
            coda = nodo;
        }
    }
    sqlite3_finalize(st);
    return testa;
}

void libera_storico_dinamico(NodoPeso *testa)
{
    while (testa) {
        NodoPeso *successivo = testa->next;
        free(testa);
        testa = successivo;
    }
}

void carica_storico_dinamico_in_buffer(int id_cliente, char *buf, int buf_size)
{
    buf[0] = '\0';
    NodoPeso *lista = carica_storico_dinamico(id_cliente);
    for (NodoPeso *n = lista; n; n = n->next) {
        char riga[128];
        snprintf(riga, sizeof(riga), "%s  ->  %.1f kg\n", n->data, n->peso_kg);
        strncat(buf, riga, buf_size - strlen(buf) - 1);
    }
    libera_storico_dinamico(lista);
}

/* ARRAY DINAMICO - lista clienti (cresce con realloc) */
ClienteInfo *carica_lista_clienti_dinamica(int *out_count)
{
    int capacita  = 4;   /* capacita' iniziale, raddoppiata se serve */
    int conteggio = 0;

    ClienteInfo *arr = (ClienteInfo *)malloc(capacita * sizeof(ClienteInfo));
    if (!arr) { if (out_count) *out_count = 0; return NULL; }

    const char *sql =
        "SELECT NOME, COGNOME, ETA, SESSO, PESO, ALTEZZA"
        " FROM UTENTI WHERE RUOLO = 'cliente' ORDER BY ID;";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db, sql, -1, &s, NULL) != SQLITE_OK) {
        free(arr);
        if (out_count) *out_count = 0;
        return NULL;
    }

    while (sqlite3_step(s) == SQLITE_ROW) {
        /* Array pieno: raddoppia la capacita' con realloc() */
        if (conteggio >= capacita) {
            capacita *= 2;
            ClienteInfo *tmp =
                (ClienteInfo *)realloc(arr, capacita * sizeof(ClienteInfo));
            if (!tmp) break;  /* realloc fallita: ci fermiamo con quanto raccolto */
            arr = tmp;
        }

        ClienteInfo *c = &arr[conteggio];
        const char *nm  = (const char *)sqlite3_column_text(s, 0);
        const char *cog = (const char *)sqlite3_column_text(s, 1);
        strncpy(c->nome,    nm  ? nm  : "", DIM_NOME - 1);
        c->nome[DIM_NOME - 1] = '\0';
        strncpy(c->cognome, cog ? cog : "", DIM_NOME - 1);
        c->cognome[DIM_NOME - 1] = '\0';
        c->eta = sqlite3_column_int(s, 2);
        const char *sx = (const char *)sqlite3_column_text(s, 3);
        c->sesso = sx ? sx[0] : 'M';
        c->peso_kg    = (float)sqlite3_column_double(s, 4);
        c->altezza_cm = (float)sqlite3_column_double(s, 5);

        conteggio++;
    }
    sqlite3_finalize(s);

    if (out_count) *out_count = conteggio;
    return arr;
}

void libera_lista_clienti_dinamica(ClienteInfo *arr)
{
    free(arr);
}