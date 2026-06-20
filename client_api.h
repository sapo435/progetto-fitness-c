#ifndef CLIENT_API_H
#define CLIENT_API_H

/*
 * client_api.h
 * ------------
 * Comunicazione col server. Il token di sessione viene incluso
 * come primo parametro in ogni comando (tranne login e registrazione).
 */

#include "client_modello.h"

/* Login: ritorna l'ID utente, scrive ruolo e token. */
int  login_server(const char *username, const char *password,
                  char *ruolo_out, int ruolo_size,
                  char *token_out, int token_size);

/* Logout: invalida il token sul server. */
int  logout_server(const char *token);

/* Registrazione (non richiede token — avviene prima del login). */
int  registra_utente_server(const Utente *u, const char *username,
                            const char *password);

/* Tutte le funzioni seguenti richiedono il token di sessione. */
int  salva_storico_server(const char *token, int id_cliente, float peso, const char *data);
int  salva_piano_server(const char *token, int id_cliente, float kcal_target, const char *note);
int  salva_scheda_server(const char *token, const char *titolo, int id_cliente);
int  salva_dettaglio_scheda_server(const char *token, int id_scheda, int id_esercizio,
                                   int serie, int rip, int recupero);
void mostra_storico_server(const char *token, int id_cliente);
void mostra_schede_server(const char *token, int id_cliente);
void mostra_piani_server(const char *token, int id_cliente);
void mostra_lista_utenti(const char *token);

#endif
