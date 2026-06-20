/*
 * server.c
 * --------
 * Comunicazione col server: login, registrazione, salvataggio
 * e visualizzazione dati remoti via socket TCP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "client_socket.h"

/* Helper: stampa dati dal server con titolo e bordi */
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

int login_server(const char *username, const char *password,
                 char *ruolo_out, int ruolo_size)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE], *token;

    snprintf(cmd, sizeof(cmd), "LOGIN|%s|%s", username, password);
    if (!invia_comando(cmd, risposta, sizeof(risposta))) return -1;
    if (strncmp(risposta, "ERRORE", 6) == 0) return -1;

    token = strtok(risposta, "|");
    if (!token) return -1;
    int id = atoi(token);

    token = strtok(NULL, "|");
    if (!token) return -1;
    strncpy(ruolo_out, token, ruolo_size - 1);
    ruolo_out[strcspn(ruolo_out, "\r\n")] = '\0';
    return id;
}

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

int salva_storico_server(int id_cliente, float peso, const char *data)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "AGGIUNGI_STORICO|%d|%.1f|%s", id_cliente, peso, data);
    return invia_comando(cmd, risposta, sizeof(risposta))
        && strncmp(risposta, "ERRORE", 6) != 0;
}

int salva_piano_server(int id_cliente, float kcal_target, const char *note)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "AGGIUNGI_PIANO_ALIMENTARE|%d|%.0f|%s",
             id_cliente, kcal_target, note);
    return invia_comando(cmd, risposta, sizeof(risposta))
        && strncmp(risposta, "ERRORE", 6) != 0;
}

int salva_scheda_server(const char *titolo, int id_cliente)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "AGGIUNGI_SCHEDA_ALLENAMENTO|%s|%d", titolo, id_cliente);
    if (!invia_comando(cmd, risposta, sizeof(risposta))) return -1;
    return (strncmp(risposta, "ERRORE", 6) == 0) ? -1 : atoi(risposta);
}

int salva_dettaglio_scheda_server(int id_scheda, int id_esercizio,
                                  int serie, int rip, int recupero)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "AGGIUNGI_DETTAGLI_SCHEDA|%d|%d|%d|%d|%d",
             id_scheda, id_esercizio, serie, rip, recupero);
    return invia_comando(cmd, risposta, sizeof(risposta))
        && strncmp(risposta, "ERRORE", 6) != 0;
}

void mostra_storico_server(int id_cliente)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "GET_STORICO|%d", id_cliente);
    if (!invia_comando(cmd, risposta, sizeof(risposta)))
        snprintf(risposta, sizeof(risposta), "Errore di connessione.");
    stampa_risposta("STORICO PESI", risposta);
}

void mostra_schede_server(int id_cliente)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "GET_SCHEDE|%d", id_cliente);
    if (!invia_comando(cmd, risposta, sizeof(risposta)))
        snprintf(risposta, sizeof(risposta), "Errore di connessione.");
    stampa_risposta("SCHEDE ALLENAMENTO", risposta);
}

void mostra_piani_server(int id_cliente)
{
    char cmd[BUFFER_SIZE], risposta[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "GET_PIANI_ALIMENTARI|%d", id_cliente);
    if (!invia_comando(cmd, risposta, sizeof(risposta)))
        snprintf(risposta, sizeof(risposta), "Errore di connessione.");
    stampa_risposta("PIANI ALIMENTARI", risposta);
}

void mostra_lista_utenti(void)
{
    char risposta[BUFFER_SIZE];
    if (!invia_comando("GET_UTENTI", risposta, sizeof(risposta)))
        snprintf(risposta, sizeof(risposta), "Errore di connessione.");
    stampa_risposta("LISTA CLIENTI", risposta);
}
