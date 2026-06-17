#include <stdio.h>
#include "db_manager.h"
#include "network_manager.h" // 1. AGGIUNGI QUESTO INCUDE!

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

    // 2. AVVIA IL SERVER (Sostituisce il vecchio commento)
    // Scegliamo la porta 8080 come standard
    avvia_server(db, 8080);

    // Questa riga verrà eseguita SOLO se il server si spegne3
    chiudi_database(db);
    printf("Sistema spento correttamente.\n");
    return 0;
}