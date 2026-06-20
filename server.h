#ifndef SERVER_H
#define SERVER_H

/*
 * server.h
 * --------
 * Funzioni di comunicazione col server: login, registrazione,
 * salvataggio e visualizzazione dati remoti.
 */

#include "modello.h"

int  login_server(const char *username, const char *password,
                  char *ruolo_out, int ruolo_size);
int  registra_utente_server(const Utente *u, const char *username,
                            const char *password);
int  salva_storico_server(int id_cliente, float peso, const char *data);
int  salva_piano_server(int id_cliente, float kcal_target, const char *note);
int  salva_scheda_server(const char *titolo, int id_cliente);
int  salva_dettaglio_scheda_server(int id_scheda, int id_esercizio,
                                   int serie, int rip, int recupero);
void mostra_storico_server(int id_cliente);
void mostra_schede_server(int id_cliente);
void mostra_piani_server(int id_cliente);
void mostra_lista_utenti(void);

#endif /* SERVER_H */
