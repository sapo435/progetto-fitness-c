#ifndef APP_DB_H
#define APP_DB_H

#include "sqlite3.h"
#include "client_modello.h"

extern sqlite3 *g_db;

int  app_db_init(const char *path);
void app_db_close(void);

int  app_login(const char *username, const char *password,
               char *ruolo_out, int ruolo_size);
int  app_registra(const Utente *u, const char *username, const char *password);

int  app_cerca_cliente_id_per_nome(const char *ricerca,
                                    char *nome_trovato, int nome_size);

/* Storico */
int  app_salva_storico(int id_cliente, float peso, const char *data);

/* Schede - ADD (aggiunge) */
int  app_salva_scheda(const char *titolo, int id_cliente);
/* Schede - REPLACE semplice (cancella + inserisce) */
int  app_sostituisci_scheda(const char *titolo, int id_cliente);
/* Schede - REPLACE con variante (salva "V:N:nome", riletto dal cliente) */
int  app_sostituisci_scheda_ex(int id_cliente, int variante,
                                const char *nome_programma);
/* Ritorna variante (0-2) assegnata dal trainer, -1 se nessuna */
int  app_get_variante_scheda(int id_cliente);

/* Piani - ADD */
int  app_salva_piano(int id_cliente, float kcal_target, const char *note);
/* Piani - REPLACE semplice */
int  app_sostituisci_piano(int id_cliente, float kcal_target, const char *note);
/* Piani - REPLACE con variante (salva "V:N:nome" in note_dieta) */
int  app_sostituisci_piano_ex(int id_cliente, int variante,
                               float kcal_target, const char *nome_piano);
/* Ritorna variante (0-2) assegnata dal trainer, -1 se nessuna */
int  app_get_variante_piano(int id_cliente);

/* Lettura */
void app_get_storico(int id_cliente, char *buf, int buf_size);
void app_get_schede(int id_cliente,  char *buf, int buf_size);
void app_get_piani(int id_cliente,   char *buf, int buf_size);
void app_get_clienti(char *buf, int buf_size);
int  app_get_utente(int id, Utente *u_out);

#endif