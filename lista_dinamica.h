#ifndef LISTA_DINAMICA_H
#define LISTA_DINAMICA_H


/*
 * lista_dinamica.h
 * Strutture dati dinamiche (malloc, realloc, free).
 */

#include "client_modello.h"

/* LISTA COLLEGATA - storico pesi */
typedef struct NodoPeso {
    char  data[32];
    float peso_kg;
    struct NodoPeso *next;
} NodoPeso;

NodoPeso *carica_storico_dinamico(int id_cliente);

void libera_storico_dinamico(NodoPeso *testa);

void carica_storico_dinamico_in_buffer(int id_cliente, char *buf, int buf_size);

/* ARRAY DINAMICO - lista clienti (cresce con realloc) */
typedef struct {
    char  nome[DIM_NOME];
    char  cognome[DIM_NOME];
    int   eta;
    char  sesso;
    float peso_kg;
    float altezza_cm;
} ClienteInfo;

ClienteInfo *carica_lista_clienti_dinamica(int *out_count);

void libera_lista_clienti_dinamica(ClienteInfo *arr);

#endif /* LISTA_DINAMICA_H */