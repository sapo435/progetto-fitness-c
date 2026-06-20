#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #define CLOSE_SOCKET close
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

#include "network_manager.h"
#include "db_manager.h"

void avvia_server(sqlite3 *db, int porta) {

    #ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
            printf("WSAStartup fallito\n");
            return;
        }
    #endif

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Errore creazione socket\n");
        #ifdef _WIN32
            WSACleanup();
        #endif
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(porta);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind fallito\n");
        CLOSE_SOCKET(server_socket);
        #ifdef _WIN32
            WSACleanup();
        #endif
        return;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR) {
        printf("Listen fallito\n");
        CLOSE_SOCKET(server_socket);
        #ifdef _WIN32
            WSACleanup();
        #endif
        return;
    }
    printf("Server in ascolto sulla porta %d...\n", porta);

    while (1) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            printf("Errore accettazione client\n");
            continue;
        }
        gestisci_client(client_socket, db);
    }

    CLOSE_SOCKET(server_socket);
    #ifdef _WIN32
        WSACleanup();
    #endif
}

void gestisci_client(SOCKET client_socket, sqlite3 *db) {
    char buf[1024];
    int bytes = recv(client_socket, buf, sizeof(buf) - 1, 0);

    if (bytes <= 0) {
        CLOSE_SOCKET(client_socket);
        return;
    }

    buf[bytes] = '\0';
    buf[strcspn(buf, "\r\n")] = '\0';
    printf("[SERVER] Ricevuto dal client: %s\n", buf);

    char copia[1024];
    strcpy(copia, buf);
    char *comando = strtok(copia, "|");

    if (strcmp(comando, "AGGIUNGI_UTENTE") == 0) {
        char *nome      = strtok(NULL, "|");
        char *cognome   = strtok(NULL, "|");
        char *ruolo     = strtok(NULL, "|");
        char *eta_tok   = strtok(NULL, "|");
        int   eta       = atoi(eta_tok);
        char *sesso     = strtok(NULL, "|");
        char *peso_tok  = strtok(NULL, "|");
        float peso      = atof(peso_tok);
        char *alt_tok   = strtok(NULL, "|");
        float altezza   = atof(alt_tok);
        char *bmr_tok   = strtok(NULL, "|");
        float bmr       = atof(bmr_tok);
        char *tdee_tok  = strtok(NULL, "|");
        float tdee      = atof(tdee_tok);
        int ris = inserisci_utente_db(db, nome, cognome, ruolo, eta, sesso, peso, altezza, bmr, tdee);
        if (ris) {
            char id_risposta[32];
            snprintf(id_risposta, sizeof(id_risposta), "%d", (int)sqlite3_last_insert_rowid(db));
            send(client_socket, id_risposta, strlen(id_risposta), 0);
        } else     send(client_socket, "ERRORE", strlen("ERRORE"), 0);

    } else if (strcmp(comando, "AGGIUNGI_ALIMENTO") == 0) {
        char *nome        = strtok(NULL, "|");
        char *kcal_tok    = strtok(NULL, "|");
        float kcal        = atof(kcal_tok);
        char *pro_tok     = strtok(NULL, "|");
        float proteine    = atof(pro_tok);
        char *carb_tok    = strtok(NULL, "|");
        float carboidrati = atof(carb_tok);
        char *grassi_tok  = strtok(NULL, "|");
        float grassi      = atof(grassi_tok);
        int ris = inserisci_alimento(db, nome, kcal, proteine, carboidrati, grassi);
        if (ris) send(client_socket, "OK", strlen("OK"), 0);
        else     send(client_socket, "ERRORE", strlen("ERRORE"), 0);

    } else if (strcmp(comando, "AGGIUNGI_ESERCIZIO") == 0) {
        char *nome             = strtok(NULL, "|");
        char *gruppo_muscolare = strtok(NULL, "|");
        char *descrizione      = strtok(NULL, "|");
        int ris = inserisci_esercizio(db, nome, gruppo_muscolare, descrizione);
        if (ris) send(client_socket, "OK", strlen("OK"), 0);
        else     send(client_socket, "ERRORE", strlen("ERRORE"), 0);

    } else if (strcmp(comando, "AGGIUNGI_SCHEDA_ALLENAMENTO") == 0) {
        char *titolo     = strtok(NULL, "|");
        char *id_cl_tok  = strtok(NULL, "|");
        int   id_cliente = atoi(id_cl_tok);
        int ris = inserisci_scheda(db, titolo, id_cliente);
        if (ris) {
            char id_risposta[32];
            snprintf(id_risposta, sizeof(id_risposta), "%d", (int)sqlite3_last_insert_rowid(db));
            send(client_socket, id_risposta, strlen(id_risposta), 0);
        } else     send(client_socket, "ERRORE", strlen("ERRORE"), 0);

    } else if (strcmp(comando, "AGGIUNGI_DETTAGLI_SCHEDA") == 0) {
        char *id_scheda_tok = strtok(NULL, "|");
        int   id_scheda     = atoi(id_scheda_tok);
        char *id_es_tok     = strtok(NULL, "|");
        int   id_esercizio  = atoi(id_es_tok);
        char *serie_tok     = strtok(NULL, "|");
        int   serie         = atoi(serie_tok);
        char *rip_tok       = strtok(NULL, "|");
        int   ripetizioni   = atoi(rip_tok);
        char *rec_tok       = strtok(NULL, "|");
        int   recupero_sec  = atoi(rec_tok);
        int ris = inserisci_dettaglio_scheda(db, id_scheda, id_esercizio, serie, ripetizioni, recupero_sec);
        if (ris) send(client_socket, "OK", strlen("OK"), 0);
        else     send(client_socket, "ERRORE", strlen("ERRORE"), 0);

    } else if (strcmp(comando, "AGGIUNGI_STORICO") == 0) {
        char *id_cl_tok  = strtok(NULL, "|");
        int   id_cliente = atoi(id_cl_tok);
        char *peso_tok   = strtok(NULL, "|");
        float peso       = atof(peso_tok);
        char *data       = strtok(NULL, "|");
        int ris = inserisci_storico(db, id_cliente, peso, data);
        if (ris) send(client_socket, "OK", strlen("OK"), 0);
        else     send(client_socket, "ERRORE", strlen("ERRORE"), 0);

    } else if (strcmp(comando, "AGGIUNGI_PIANO_ALIMENTARE") == 0) {
        char *id_cl_tok   = strtok(NULL, "|");
        int   id_cliente  = atoi(id_cl_tok);
        char *kcal_tok    = strtok(NULL, "|");
        float kcal_target = atof(kcal_tok);
        char *note        = strtok(NULL, "|");
        int ris = inserisci_piani_alimentari(db, id_cliente, kcal_target, note);
        if (ris) send(client_socket, "OK", strlen("OK"), 0);
        else     send(client_socket, "ERRORE", strlen("ERRORE"), 0);

    } else if (strcmp(comando, "GET_UTENTI") == 0) {
        char output[4096];
        get_utenti(db, output, sizeof(output));
        send(client_socket, output, strlen(output), 0);

    } else if (strcmp(comando, "GET_SCHEDE") == 0) {
        char *id_cl_tok  = strtok(NULL, "|");
        int   id_cliente = atoi(id_cl_tok);
        char output[4096];
        get_schede(db, id_cliente, output, sizeof(output));
        send(client_socket, output, strlen(output), 0);

    } else if (strcmp(comando, "GET_STORICO") == 0) {
        char *id_cl_tok  = strtok(NULL, "|");
        int   id_cliente = atoi(id_cl_tok);
        char output[4096];
        get_storico(db, id_cliente, output, sizeof(output));
        send(client_socket, output, strlen(output), 0);

    } else if (strcmp(comando, "GET_ALIMENTI") == 0) {
        char output[4096];
        get_alimenti(db, output, sizeof(output));
        send(client_socket, output, strlen(output), 0);

    } else if (strcmp(comando, "GET_ESERCIZI") == 0) {
        char output[4096];
        get_esercizi(db, output, sizeof(output));
        send(client_socket, output, strlen(output), 0);

    } else if (strcmp(comando, "GET_PIANI_ALIMENTARI") == 0) {
        char *id_cl_tok  = strtok(NULL, "|");
        int   id_cliente = atoi(id_cl_tok);
        char output[4096];
        get_piani_alimentari(db, id_cliente, output, sizeof(output));
        send(client_socket, output, strlen(output), 0);

    } else if (strcmp(comando, "GET_DETTAGLI_SCHEDA") == 0) {
        char *id_scheda_tok = strtok(NULL, "|");
        int   id_scheda     = atoi(id_scheda_tok);
        char output[4096];
        get_dettagli_scheda(db, id_scheda, output, sizeof(output));
        send(client_socket, output, strlen(output), 0);

    } else if (strcmp(comando, "REGISTRA") == 0) {
        char *id_tok    = strtok(NULL, "|");
        int   id_utente = atoi(id_tok);
        char *username  = strtok(NULL, "|");
        char *password  = strtok(NULL, "|");
        int ris = inserisci_credenziali(db, id_utente, username, password);
        if (ris) send(client_socket, "OK", strlen("OK"), 0);
        else     send(client_socket, "ERRORE", strlen("ERRORE"), 0);

    } else if (strcmp(comando, "LOGIN") == 0) {
        char *username = strtok(NULL, "|");
        char *password = strtok(NULL, "|");
        int ris = verifica_login(db, username, password);
        if (ris != -1) {
            char ruolo[32];
            get_ruolo_utente(db, ris, ruolo, sizeof(ruolo));
            char risposta[64];
            snprintf(risposta, sizeof(risposta), "%d|%s", ris, ruolo);
            send(client_socket, risposta, strlen(risposta), 0);
        } else {
            send(client_socket, "ERRORE", strlen("ERRORE"), 0);
        }

    } else {
        send(client_socket, "COMANDO_SCONOSCIUTO", strlen("COMANDO_SCONOSCIUTO"), 0);
    }

    CLOSE_SOCKET(client_socket);
}

void get_utenti(sqlite3 *db, char *output, int output_size) {
    output[0] = '\0';
    const char *sql = "SELECT ID, NOME, COGNOME, RUOLO, ETA, SESSO, PESO, ALTEZZA, BMR, TDEE FROM UTENTI;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id              = sqlite3_column_int(stmt, 0);
        const char *nome    = sqlite3_column_text(stmt, 1);
        const char *cognome = sqlite3_column_text(stmt, 2);
        const char *ruolo   = sqlite3_column_text(stmt, 3);
        int eta             = sqlite3_column_int(stmt, 4);
        const char *sesso   = sqlite3_column_text(stmt, 5);
        double peso         = sqlite3_column_double(stmt, 6);
        double altezza      = sqlite3_column_double(stmt, 7);
        double bmr          = sqlite3_column_double(stmt, 8);
        double tdee         = sqlite3_column_double(stmt, 9);
        char riga[256];
        snprintf(riga, sizeof(riga), "%d|%s|%s|%s|%d|%s|%f|%f|%f|%f\n", id, nome, cognome, ruolo, eta, sesso, peso, altezza, bmr, tdee);
        strncat(output, riga, output_size - strlen(output) - 1);
    }
    sqlite3_finalize(stmt);
}

void get_alimenti(sqlite3 *db, char *output, int output_size) {
    output[0] = '\0';
    const char *sql = "SELECT NOME, KCAL, PROTEINE, CARBOIDRATI, GRASSI FROM ALIMENTI;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *nome   = sqlite3_column_text(stmt, 0);
        double kcal        = sqlite3_column_double(stmt, 1);
        double proteine    = sqlite3_column_double(stmt, 2);
        double carboidrati = sqlite3_column_double(stmt, 3);
        double grassi      = sqlite3_column_double(stmt, 4);
        char riga[256];
        snprintf(riga, sizeof(riga), "%s|%f|%f|%f|%f\n", nome, kcal, proteine, carboidrati, grassi);
        strncat(output, riga, output_size - strlen(output) - 1);
    }
    sqlite3_finalize(stmt);
}

void get_esercizi(sqlite3 *db, char *output, int output_size) {
    output[0] = '\0';
    const char *sql = "SELECT NOME, GRUPPO_MUSCOLARE, DESCRIZIONE FROM ESERCIZI;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *nome             = sqlite3_column_text(stmt, 0);
        const char *gruppo_muscolare = sqlite3_column_text(stmt, 1);
        const char *descrizione      = sqlite3_column_text(stmt, 2);
        char riga[256];
        snprintf(riga, sizeof(riga), "%s|%s|%s\n", nome, gruppo_muscolare, descrizione);
        strncat(output, riga, output_size - strlen(output) - 1);
    }
    sqlite3_finalize(stmt);
}

void get_schede(sqlite3 *db, int id_cliente, char *output, int output_size) {
    output[0] = '\0';
    const char *sql = "SELECT ID, TITOLO, ID_CLIENTE FROM SCHEDE_ALLENAMENTO WHERE ID_CLIENTE = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, id_cliente);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id             = sqlite3_column_int(stmt, 0);
        const char *titolo = sqlite3_column_text(stmt, 1);
        int id_client      = sqlite3_column_int(stmt, 2);
        char riga[256];
        snprintf(riga, sizeof(riga), "%d|%s|%d\n", id, titolo, id_client);
        strncat(output, riga, output_size - strlen(output) - 1);
    }
    sqlite3_finalize(stmt);
}

void get_dettagli_scheda(sqlite3 *db, int id_scheda, char *output, int output_size) {
    output[0] = '\0';
    const char *sql = "SELECT ID_SCHEDA, ID_ESERCIZIO, SERIE, RIPETIZIONI, RECUPERO_SEC FROM DETTAGLI_SCHEDA WHERE ID_SCHEDA = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, id_scheda);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id_sched     = sqlite3_column_int(stmt, 0);
        int id_esercizio = sqlite3_column_int(stmt, 1);
        int serie        = sqlite3_column_int(stmt, 2);
        int ripetizioni  = sqlite3_column_int(stmt, 3);
        int recupero_sec = sqlite3_column_int(stmt, 4);
        char riga[256];
        snprintf(riga, sizeof(riga), "%d|%d|%d|%d|%d\n", id_sched, id_esercizio, serie, ripetizioni, recupero_sec);
        strncat(output, riga, output_size - strlen(output) - 1);
    }
    sqlite3_finalize(stmt);
}

void get_storico(sqlite3 *db, int id_cliente, char *output, int output_size) {
    output[0] = '\0';
    const char *sql = "SELECT ID, ID_CLIENTE, PESO_REGISTRATO, DATA FROM STORICO WHERE ID_CLIENTE = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, id_cliente);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id                 = sqlite3_column_int(stmt, 0);
        int id_client          = sqlite3_column_int(stmt, 1);
        double peso_registrato = sqlite3_column_double(stmt, 2);
        const char *data       = sqlite3_column_text(stmt, 3);
        char riga[256];
        snprintf(riga, sizeof(riga), "%d|%d|%f|%s\n", id, id_client, peso_registrato, data);
        strncat(output, riga, output_size - strlen(output) - 1);
    }
    sqlite3_finalize(stmt);
}

void get_piani_alimentari(sqlite3 *db, int id_cliente, char *output, int output_size) {
    output[0] = '\0';
    const char *sql = "SELECT ID, ID_CLIENTE, KCAL_TARGET, NOTE_DIETA FROM PIANI_ALIMENTARI WHERE ID_CLIENTE = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, id_cliente);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id                 = sqlite3_column_int(stmt, 0);
        int id_client          = sqlite3_column_int(stmt, 1);
        double kcal_target     = sqlite3_column_double(stmt, 2);
        const char *note_dieta = sqlite3_column_text(stmt, 3);
        char riga[256];
        snprintf(riga, sizeof(riga), "%d|%d|%f|%s\n", id, id_client, kcal_target, note_dieta);
        strncat(output, riga, output_size - strlen(output) - 1);
    }
    sqlite3_finalize(stmt);
}

void get_ruolo_utente(sqlite3 *db, int id_utente, char *output, int output_size) {
    output[0] = '\0';
    const char *sql = "SELECT RUOLO FROM UTENTI WHERE ID = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, id_utente);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *ruolo = sqlite3_column_text(stmt, 0);
        snprintf(output, output_size, "%s", ruolo);
    }
    sqlite3_finalize(stmt);
}