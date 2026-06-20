#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "db_manager.h"
#include "network_manager.h"

int main() {
    // Inizializzazione del Database
    sqlite3 *db = inizializza_database("palestra.db");
    if (!db) {
        fprintf(stderr, "Errore fatale: impossibile aprire il database.\n");
        return 1;
    }

    // Creazione delle tabelle (se non esistono)
    crea_tabelle(db);
    printf("Database pronto e tabelle verificate.\n");

    //  AVVIO DEL SERVER
    // Scegliamo la porta 8080 come standard
    srand(time(NULL));
    avvia_server(db, 8080);

    chiudi_database(db);
    printf("Sistema spento correttamente.\n");
    return 0;
}