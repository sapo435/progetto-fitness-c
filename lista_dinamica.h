#ifndef LISTA_DINAMICA_H
#define LISTA_DINAMICA_H

/*
 * lista_dinamica.h
 * ----------------
 * Strutture dati dinamiche con gestione esplicita della memoria
 * (malloc / realloc / free). Estensione non richiesta dalla traccia.
 *
 * Usate per:
 *   - Storico pesi del cliente  -> lista collegata caricata dal DB
 *   - Lista clienti del trainer -> array dinamico (raddoppiato con
 *                                  realloc) caricato dal DB
 */

#include "client_modello.h"

/* ================================================================
 *  LISTA COLLEGATA — storico pesi
 * ================================================================ */
typedef struct NodoPeso {
    char  data[32];
    float peso_kg;
    struct NodoPeso *next;
} NodoPeso;

/* Carica lo storico pesi di un cliente come lista collegata.
   Ogni nodo e' allocato con malloc(); il chiamante DEVE liberare
   la lista con libera_storico_dinamico(). Ritorna NULL se vuoto
   o in caso di errore. */
NodoPeso *carica_storico_dinamico(int id_cliente);

/* Libera tutti i nodi della lista (free su ciascun nodo). */
void libera_storico_dinamico(NodoPeso *testa);

/* Comodita' per la GUI: costruisce la lista, la formatta in un
   buffer di testo gia' pronto da disegnare, poi libera la memoria
   internamente. Il chiamante non deve preoccuparsi della lista. */
void carica_storico_dinamico_in_buffer(int id_cliente, char *buf, int buf_size);

/* ================================================================
 *  ARRAY DINAMICO — lista clienti (cresce con realloc)
 * ================================================================ */
typedef struct {
    char  nome[DIM_NOME];
    char  cognome[DIM_NOME];
    int   eta;
    char  sesso;
    float peso_kg;
    float altezza_cm;
} ClienteInfo;

/* Carica tutti i clienti in un array allocato dinamicamente
   (capacita' iniziale 4, raddoppiata con realloc() quando serve).
   *out_count riceve il numero di elementi validi.
   Il chiamante DEVE liberare l'array con libera_lista_clienti_dinamica(). */
ClienteInfo *carica_lista_clienti_dinamica(int *out_count);

/* Libera l'array (free). */
void libera_lista_clienti_dinamica(ClienteInfo *arr);

#endif /* LISTA_DINAMICA_H */