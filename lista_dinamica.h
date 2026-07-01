#ifndef LISTA_DINAMICA_H
#define LISTA_DINAMICA_H

/**
 * @file lista_dinamica.h
 * @brief Strutture dati dinamiche con gestione esplicita della memoria
 *        (malloc/realloc/free): lista collegata per lo storico pesi
 *        e array dinamico per la lista clienti del trainer.
 */

#include "client_modello.h"

/** @brief Nodo della lista collegata usata per lo storico pesi di un cliente. */
typedef struct NodoPeso {
    char  data[32];          /**< Data della registrazione (formato "YYYY-MM-DD"). */
    float peso_kg;           /**< Peso registrato in chilogrammi. */
    struct NodoPeso *next;   /**< Puntatore al nodo successivo, NULL se ultimo. */
} NodoPeso;

/**
 * @brief Carica lo storico pesi di un cliente dal database come lista
 *        collegata, allocando ogni nodo con malloc().
 * @param id_cliente ID del cliente.
 * @return Puntatore alla testa della lista (NULL se vuota o errore).
 *         Il chiamante deve liberarla con libera_storico_dinamico().
 */
NodoPeso *carica_storico_dinamico(int id_cliente);

/**
 * @brief Libera tutti i nodi della lista collegata (free su ciascun nodo).
 * @param testa Testa della lista da liberare.
 */
void libera_storico_dinamico(NodoPeso *testa);

/**
 * @brief Carica lo storico pesi come lista collegata, lo formatta in
 *        testo per la GUI e libera automaticamente la memoria usata.
 * @param id_cliente ID del cliente.
 * @param[out] buf Buffer di destinazione.
 * @param buf_size Dimensione del buffer.
 */
void carica_storico_dinamico_in_buffer(int id_cliente, char *buf, int buf_size);

/** @brief Dati sintetici di un cliente, usati nella lista clienti del trainer. */
typedef struct {
    char  nome[DIM_NOME];
    char  cognome[DIM_NOME];
    int   eta;
    char  sesso;
    float peso_kg;
    float altezza_cm;
} ClienteInfo;

/**
 * @brief Carica tutti i clienti dal database in un array allocato
 *        dinamicamente, che raddoppia la propria capacita' con
 *        realloc() man mano che serve.
 * @param[out] out_count Numero di clienti effettivamente caricati.
 * @return Puntatore all'array (NULL se errore). Il chiamante deve
 *         liberarlo con libera_lista_clienti_dinamica().
 */
ClienteInfo *carica_lista_clienti_dinamica(int *out_count);

/**
 * @brief Libera l'array di clienti allocato dinamicamente (free).
 * @param arr Array da liberare.
 */
void libera_lista_clienti_dinamica(ClienteInfo *arr);

#endif /* LISTA_DINAMICA_H */
