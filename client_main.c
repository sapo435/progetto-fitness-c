/*
 * client_main.c
 * -------------
 * Punto di ingresso dell'applicazione New Fitness (lato client).
 * Dopo il login, salva il token di sessione e lo include
 * in ogni comunicazione col server. Effettua il logout all'uscita.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "client_modello.h"
#include "client_logica.h"
#include "client_ui.h"
#include "client_api.h"

int main(void)
{
    Utente u;
    ProgrammaAllenamento programma;
    PianoNutrizionale    piano;
    float settimane;
    int   scelta, id_server;
    char  username[DIM_NOME], password[DIM_NOME];
    char  ruolo[DIM_NOME];
    char  token[DIM_TOKEN];  /* Token di sessione */

    token[0] = '\0';

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

        id_server = login_server(username, password,
                                 ruolo, sizeof(ruolo),
                                 token, sizeof(token));
        if (id_server < 0) {
            printf("  Username o password errati.\n");
            return 1;
        }

        printf("  Accesso consentito! Benvenuto, %s (ruolo: %s)\n",
               username, ruolo);

        /* ---- SMISTAMENTO PER RUOLO ---- */
        if (strcmp(ruolo, "trainer") == 0) {
            area_trainer(id_server, token);
            logout_server(token);
            printf("  Logout effettuato.\n");
            return 0;
        }

        /* Menu cliente */
        printf("\n  Cosa vuoi fare?\n");
        printf("    1. Visualizza report e programmi\n");
        printf("    2. Sezione feedback settimanale\n");
        printf("    3. Visualizza storico pesi\n");
        printf("    4. Visualizza le tue schede\n");
        printf("    5. Visualizza i tuoi piani alimentari\n");
        printf("    0. Logout\n");
        scelta = leggi_int("  Scelta");

        if (scelta == 0) {
            logout_server(token);
            printf("  Logout effettuato. A presto!\n");
            return 0;
        }

        if (scelta == 2) {
            printf("\n  Per il feedback, inserisci i tuoi dati attuali:\n");
            inserisci_dati(&u);
            strncpy(u.username, username, DIM_NOME - 1);
            u.bmi       = calcola_bmi(u.peso_kg, u.altezza_cm);
            u.obiettivo = determina_obiettivo(&u);
            calcola_metriche(&u);
            u.settimana_corrente = 1;
            u.num_pesi           = 0;
            registra_peso(&u, u.peso_kg);

            sezione_feedback(&u, id_server, token);
            logout_server(token);
            printf("  Logout effettuato.\n");
            return 0;

        } else if (scelta == 3) {
            mostra_storico_server(token, id_server);
            logout_server(token);
            printf("  Logout effettuato.\n");
            return 0;

        } else if (scelta == 4) {
            mostra_schede_server(token, id_server);
            logout_server(token);
            printf("  Logout effettuato.\n");
            return 0;

        } else if (scelta == 5) {
            mostra_piani_server(token, id_server);
            logout_server(token);
            printf("  Logout effettuato.\n");
            return 0;
        }

        /* scelta == 1: report e programmi */
        printf("\n  Per il report, inserisci i tuoi dati attuali:\n");
        inserisci_dati(&u);
        strncpy(u.username, username, DIM_NOME - 1);
        u.bmi       = calcola_bmi(u.peso_kg, u.altezza_cm);
        u.obiettivo = determina_obiettivo(&u);
        calcola_metriche(&u);

    } else {

        /* ---- REGISTRAZIONE (senza token) ---- */
        printf("\n");
        linea('*', 62);
        printf("  NUOVO ACCOUNT\n");
        linea('*', 62);

        leggi_stringa("  Scegli un username", username, DIM_NOME);
        leggi_stringa("  Scegli una password", password, DIM_NOME);

        inserisci_dati(&u);
        strncpy(u.username, username, DIM_NOME - 1);
        u.bmi       = calcola_bmi(u.peso_kg, u.altezza_cm);
        u.obiettivo = determina_obiettivo(&u);
        calcola_metriche(&u);

        id_server = registra_utente_server(&u, username, password);
        if (id_server < 0) {
            printf("  Errore nella registrazione. Riprova.\n");
            return 1;
        }

        printf("\n  Account '%s' registrato! (ID: %d)\n", username, id_server);

        /* Login automatico dopo registrazione per ottenere il token */
        id_server = login_server(username, password,
                                 ruolo, sizeof(ruolo),
                                 token, sizeof(token));
        if (id_server < 0) {
            printf("  Registrazione ok ma login fallito. Riprova ad accedere.\n");
            return 1;
        }

        /* Salva peso iniziale (con token) */
        {
            char data_oggi[16];
            time_t t = time(NULL);
            strftime(data_oggi, sizeof(data_oggi), "%Y-%m-%d", localtime(&t));
            salva_storico_server(token, id_server, u.peso_kg, data_oggi);
        }
    }

    /* ---- REPORT E PROGRAMMI ---- */
    printf("\n");
    stampa_report(&u);

    if (u.servizio == SERVIZIO_SOLO_ALLENAMENTO ||
        u.servizio == SERVIZIO_ENTRAMBI) {
        printf("\n");
        programma = costruisci_programma(&u);
        stampa_programma(&programma);
    }

    if (u.servizio == SERVIZIO_SOLO_NUTRIZIONE ||
        u.servizio == SERVIZIO_ENTRAMBI) {
        printf("\n");
        piano = costruisci_piano(&u);
        stampa_piano(&piano);
        salva_piano_server(token, id_server, u.kcal_obiettivo,
                           stringa_obiettivo(u.obiettivo));
    }

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

    /* Logout prima di uscire */
    if (token[0] != '\0') {
        logout_server(token);
        printf("  Logout effettuato.\n");
    }

    return 0;
}
