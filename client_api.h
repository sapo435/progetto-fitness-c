/*
 * client_api.h
 * ------------
 * Dichiarazioni delle funzioni di comunicazione col server.
 * Le funzioni *_server stampano su stdout (interfaccia testuale).
 * Le funzioni *_buf scrivono la risposta in un buffer fornito
 * dal chiamante (interfaccia grafica Raylib).
 */

#ifndef CLIENT_API_H
#define CLIENT_API_H

#include "client_modello.h"

/* ---- Autenticazione ---- */
int  login_server(const char *username, const char *password,
                  char *ruolo_out, int ruolo_size,
                  char *token_out, int token_size);
int  logout_server(const char *token);

/* ---- Registrazione ---- */
int  registra_utente_server(const Utente *u, const char *username,
                             const char *password);

/* ---- Salvataggio dati (richiedono token) ---- */
int  salva_storico_server(const char *token, int id_cliente,
                           float peso, const char *data);
int  salva_piano_server(const char *token, int id_cliente,
                         float kcal_target, const char *note);
int  salva_scheda_server(const char *token, const char *titolo,
                          int id_cliente);
int  salva_dettaglio_scheda_server(const char *token, int id_scheda,
                                    int id_esercizio, int serie,
                                    int rip, int recupero);

/* ---- Lettura dati — varianti stdout (interfaccia testuale) ---- */
void mostra_storico_server(const char *token, int id_cliente);
void mostra_schede_server(const char *token, int id_cliente);
void mostra_piani_server(const char *token, int id_cliente);
void mostra_lista_utenti(const char *token);

/* ---- Lettura dati — varianti buffer (interfaccia grafica Raylib) ----
 *
 * Scrivono la risposta del server direttamente in buf invece di
 * stamparla con printf. buf deve essere allocato dal chiamante;
 * buf_size indica la dimensione massima incluso il terminatore.
 * In caso di errore di connessione scrivono un messaggio di errore.
 */
void mostra_storico_buf(const char *token, int id_cliente,
                        char *buf, int buf_size);
void mostra_schede_buf(const char *token, int id_cliente,
                       char *buf, int buf_size);
void mostra_piani_buf(const char *token, int id_cliente,
                      char *buf, int buf_size);
void mostra_lista_utenti_buf(const char *token,
                             char *buf, int buf_size);

#endif /* CLIENT_API_H */
