#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include "sqlite3.h"

// Lifecycle
sqlite3* inizializza_database(const char* db_path);
void chiudi_database(sqlite3* db);
void crea_tabelle(sqlite3* db);

//INSERIMENTO
int inserisci_utente_db(sqlite3* db, const char* nome, const char* cognome, const char* ruolo, int eta, const char* sesso, float peso, float altezza, float bmr, float tdee);
int inserisci_alimento(sqlite3* db, const char* nome, float kcal, float pro, float carb, float grassi);
int inserisci_esercizio(sqlite3* db, const char* nome, const char* gruppo, const char* desc);
int inserisci_scheda(sqlite3* db, const char* titolo, int id_cliente);
int inserisci_dettaglio_scheda(sqlite3* db, int id_scheda, int id_esercizio, int serie, int rip, int rec);
int inserisci_storico(sqlite3* db, int id_cliente, float peso, const char* data);
int inserisci_piani_alimentari(sqlite3* db, int id_cliente, float kcal_target, const char* note);

//GET
// CRUD - Operazioni di Lettura
void get_utenti(sqlite3 *db, char *output, int output_size);
void get_alimenti(sqlite3 *db, char *output, int output_size);
void get_esercizi(sqlite3 *db, char *output, int output_size);
void get_schede(sqlite3 *db, int id_cliente, char *output, int output_size);
void get_storico(sqlite3 *db, int id_cliente, char *output, int output_size);
void get_piani_alimentari(sqlite3 *db, int id_cliente, char *output, int output_size);
void get_dettagli_scheda(sqlite3 *db, int id_scheda, char *output, int output_size);

#endif