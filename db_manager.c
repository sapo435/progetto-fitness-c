#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "db_manager.h"

static void esegui_query(sqlite3 *db, const char *sql) {
    char *errore = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &errore) != SQLITE_OK) {
        fprintf(stderr, "SQL Error: %s\n", errore);
        sqlite3_free(errore);
    }
}

sqlite3* inizializza_database(const char* db_path) {
    sqlite3 *db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) return NULL;
    esegui_query(db, "PRAGMA foreign_keys = ON;");
    return db;
}

void crea_tabelle(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS UTENTI (ID INTEGER PRIMARY KEY AUTOINCREMENT, NOME TEXT, COGNOME TEXT, RUOLO TEXT, ETA INTEGER, SESSO TEXT, PESO REAL, ALTEZZA REAL, BMR REAL, TDEE REAL);"
        "CREATE TABLE IF NOT EXISTS CREDENZIALI (ID_UTENTE INTEGER PRIMARY KEY, USERNAME TEXT UNIQUE NOT NULL, PASSWORD TEXT NOT NULL, FOREIGN KEY(ID_UTENTE) REFERENCES UTENTI(ID) ON DELETE CASCADE);"
        "CREATE TABLE IF NOT EXISTS ESERCIZI (ID INTEGER PRIMARY KEY AUTOINCREMENT, NOME TEXT, GRUPPO_MUSCOLARE TEXT, DESCRIZIONE TEXT);"
        "CREATE TABLE IF NOT EXISTS ALIMENTI (ID INTEGER PRIMARY KEY AUTOINCREMENT, NOME TEXT, KCAL REAL, PROTEINE REAL, CARBOIDRATI REAL, GRASSI REAL);"
        "CREATE TABLE IF NOT EXISTS SCHEDE_ALLENAMENTO (ID INTEGER PRIMARY KEY AUTOINCREMENT, TITOLO TEXT, ID_CLIENTE INTEGER, FOREIGN KEY(ID_CLIENTE) REFERENCES UTENTI(ID));"
        "CREATE TABLE IF NOT EXISTS DETTAGLI_SCHEDA (ID_SCHEDA INTEGER, ID_ESERCIZIO INTEGER, SERIE INTEGER, RIPETIZIONI INTEGER, RECUPERO_SEC INTEGER, FOREIGN KEY(ID_SCHEDA) REFERENCES SCHEDE_ALLENAMENTO(ID), FOREIGN KEY(ID_ESERCIZIO) REFERENCES ESERCIZI(ID));"
        "CREATE TABLE IF NOT EXISTS STORICO (ID INTEGER PRIMARY KEY AUTOINCREMENT, ID_CLIENTE INTEGER, DATA TEXT, PESO_REGISTRATO REAL, FOREIGN KEY(ID_CLIENTE) REFERENCES UTENTI(ID));"
        "CREATE TABLE IF NOT EXISTS PIANI_ALIMENTARI (ID INTEGER PRIMARY KEY AUTOINCREMENT, ID_CLIENTE INTEGER, KCAL_TARGET REAL, NOTE_DIETA TEXT, FOREIGN KEY(ID_CLIENTE) REFERENCES UTENTI(ID));"
        "CREATE TABLE IF NOT EXISTS SESSIONI ( TOKEN TEXT PRIMARY KEY , ID_UTENTE INTEGER , SCADENZA INTEGER , FOREIGN KEY (ID_UTENTE) REFERENCES UTENTI(ID));";

    esegui_query(db, sql);
}

int inserisci_utente_db(sqlite3* db, const char* nome, const char* cognome, const char* ruolo, int eta, const char* sesso, float peso, float altezza, float bmr, float tdee) {
    const char* sql = "INSERT INTO UTENTI (NOME, COGNOME, RUOLO, ETA, SESSO, PESO, ALTEZZA, BMR, TDEE) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, cognome, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, ruolo, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, eta);
    sqlite3_bind_text(stmt, 5, sesso, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 6, peso);
    sqlite3_bind_double(stmt, 7, altezza);
    sqlite3_bind_double(stmt, 8, bmr);
    sqlite3_bind_double(stmt, 9, tdee);
    int rc = sqlite3_step(stmt); sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

int inserisci_alimento(sqlite3* db, const char* nome, float kcal, float pro, float carb, float grassi) {
    const char* sql = "INSERT INTO ALIMENTI (NOME, KCAL, PROTEINE, CARBOIDRATI, GRASSI) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, kcal);
    sqlite3_bind_double(stmt, 3, pro);
    sqlite3_bind_double(stmt, 4, carb);
    sqlite3_bind_double(stmt, 5, grassi);
    int rc = sqlite3_step(stmt); sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

int inserisci_esercizio(sqlite3* db, const char* nome, const char* gruppo, const char* desc) {
    const char* sql = "INSERT INTO ESERCIZI (NOME, GRUPPO_MUSCOLARE, DESCRIZIONE) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, gruppo, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, desc, -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt); sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

int inserisci_scheda(sqlite3* db, const char* titolo, int id_cliente) {
    const char* sql = "INSERT INTO SCHEDE_ALLENAMENTO (TITOLO, ID_CLIENTE) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, titolo, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, id_cliente);
    int rc = sqlite3_step(stmt); sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

int inserisci_dettaglio_scheda(sqlite3* db, int id_scheda, int id_esercizio, int serie, int rip, int rec) {
    const char* sql = "INSERT INTO DETTAGLI_SCHEDA (ID_SCHEDA, ID_ESERCIZIO, SERIE, RIPETIZIONI, RECUPERO_SEC) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, id_scheda);
    sqlite3_bind_int(stmt, 2, id_esercizio);
    sqlite3_bind_int(stmt, 3, serie);
    sqlite3_bind_int(stmt, 4, rip);
    sqlite3_bind_int(stmt, 5, rec);
    int rc = sqlite3_step(stmt); sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

int inserisci_storico(sqlite3* db, int id_cliente, float peso, const char* data) {
    const char* sql = "INSERT INTO STORICO (ID_CLIENTE, PESO_REGISTRATO, DATA) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, id_cliente);
    sqlite3_bind_double(stmt, 2, peso);
    sqlite3_bind_text(stmt, 3, data, -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt); sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}
int inserisci_piani_alimentari(sqlite3* db, int id_cliente , float kcal_target , const char* note) {
    const char* sql = "INSERT INTO PIANI_ALIMENTARI (ID_CLIENTE, KCAL_TARGET, NOTE_DIETA) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, id_cliente);
    sqlite3_bind_double(stmt, 2, kcal_target);
    sqlite3_bind_text(stmt, 3, note, -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);


}
int inserisci_credenziali(sqlite3* db,int id_utente,  const char* username , const char* password) {
    const char* sql = "INSERT INTO CREDENZIALI (ID_UTENTE, USERNAME, PASSWORD) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, id_utente);
    sqlite3_bind_text(stmt, 2, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3,password , -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}
int inserisci_sessione(sqlite3* db, const char* token, int id_cliente,long  scadenza) {
    const char* sql ="INSERT INTO SESSIONI (TOKEN,ID_UTENTE,SCADENZA) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, id_cliente);
    sqlite3_bind_int64(stmt, 3, scadenza);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

int verifica_login(sqlite3 *db, const char* username, const char* password) {
    const char* sql = "SELECT ID_UTENTE FROM CREDENZIALI WHERE USERNAME = ? AND PASSWORD = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2,password , -1, SQLITE_TRANSIENT);
    int id_utente = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id_utente = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return id_utente;
}

int valida_sessione(sqlite3* db, const char* token) {
    const char* sql= "SELECT ID_UTENTE FROM SESSIONI WHERE TOKEN = ? AND SCADENZA > ? ";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, (long)time(NULL));
    int id_utente = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id_utente = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return id_utente;;
}
void genera_token(char* output, int output_size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < output_size-1; i++) {
        output[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    output[output_size-1] = '\0';
}

void chiudi_database(sqlite3* db) { if (db) sqlite3_close(db); }