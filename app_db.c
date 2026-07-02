#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "app_db.h"

sqlite3 *g_db = NULL;

static void esegui(const char *sql)
{
    char *err = NULL;
    if (sqlite3_exec(g_db, sql, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
    }
}

int app_db_init(const char *path)
{
    if (sqlite3_open(path, &g_db) != SQLITE_OK) return 0;
    esegui("PRAGMA foreign_keys = ON;");
    /* Di default LIKE in SQLite ignora maiuscole/minuscole (solo ASCII):
       senza questa pragma "Mario" e "mario" sarebbero indistinguibili
       nella ricerca cliente. La attiviamo per poterli distinguere. */
    esegui("PRAGMA case_sensitive_like = ON;");
    esegui(
        "CREATE TABLE IF NOT EXISTS UTENTI ("
        "  ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  NOME TEXT, COGNOME TEXT, RUOLO TEXT,"
        "  ETA INTEGER, SESSO TEXT,"
        "  PESO REAL, ALTEZZA REAL, BMR REAL, TDEE REAL);"

        "CREATE TABLE IF NOT EXISTS CREDENZIALI ("
        "  ID_UTENTE INTEGER PRIMARY KEY,"
        "  USERNAME TEXT UNIQUE NOT NULL, PASSWORD TEXT NOT NULL,"
        "  FOREIGN KEY(ID_UTENTE) REFERENCES UTENTI(ID) ON DELETE CASCADE);"

        "CREATE TABLE IF NOT EXISTS STORICO ("
        "  ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  ID_CLIENTE INTEGER, DATA TEXT, PESO_REGISTRATO REAL,"
        "  FOREIGN KEY(ID_CLIENTE) REFERENCES UTENTI(ID));"

        "CREATE TABLE IF NOT EXISTS PIANI_ALIMENTARI ("
        "  ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  ID_CLIENTE INTEGER, KCAL_TARGET REAL, NOTE_DIETA TEXT,"
        "  FOREIGN KEY(ID_CLIENTE) REFERENCES UTENTI(ID));"

        "CREATE TABLE IF NOT EXISTS SCHEDE_ALLENAMENTO ("
        "  ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  TITOLO TEXT, ID_CLIENTE INTEGER,"
        "  FOREIGN KEY(ID_CLIENTE) REFERENCES UTENTI(ID));"
    );
    esegui(
        "INSERT OR IGNORE INTO UTENTI"
        " (ID,NOME,COGNOME,RUOLO,ETA,SESSO,PESO,ALTEZZA,BMR,TDEE)"
        " VALUES (1,'Admin','Trainer','trainer',30,'M',80,180,1800,2200);"
    );
    esegui(
        "INSERT OR IGNORE INTO CREDENZIALI (ID_UTENTE,USERNAME,PASSWORD)"
        " VALUES (1,'trainer','trainer123');"
    );
    esegui("ALTER TABLE UTENTI ADD COLUMN SERVIZIO INTEGER DEFAULT 2;");
    /* FIX: peso_obiettivo_kg non veniva mai salvato nel DB, quindi ogni
       volta che l'utente veniva ricaricato (es. ricerca del trainer, o
       un nuovo login) l'obiettivo veniva ricalcolato con peso_obiettivo=0,
       facendo apparire sempre "Dimagrimento" indipendentemente da quanto
       scelto in fase di registrazione. */
    esegui("ALTER TABLE UTENTI ADD COLUMN PESO_OBIETTIVO REAL DEFAULT 0;");
    return 1;
}

void app_db_close(void) { if (g_db) { sqlite3_close(g_db); g_db = NULL; } }

/* AUTH */
int app_login(const char *username, const char *password,
              char *ruolo_out, int ruolo_size)
{
    const char *sql =
        "SELECT U.ID, U.RUOLO FROM CREDENZIALI C "
        "JOIN UTENTI U ON U.ID=C.ID_UTENTE "
        "WHERE C.USERNAME=? AND C.PASSWORD=?;";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql,-1,&s,NULL)!=SQLITE_OK) return -1;
    sqlite3_bind_text(s,1,username,-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(s,2,password,-1,SQLITE_TRANSIENT);
    int id=-1;
    if (sqlite3_step(s)==SQLITE_ROW) {
        id=sqlite3_column_int(s,0);
        strncpy(ruolo_out,(const char*)sqlite3_column_text(s,1),ruolo_size-1);
        ruolo_out[ruolo_size-1]='\0';
    }
    sqlite3_finalize(s); return id;
}

int app_registra(const Utente *u, const char *username, const char *password)
{
    const char *sql1=
        "INSERT INTO UTENTI"
        " (NOME,COGNOME,RUOLO,ETA,SESSO,PESO,ALTEZZA,BMR,TDEE,SERVIZIO,PESO_OBIETTIVO)"
        " VALUES (?,?,?,?,?,?,?,?,?,?,?);";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql1,-1,&s,NULL)!=SQLITE_OK) return -1;
    sqlite3_bind_text  (s,1,u->nome,-1,SQLITE_TRANSIENT);
    sqlite3_bind_text  (s,2,u->cognome,-1,SQLITE_TRANSIENT);
    sqlite3_bind_text  (s,3,"cliente",-1,SQLITE_TRANSIENT);
    sqlite3_bind_int   (s,4,u->eta);
    char sesso[2]={u->sesso,'\0'};
    sqlite3_bind_text  (s,5,sesso,-1,SQLITE_TRANSIENT);
    sqlite3_bind_double(s,6,u->peso_kg);
    sqlite3_bind_double(s,7,u->altezza_cm);
    sqlite3_bind_double(s,8,u->bmr);
    sqlite3_bind_double(s,9,u->tdee_kcal);
    sqlite3_bind_int   (s,10,(int)u->servizio);
    sqlite3_bind_double(s,11,u->peso_obiettivo_kg);
    int rc=sqlite3_step(s); sqlite3_finalize(s);
    if (rc!=SQLITE_DONE) return -1;
    int id=(int)sqlite3_last_insert_rowid(g_db);
    const char *sql2=
        "INSERT INTO CREDENZIALI (ID_UTENTE,USERNAME,PASSWORD) VALUES (?,?,?);";
    if (sqlite3_prepare_v2(g_db,sql2,-1,&s,NULL)!=SQLITE_OK) return -1;
    sqlite3_bind_int (s,1,id);
    sqlite3_bind_text(s,2,username,-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(s,3,password,-1,SQLITE_TRANSIENT);
    rc=sqlite3_step(s); sqlite3_finalize(s);
    return (rc==SQLITE_DONE)?id:-1;
}

/* RICERCA CLIENTE */
int app_cerca_cliente_id_per_nome(const char *ricerca,
                                   char *nome_trovato, int nome_size)
{
    if (!ricerca||strlen(ricerca)==0) return -1;
    char pattern[DIM_NOME+2];
    snprintf(pattern,sizeof(pattern),"%%%s%%",ricerca);
    const char *sql=
        "SELECT U.ID,U.NOME,U.COGNOME FROM UTENTI U "
        "LEFT JOIN CREDENZIALI C ON C.ID_UTENTE=U.ID "
        "WHERE U.RUOLO='cliente' AND ("
        "  U.NOME LIKE ? OR U.COGNOME LIKE ? OR C.USERNAME LIKE ?"
        ") ORDER BY U.ID LIMIT 1;";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql,-1,&s,NULL)!=SQLITE_OK) return -1;
    sqlite3_bind_text(s,1,pattern,-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(s,2,pattern,-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(s,3,pattern,-1,SQLITE_TRANSIENT);
    int id=-1;
    if (sqlite3_step(s)==SQLITE_ROW) {
        id=sqlite3_column_int(s,0);
        if (nome_trovato) {
            const char *nm=(const char*)sqlite3_column_text(s,1);
            const char *cg=(const char*)sqlite3_column_text(s,2);
            snprintf(nome_trovato,nome_size,"%s %s",nm?nm:"",cg?cg:"");
        }
    }
    sqlite3_finalize(s); return id;
}

/* STORICO */
int app_salva_storico(int id_cliente, float peso, const char *data)
{
    const char *sql=
        "INSERT INTO STORICO (ID_CLIENTE,PESO_REGISTRATO,DATA) VALUES (?,?,?);";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql,-1,&s,NULL)!=SQLITE_OK) return 0;
    sqlite3_bind_int   (s,1,id_cliente);
    sqlite3_bind_double(s,2,peso);
    sqlite3_bind_text  (s,3,data,-1,SQLITE_TRANSIENT);
    int rc=sqlite3_step(s); sqlite3_finalize(s);
    if (rc==SQLITE_DONE) {
        const char *upd="UPDATE UTENTI SET PESO=? WHERE ID=?;";
        if (sqlite3_prepare_v2(g_db,upd,-1,&s,NULL)==SQLITE_OK) {
            sqlite3_bind_double(s,1,peso);
            sqlite3_bind_int   (s,2,id_cliente);
            sqlite3_step(s); sqlite3_finalize(s);
        }
    }
    return (rc==SQLITE_DONE);
}

/* SCHEDE - ADD (nessuna cancellazione) */
int app_salva_scheda(const char *titolo, int id_cliente)
{
    const char *sql=
        "INSERT INTO SCHEDE_ALLENAMENTO (TITOLO,ID_CLIENTE) VALUES (?,?);";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql,-1,&s,NULL)!=SQLITE_OK) return -1;
    sqlite3_bind_text(s,1,titolo,-1,SQLITE_TRANSIENT);
    sqlite3_bind_int (s,2,id_cliente);
    int rc=sqlite3_step(s); sqlite3_finalize(s);
    return (rc==SQLITE_DONE)?(int)sqlite3_last_insert_rowid(g_db):-1;
}

/* SCHEDE - REPLACE con variante
 *  Il titolo viene salvato come "V:N:NomeProgramma" dove N = 0,1,2.
 *  app_get_variante_scheda() lo legge e restituisce N. */
int app_sostituisci_scheda(const char *titolo, int id_cliente)
{
    /* Cancella tutte le schede precedenti */
    const char *del="DELETE FROM SCHEDE_ALLENAMENTO WHERE ID_CLIENTE=?;";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,del,-1,&s,NULL)!=SQLITE_OK) return -1;
    sqlite3_bind_int(s,1,id_cliente);
    sqlite3_step(s); sqlite3_finalize(s);
    return app_salva_scheda(titolo, id_cliente);
}

int app_sostituisci_scheda_ex(int id_cliente, int variante,
                               const char *nome_programma)
{
    char titolo[DIM_NOME];
    snprintf(titolo, sizeof(titolo), "V:%d:%s", variante, nome_programma);
    return app_sostituisci_scheda(titolo, id_cliente);
}

/* Legge la variante scheda assegnata dal trainer. */
int app_get_variante_scheda(int id_cliente)
{
    const char *sql=
        "SELECT TITOLO FROM SCHEDE_ALLENAMENTO "
        "WHERE ID_CLIENTE=? ORDER BY ID DESC LIMIT 1;";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql,-1,&s,NULL)!=SQLITE_OK) return -1;
    sqlite3_bind_int(s,1,id_cliente);
    int variante=-1;
    if (sqlite3_step(s)==SQLITE_ROW) {
        const char *t=(const char*)sqlite3_column_text(s,0);
        if (t && strncmp(t,"V:",2)==0)
            variante=atoi(t+2);
    }
    sqlite3_finalize(s); return variante;
}

/* PIANI - ADD */
int app_salva_piano(int id_cliente, float kcal_target, const char *note)
{
    const char *sql=
        "INSERT INTO PIANI_ALIMENTARI (ID_CLIENTE,KCAL_TARGET,NOTE_DIETA)"
        " VALUES (?,?,?);";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql,-1,&s,NULL)!=SQLITE_OK) return 0;
    sqlite3_bind_int   (s,1,id_cliente);
    sqlite3_bind_double(s,2,kcal_target);
    sqlite3_bind_text  (s,3,note,-1,SQLITE_TRANSIENT);
    int rc=sqlite3_step(s); sqlite3_finalize(s);
    return (rc==SQLITE_DONE);
}

/* PIANI - REPLACE con variante
 *  Note_dieta = "V:N:NomePiano" */
int app_sostituisci_piano(int id_cliente, float kcal_target, const char *note)
{
    const char *del="DELETE FROM PIANI_ALIMENTARI WHERE ID_CLIENTE=?;";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,del,-1,&s,NULL)!=SQLITE_OK) return 0;
    sqlite3_bind_int(s,1,id_cliente);
    sqlite3_step(s); sqlite3_finalize(s);
    return app_salva_piano(id_cliente, kcal_target, note);
}

int app_sostituisci_piano_ex(int id_cliente, int variante,
                              float kcal_target, const char *nome_piano)
{
    char note[DIM_TESTO];
    snprintf(note, sizeof(note), "V:%d:%s", variante, nome_piano);
    return app_sostituisci_piano(id_cliente, kcal_target, note);
}

/* Legge la variante piano assegnata dal trainer. -1 se nessuna. */
int app_get_variante_piano(int id_cliente)
{
    const char *sql=
        "SELECT NOTE_DIETA FROM PIANI_ALIMENTARI "
        "WHERE ID_CLIENTE=? ORDER BY ID DESC LIMIT 1;";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql,-1,&s,NULL)!=SQLITE_OK) return -1;
    sqlite3_bind_int(s,1,id_cliente);
    int variante=-1;
    if (sqlite3_step(s)==SQLITE_ROW) {
        const char *t=(const char*)sqlite3_column_text(s,0);
        if (t && strncmp(t,"V:",2)==0)
            variante=atoi(t+2);
    }
    sqlite3_finalize(s); return variante;
}

/* LETTURA */
void app_get_storico(int id_cliente, char *buf, int buf_size)
{
    buf[0]='\0';
    const char *sql=
        "SELECT DATA,PESO_REGISTRATO FROM STORICO "
        "WHERE ID_CLIENTE=? ORDER BY DATA;";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql,-1,&s,NULL)!=SQLITE_OK) return;
    sqlite3_bind_int(s,1,id_cliente);
    while (sqlite3_step(s)==SQLITE_ROW) {
        const char *data=(const char*)sqlite3_column_text(s,0);
        double peso=sqlite3_column_double(s,1);
        char riga[128];
        snprintf(riga,sizeof(riga),"%s  ->  %.1f kg\n",data,peso);
        strncat(buf,riga,buf_size-strlen(buf)-1);
    }
    sqlite3_finalize(s);
}

/* Le schede con titolo "V:N:..." sono generate automaticamente e mostrate */
void app_get_schede(int id_cliente, char *buf, int buf_size)
{
    buf[0]='\0';
    const char *sql=
        "SELECT ID,TITOLO FROM SCHEDE_ALLENAMENTO WHERE ID_CLIENTE=?;";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql,-1,&s,NULL)!=SQLITE_OK) return;
    sqlite3_bind_int(s,1,id_cliente);
    while (sqlite3_step(s)==SQLITE_ROW) {
        int         id    =sqlite3_column_int(s,0);
        const char *titolo=(const char*)sqlite3_column_text(s,1);
        if (!titolo) continue;
        /* Salta le schede generate automaticamente (gia' mostrate come programma) */
        if (strncmp(titolo,"V:",2)==0) continue;
        char riga[256];
        snprintf(riga,sizeof(riga),"[ID %d]  %s\n",id,titolo);
        strncat(buf,riga,buf_size-strlen(buf)-1);
    }
    sqlite3_finalize(s);
}

/* I piani con NOTE_DIETA "V:N:..." sono gia' mostrati come piano completo. */
void app_get_piani(int id_cliente, char *buf, int buf_size)
{
    buf[0]='\0';
    const char *sql=
        "SELECT KCAL_TARGET,NOTE_DIETA FROM PIANI_ALIMENTARI WHERE ID_CLIENTE=?;";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql,-1,&s,NULL)!=SQLITE_OK) return;
    sqlite3_bind_int(s,1,id_cliente);
    while (sqlite3_step(s)==SQLITE_ROW) {
        double      kcal=sqlite3_column_double(s,0);
        const char *note=(const char*)sqlite3_column_text(s,1);
        if (note && strncmp(note,"V:",2)==0) continue;
        char riga[256];
        snprintf(riga,sizeof(riga),"%.0f kcal  -  %s\n",kcal,note?note:"");
        strncat(buf,riga,buf_size-strlen(buf)-1);
    }
    sqlite3_finalize(s);
}

void app_get_clienti(char *buf, int buf_size)
{
    buf[0]='\0';
    const char *sql=
        "SELECT NOME,COGNOME,ETA,SESSO,PESO,ALTEZZA "
        "FROM UTENTI WHERE RUOLO='cliente' ORDER BY ID;";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql,-1,&s,NULL)!=SQLITE_OK) return;
    while (sqlite3_step(s)==SQLITE_ROW) {
        const char *nome=(const char*)sqlite3_column_text(s,0);
        const char *cog =(const char*)sqlite3_column_text(s,1);
        int eta=sqlite3_column_int(s,2);
        const char *sex=(const char*)sqlite3_column_text(s,3);
        double peso=sqlite3_column_double(s,4);
        double alt =sqlite3_column_double(s,5);
        char riga[256];
        snprintf(riga,sizeof(riga),"%-15s %-15s  %d anni  %s  %.1fkg  %.0fcm\n",
                 nome,cog,eta,sex,peso,alt);
        strncat(buf,riga,buf_size-strlen(buf)-1);
    }
    sqlite3_finalize(s);
}

int app_get_utente(int id, Utente *u)
{
    const char *sql=
        "SELECT NOME,COGNOME,ETA,SESSO,PESO,ALTEZZA,BMR,TDEE,SERVIZIO,PESO_OBIETTIVO "
        "FROM UTENTI WHERE ID=?;";
    sqlite3_stmt *s;
    if (sqlite3_prepare_v2(g_db,sql,-1,&s,NULL)!=SQLITE_OK) return 0;
    sqlite3_bind_int(s,1,id);
    int ok=0;
    if (sqlite3_step(s)==SQLITE_ROW) {
        strncpy(u->nome,   (const char*)sqlite3_column_text(s,0),DIM_NOME-1);
        strncpy(u->cognome,(const char*)sqlite3_column_text(s,1),DIM_NOME-1);
        u->eta       =sqlite3_column_int(s,2);
        const char *sx=(const char*)sqlite3_column_text(s,3);
        u->sesso     =sx?sx[0]:'M';
        u->peso_kg   =(float)sqlite3_column_double(s,4);
        u->altezza_cm=(float)sqlite3_column_double(s,5);
        u->bmr       =(float)sqlite3_column_double(s,6);
        u->tdee_kcal =(float)sqlite3_column_double(s,7);
        u->servizio  =(Servizio)sqlite3_column_int(s,8);
        u->peso_obiettivo_kg=(float)sqlite3_column_double(s,9);
        ok=1;
    }
    sqlite3_finalize(s); return ok;
}