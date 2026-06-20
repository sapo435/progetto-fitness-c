/*
 * main_logic.c
 * ------------
 * Punto di ingresso dell'applicazione New Fitness (lato client).
 *
 * FLUSSO:
 *   1. L'utente sceglie se accedere o registrarsi
 *   2. Il login/registrazione passa dal server via socket
 *   3. In base al ruolo (cliente/trainer) si apre l'area giusta
 *   4. I calcoli (BMR, TDEE, macro) restano lato client (logica.c)
 *   5. I dati vengono salvati/letti dal server
 */

#include <stdio.h>
#include <string.h>
#include "modello.h"
#include "logica.h"
#include "interfaccia.h"
#include "server.h"
#include <time.h>

int main(void)
{
    Utente u;
    ProgrammaAllenamento programma;
    PianoNutrizionale    piano;
    float settimane;
    int   scelta, id_server;
    char  username[DIM_NOME], password[DIM_NOME];
    char  ruolo[DIM_NOME];

    /* Menu iniziale */
    linea('*', 62);
    printf("             NEW FITNESS\n");
    linea('*', 62);
    printf("\n  Cosa vuoi fare?\n");
    printf("    1. Accedere (login)\n");
    printf("    2. Creare un nuovo account\n");
    scelta = leggi_int("  Scelta");

    if (scelta == 1) {

        /* ---- LOGIN ---- */
        linea('=', 62);
        printf("  LOGIN\n");
        linea('=', 62);
        leggi_stringa("  Username", username, DIM_NOME);
        leggi_stringa("  Password", password, DIM_NOME);

        id_server = login_server(username, password, ruolo, sizeof(ruolo));
        if (id_server < 0) {
            printf("  Username o password errati.\n");
            return 1;
        }

        printf("  Accesso consentito! Benvenuto, %s (ruolo: %s)\n",
               username, ruolo);

        /* ---- SMISTAMENTO PER RUOLO ---- */
        if (strcmp(ruolo, "trainer") == 0) {
            area_trainer(id_server);
            return 0;
        }

        /* Se e' un cliente, mostra il menu cliente */
        printf("\n  Cosa vuoi fare?\n");
        printf("    1. Visualizza report e programmi\n");
        printf("    2. Sezione feedback settimanale\n");
        printf("    3. Visualizza storico pesi (dal server)\n");
        printf("    4. Visualizza le tue schede (dal server)\n");
        printf("    5. Visualizza i tuoi piani alimentari (dal server)\n");
        scelta = leggi_int("  Scelta");

        if (scelta == 2) {
            /* Per il feedback serve l'utente in memoria.
               Chiediamo i dati base per i calcoli locali. */
            printf("\n  Per il feedback, inserisci i tuoi dati attuali:\n");
            inserisci_dati(&u);
            strncpy(u.username, username, DIM_NOME - 1);
            u.bmi       = calcola_bmi(u.peso_kg, u.altezza_cm);
            u.obiettivo = determina_obiettivo(&u);
            calcola_metriche(&u);
            u.settimana_corrente = 1;
            u.num_pesi           = 0;
            registra_peso(&u, u.peso_kg);

            sezione_feedback(&u, id_server);
            return 0;

        } else if (scelta == 3) {
            mostra_storico_server(id_server);
            return 0;

        } else if (scelta == 4) {
            mostra_schede_server(id_server);
            return 0;

        } else if (scelta == 5) {
            mostra_piani_server(id_server);
            return 0;
        }

        /* scelta == 1: mostra report e programmi.
           Servono i dati per i calcoli locali. */
        printf("\n  Per il report, inserisci i tuoi dati attuali:\n");
        inserisci_dati(&u);
        strncpy(u.username, username, DIM_NOME - 1);
        u.bmi       = calcola_bmi(u.peso_kg, u.altezza_cm);
        u.obiettivo = determina_obiettivo(&u);
        calcola_metriche(&u);

    } else {

        /* ---- REGISTRAZIONE ---- */
        printf("\n");
        linea('*', 62);
        printf("  NUOVO ACCOUNT\n");
        linea('*', 62);

        leggi_stringa("  Scegli un username", username, DIM_NOME);
        leggi_stringa("  Scegli una password", password, DIM_NOME);

        /* Inserimento dati e calcoli (tutto lato client) */
        inserisci_dati(&u);
        strncpy(u.username, username, DIM_NOME - 1);
        u.bmi       = calcola_bmi(u.peso_kg, u.altezza_cm);
        u.obiettivo = determina_obiettivo(&u);
        calcola_metriche(&u);

        /* Invia al server */
        id_server = registra_utente_server(&u, username, password);
        if (id_server < 0) {
            printf("  Errore nella registrazione. Riprova.\n");
            return 1;
        }

        printf("\n  Account '%s' registrato con successo! (ID: %d)\n",
               username, id_server);

        /* Salva il peso iniziale nello storico del server */
        {
            char data_oggi_str[16];
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            strftime(data_oggi_str, sizeof(data_oggi_str), "%Y-%m-%d", tm);
            salva_storico_server(id_server, u.peso_kg, data_oggi_str);
        }
    }

    /* ---- STAMPA REPORT E PROGRAMMI (calcoli lato client) ---- */
    printf("\n");
    stampa_report(&u);

    /* Programma di allenamento */
    if (u.servizio == SERVIZIO_SOLO_ALLENAMENTO ||
        u.servizio == SERVIZIO_ENTRAMBI) {
        printf("\n");
        programma = costruisci_programma(&u);
        stampa_programma(&programma);
    }

    /* Piano nutrizionale */
    if (u.servizio == SERVIZIO_SOLO_NUTRIZIONE ||
        u.servizio == SERVIZIO_ENTRAMBI) {
        printf("\n");
        piano = costruisci_piano(&u);
        stampa_piano(&piano);

        /* Salva il piano sul server */
        salva_piano_server(id_server, u.kcal_obiettivo,
                           stringa_obiettivo(u.obiettivo));
    }

    /* Messaggio finale */
    printf("\n");
    linea('*', 62);
    settimane = stima_settimane(&u);
    if (settimane > 0.0f)
        printf("  Obiettivo in %.0f settimane. Buon allenamento, %s!\n",
               settimane, u.nome);
    else
        printf("  Sei gia' al peso obiettivo. Mantieni la rotta, %s!\n",
               u.nome);
    linea('*', 62);
    printf("\n");

    return 0;
}
