/*
 * client_api.c
 * ------------
 * Comunicazione col server via socket TCP.
 * Ogni comando (tranne LOGIN e REGISTRA) include il token
 * di sessione come primo parametro.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client_api.h"
#include "client_socket.h"

static void stampa_risposta(const char *titolo, const char *risposta)
{
    printf("==============================================================\n");
    printf("  %s\n", titolo);
    printf("==============================================================\n");
    if (strlen(risposta) == 0 || strncmp(risposta, "ERRORE", 6) == 0)
        printf("  Nessun dato disponibile.\n");
    else
        printf("  %s\n", risposta);
    printf("==============================================================\n");
}

/* ---- LOGIN: risposta attesa "id|ruolo|token" ---- */
int login_server(const char *username, const char *password,
                 char *ruolo_out, int ruolo_size,
                 char *token_out, int token_size)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE], *tok;

    snprintf(cmd, sizeof(cmd), "LOGIN|%s|%s", username, password);
    if (!invia_comando(cmd, risposta, sizeof(risposta))) return -1;
    if (strncmp(risposta, "ERRORE", 6) == 0) return -1;

    /* Parsa "token|id|ruolo" (ordine del server) */
    tok = strtok(risposta, "|");
    if (!tok) return -1;
    strncpy(token_out, tok, token_size - 1);
    token_out[strcspn(token_out, "\r\n")] = '\0';

    tok = strtok(NULL, "|");
    if (!tok) return -1;
    int id = atoi(tok);

    tok = strtok(NULL, "|");
    if (!tok) return -1;
    strncpy(ruolo_out, tok, ruolo_size - 1);
    ruolo_out[strcspn(ruolo_out, "\r\n")] = '\0';

    return id;
}

/* ---- LOGOUT ---- */
int logout_server(const char *token)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "LOGOUT|%s", token);
    return invia_comando(cmd, risposta, sizeof(risposta))
        && strncmp(risposta, "ERRORE", 6) != 0;
}

/* ---- REGISTRAZIONE (senza token) ---- */
int registra_utente_server(const Utente *u, const char *username,
                           const char *password)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    int id;

    snprintf(cmd, sizeof(cmd),
             "AGGIUNGI_UTENTE|%s|%s|cliente|%d|%c|%.1f|%.1f|%.0f|%.0f",
             u->nome, u->cognome, u->eta, u->sesso,
             u->peso_kg, u->altezza_cm, u->bmr, u->tdee_kcal);
    if (!invia_comando(cmd, risposta, sizeof(risposta))) return -1;
    if (strncmp(risposta, "ERRORE", 6) == 0) return -1;

    id = atoi(risposta);
    if (id <= 0) {
        char *p = strchr(risposta, '|');
        id = p ? atoi(p + 1) : 1;
    }

    snprintf(cmd, sizeof(cmd), "REGISTRA|%d|%s|%s", id, username, password);
    if (!invia_comando(cmd, risposta, sizeof(risposta))) return -1;
    if (strncmp(risposta, "ERRORE", 6) == 0) return -1;

    return id;
}

/* ---- COMANDI CON TOKEN ---- */

int salva_storico_server(const char *token, int id_cliente, float peso, const char *data)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "AGGIUNGI_STORICO|%s|%d|%.1f|%s",
             token, id_cliente, peso, data);
    return invia_comando(cmd, risposta, sizeof(risposta))
        && strncmp(risposta, "ERRORE", 6) != 0;
}

int salva_piano_server(const char *token, int id_cliente, float kcal_target, const char *note)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "AGGIUNGI_PIANO_ALIMENTARE|%s|%d|%.0f|%s",
             token, id_cliente, kcal_target, note);
    return invia_comando(cmd, risposta, sizeof(risposta))
        && strncmp(risposta, "ERRORE", 6) != 0;
}

int salva_scheda_server(const char *token, const char *titolo, int id_cliente)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "AGGIUNGI_SCHEDA_ALLENAMENTO|%s|%s|%d",
             token, titolo, id_cliente);
    if (!invia_comando(cmd, risposta, sizeof(risposta))) return -1;
    return (strncmp(risposta, "ERRORE", 6) == 0) ? -1 : atoi(risposta);
}

int salva_dettaglio_scheda_server(const char *token, int id_scheda, int id_esercizio,
                                  int serie, int rip, int recupero)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "AGGIUNGI_DETTAGLI_SCHEDA|%s|%d|%d|%d|%d|%d",
             token, id_scheda, id_esercizio, serie, rip, recupero);
    return invia_comando(cmd, risposta, sizeof(risposta))
        && strncmp(risposta, "ERRORE", 6) != 0;
}

void mostra_storico_server(const char *token, int id_cliente)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "GET_STORICO|%s|%d", token, id_cliente);
    if (!invia_comando(cmd, risposta, sizeof(risposta)))
        snprintf(risposta, sizeof(risposta), "Errore di connessione.");
    stampa_risposta("STORICO PESI", risposta);
}

void mostra_schede_server(const char *token, int id_cliente)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "GET_SCHEDE|%s|%d", token, id_cliente);
    if (!invia_comando(cmd, risposta, sizeof(risposta)))
        snprintf(risposta, sizeof(risposta), "Errore di connessione.");
    stampa_risposta("SCHEDE ALLENAMENTO", risposta);
}

void mostra_piani_server(const char *token, int id_cliente)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "GET_PIANI_ALIMENTARI|%s|%d", token, id_cliente);
    if (!invia_comando(cmd, risposta, sizeof(risposta)))
        snprintf(risposta, sizeof(risposta), "Errore di connessione.");
    stampa_risposta("PIANI ALIMENTARI", risposta);
}

void mostra_lista_utenti(const char *token)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "GET_UTENTI|%s", token);
    if (!invia_comando(cmd, risposta, sizeof(risposta)))
        snprintf(risposta, sizeof(risposta), "Errore di connessione.");
    stampa_risposta("LISTA CLIENTI", risposta);
}

/* ================================================================
 *  VARIANTI _buf — scrivono la risposta in un buffer invece di
 *  stamparla con printf. Necessarie per la GUI Raylib, che disegna
 *  il testo con DrawText() invece di usare stdout.
 * ================================================================ */

void mostra_storico_buf(const char *token, int id_cliente,
                        char *buf, int buf_size)
{
    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "GET_STORICO|%s|%d", token, id_cliente);
    if (!invia_comando(cmd, buf, buf_size))
        strncpy(buf, "Errore di connessione.", buf_size - 1);
}

void mostra_schede_buf(const char *token, int id_cliente,
                       char *buf, int buf_size)
{
    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "GET_SCHEDE|%s|%d", token, id_cliente);
    if (!invia_comando(cmd, buf, buf_size))
        strncpy(buf, "Errore di connessione.", buf_size - 1);
}

void mostra_piani_buf(const char *token, int id_cliente,
                      char *buf, int buf_size)
{
    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "GET_PIANI_ALIMENTARI|%s|%d", token, id_cliente);
    if (!invia_comando(cmd, buf, buf_size))
        strncpy(buf, "Errore di connessione.", buf_size - 1);
}

void mostra_lista_utenti_buf(const char *token,
                             char *buf, int buf_size)
{
    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "GET_UTENTI|%s", token);
    if (!invia_comando(cmd, buf, buf_size))
        strncpy(buf, "Errore di connessione.", buf_size - 1);
}
