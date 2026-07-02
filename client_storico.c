#include <stdio.h>
#include <string.h>
#include <math.h>
#include "client_logica.h"

/*
 * client_storico.c
 * Gestione storico pesi.
 */

/* STORICO PESI */
void registra_peso(Utente *u, float nuovo_peso)
{
    if (u->num_pesi >= MAX_STORICO) {
        for (int i = 0; i < MAX_STORICO - 1; i++)
            u->storico_peso[i] = u->storico_peso[i + 1];
        u->num_pesi = MAX_STORICO - 1;
    }
    RegistrazionePeso *r = &u->storico_peso[u->num_pesi++];
    r->peso_kg   = nuovo_peso;
    r->settimana = u->settimana_corrente;
    u->peso_kg   = nuovo_peso;
    u->bmi       = calcola_bmi(u->peso_kg, u->altezza_cm);
    u->obiettivo = determina_obiettivo(u);
    calcola_metriche(u);
}

float variazione_media_peso(const Utente *u)
{
    int n = u->num_pesi;
    if (n < 2) return 0.0f;
    int confronto = (n >= 3) ? n - 3 : 0;
    return (u->storico_peso[n-1].peso_kg - u->storico_peso[confronto].peso_kg)
           / (float)(u->storico_peso[n-1].settimana - u->storico_peso[confronto].settimana + 1);
}