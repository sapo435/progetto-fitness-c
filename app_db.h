#ifndef APP_DB_H
#define APP_DB_H

/**
 * @file app_db.h
 * @brief Strato di accesso al database SQLite: autenticazione,
 *        gestione utenti, storico pesi, schede e piani alimentari.
 */

#include "sqlite3.h"
#include "client_modello.h"

/** @brief Handle globale del database SQLite, aperto da app_db_init(). */
extern sqlite3 *g_db;

/**
 * @brief Apre (o crea) il database e inizializza lo schema delle tabelle.
 * @param path Percorso del file .db.
 * @return 1 in caso di successo, 0 in caso di errore.
 */
int  app_db_init(const char *path);

/** @brief Chiude la connessione al database. */
void app_db_close(void);

/**
 * @brief Verifica le credenziali di accesso.
 * @param username Username inserito.
 * @param password Password inserita.
 * @param[out] ruolo_out Buffer in cui viene scritto il ruolo ("cliente" o "trainer").
 * @param ruolo_size Dimensione del buffer ruolo_out.
 * @return ID dell'utente se le credenziali sono corrette, -1 altrimenti.
 */
int  app_login(const char *username, const char *password,
               char *ruolo_out, int ruolo_size);

/**
 * @brief Registra un nuovo utente nel database (dati anagrafici + credenziali).
 * @param u Dati anagrafici e metriche del nuovo utente.
 * @param username Username scelto.
 * @param password Password scelta.
 * @param ruolo "cliente" o "trainer".
 * @return ID del nuovo utente, -1 in caso di errore (es. username duplicato).
 */
int  app_registra(const Utente *u, const char *username, const char *password,
                   const char *ruolo);

/**
 * @brief Cerca un cliente per nome, cognome o username (ricerca parziale).
 * @param ricerca Testo da cercare.
 * @param[out] nome_trovato Buffer in cui viene scritto "Nome Cognome" del cliente trovato.
 * @param nome_size Dimensione del buffer nome_trovato.
 * @return ID del cliente trovato, -1 se nessun cliente corrisponde.
 */
int  app_cerca_cliente_id_per_nome(const char *ricerca,
                                    char *nome_trovato, int nome_size);

/**
 * @brief Salva una nuova registrazione di peso nello storico e aggiorna
 *        il peso corrente dell'utente.
 * @param id_cliente ID del cliente.
 * @param peso Peso registrato in chilogrammi.
 * @param data Data della registrazione (formato "YYYY-MM-DD").
 * @return 1 in caso di successo, 0 in caso di errore.
 */
int  app_salva_storico(int id_cliente, float peso, const char *data);

/**
 * @brief Aggiunge una scheda manuale senza cancellare le precedenti.
 * @param titolo Titolo della scheda.
 * @param id_cliente ID del cliente a cui assegnarla.
 * @return ID della scheda creata, -1 in caso di errore.
 */
int  app_salva_scheda(const char *titolo, int id_cliente);

/**
 * @brief Cancella tutte le schede del cliente e ne salva una nuova.
 * @param titolo Titolo della nuova scheda.
 * @param id_cliente ID del cliente.
 * @return ID della scheda creata, -1 in caso di errore.
 */
int  app_sostituisci_scheda(const char *titolo, int id_cliente);

/**
 * @brief Sostituisce la scheda del cliente con una variante generata
 *        automaticamente (salvata internamente come "V:N:NomeProgramma").
 * @param id_cliente ID del cliente.
 * @param variante Indice della variante (0-2).
 * @param nome_programma Nome del programma generato.
 * @return ID della scheda creata, -1 in caso di errore.
 */
int  app_sostituisci_scheda_ex(int id_cliente, int variante,
                                const char *nome_programma);

/**
 * @brief Legge la variante di scheda assegnata dal trainer.
 * @param id_cliente ID del cliente.
 * @return Indice variante (0-2), -1 se non e' stata assegnata alcuna variante.
 */
int  app_get_variante_scheda(int id_cliente);

/**
 * @brief Aggiunge un piano nutrizionale manuale senza cancellare i precedenti.
 * @param id_cliente ID del cliente a cui assegnarlo.
 * @param kcal_target Kcal obiettivo giornaliere.
 * @param note Note testuali sul piano.
 * @return 1 in caso di successo, 0 in caso di errore.
 */
int  app_salva_piano(int id_cliente, float kcal_target, const char *note);

/**
 * @brief Cancella tutti i piani del cliente e ne salva uno nuovo.
 * @param id_cliente ID del cliente.
 * @param kcal_target Kcal obiettivo giornaliere.
 * @param note Note testuali sul piano.
 * @return 1 in caso di successo, 0 in caso di errore.
 */
int  app_sostituisci_piano(int id_cliente, float kcal_target, const char *note);

/**
 * @brief Sostituisce il piano del cliente con una variante generata
 *        automaticamente (salvata internamente come "V:N:NomePiano").
 * @param id_cliente ID del cliente.
 * @param variante Indice della variante (0-2).
 * @param kcal_target Kcal obiettivo giornaliere.
 * @param nome_piano Nome del piano generato.
 * @return 1 in caso di successo, 0 in caso di errore.
 */
int  app_sostituisci_piano_ex(int id_cliente, int variante,
                               float kcal_target, const char *nome_piano);

/**
 * @brief Legge la variante di piano assegnata dal trainer.
 * @param id_cliente ID del cliente.
 * @return Indice variante (0-2), -1 se non e' stata assegnata alcuna variante.
 */
int  app_get_variante_piano(int id_cliente);

/**
 * @brief Legge lo storico pesi di un cliente e lo formatta in testo.
 * @param id_cliente ID del cliente.
 * @param[out] buf Buffer di destinazione.
 * @param buf_size Dimensione del buffer.
 */
void app_get_storico(int id_cliente, char *buf, int buf_size);

/**
 * @brief Legge le schede manuali (non generate automaticamente) di un cliente.
 * @param id_cliente ID del cliente.
 * @param[out] buf Buffer di destinazione.
 * @param buf_size Dimensione del buffer.
 */
void app_get_schede(int id_cliente,  char *buf, int buf_size);

/**
 * @brief Legge i piani manuali (non generati automaticamente) di un cliente.
 * @param id_cliente ID del cliente.
 * @param[out] buf Buffer di destinazione.
 * @param buf_size Dimensione del buffer.
 */
void app_get_piani(int id_cliente,   char *buf, int buf_size);

/**
 * @brief Legge l'elenco di tutti i clienti registrati e lo formatta in testo.
 * @param[out] buf Buffer di destinazione.
 * @param buf_size Dimensione del buffer.
 */
void app_get_clienti(char *buf, int buf_size);

/**
 * @brief Carica i dati anagrafici e le metriche salvate di un utente dal database.
 * @param id ID dell'utente.
 * @param[out] u_out Struttura in cui vengono scritti i dati letti.
 * @return 1 se l'utente e' stato trovato, 0 altrimenti.
 */
int  app_get_utente(int id, Utente *u_out);

#endif