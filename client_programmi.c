#include <stdio.h>
#include <string.h>
#include <math.h>
#include "client_logica.h"

/*
 * client_programmi.c
 * Costruzione dei programmi di allenamento: funzioni helper
 * (aggiungi_esercizio, aggiungi_sessione) e le funzioni che
 * generano il programma standard e le 3 varianti per obiettivo.
 */

/* PROGRAMMA DI ALLENAMENTO */
void aggiungi_esercizio(Sessione *s, const char *nome,
                        const char *muscolo, int serie,
                        int rip, int riposo, const char *note)
{
    if (s->num_esercizi >= MAX_ESERCIZI) return;
    Esercizio *e = &s->esercizi[s->num_esercizi++];
    strncpy(e->nome_esercizio,   nome,    DIM_NOME  - 1);
    strncpy(e->gruppo_muscolare, muscolo, DIM_NOME  - 1);
    strncpy(e->note,             note,    DIM_TESTO - 1);
    e->serie       = serie;
    e->ripetizioni = rip;
    e->riposo_sec  = riposo;
}

Sessione *aggiungi_sessione(ProgrammaAllenamento *p,
                            const char *titolo, int giorno)
{
    if (p->num_sessioni >= MAX_SESSIONI) return NULL;
    Sessione *s = &p->sessioni[p->num_sessioni++];
    memset(s, 0, sizeof(Sessione));
    strncpy(s->titolo, titolo, DIM_NOME - 1);
    s->giorno_settimana = giorno;
    return s;
}

ProgrammaAllenamento costruisci_programma(const Utente *u)
{
    ProgrammaAllenamento p;
    Sessione *s;
    memset(&p, 0, sizeof(ProgrammaAllenamento));
    p.obiettivo        = u->obiettivo;
    p.livello          = determina_livello(u->livello_attivita);
    p.durata_settimane = 8;

    switch (u->obiettivo) {
        case OBIETTIVO_DIMAGRIMENTO:
            strncpy(p.nome, "Programma Bruciagrassi", DIM_NOME - 1);
            strncpy(p.descrizione, "Cardio + circuit training per massimizzare il dispendio calorico", DIM_TESTO - 1);
            s = aggiungi_sessione(&p, "Cardio + Core - Lunedi'", 0);
            aggiungi_esercizio(s, "Corsa/camminata veloce", "Cardio",       1, 30,  0, "30 min a ritmo sostenuto");
            aggiungi_esercizio(s, "Mountain Climber",       "Core",         3, 20, 45, "Velocita' controllata");
            aggiungi_esercizio(s, "Plank",                  "Core",         3, 45, 45, "45 secondi per serie");
            aggiungi_esercizio(s, "Burpees",                "Corpo intero", 3, 10, 60, "Massima esplosivita'");
            s = aggiungi_sessione(&p, "Circuit Training - Mercoledi'", 2);
            aggiungi_esercizio(s, "Squat a corpo libero", "Gambe",        4, 15, 45, "");
            aggiungi_esercizio(s, "Push-up",              "Petto",        4, 12, 45, "In ginocchio se necessario");
            aggiungi_esercizio(s, "Affondi alternati",    "Gambe/Glutei", 3, 12, 45, "Per lato");
            aggiungi_esercizio(s, "Jumping Jack",         "Cardio",       3, 30, 30, "");
            aggiungi_esercizio(s, "Russian Twist",        "Core",         3, 20, 45, "Con peso opzionale");
            s = aggiungi_sessione(&p, "HIIT + Stretching - Venerdi'", 4);
            aggiungi_esercizio(s, "Sprint 20s / riposo 40s", "Cardio",    8,  1, 40, "8 intervalli totali");
            aggiungi_esercizio(s, "Squat Jump",              "Gambe",     3, 10, 60, "Atterraggio morbido");
            aggiungi_esercizio(s, "Stretching globale",      "Mobilita'", 1, 10,  0, "10 min defaticamento");
            break;

        case OBIETTIVO_MASSA:
            strncpy(p.nome, "Programma Ipertrofia", DIM_NOME - 1);
            strncpy(p.descrizione, "Push/Pull/Legs per massimizzare la crescita muscolare", DIM_TESTO - 1);
            s = aggiungi_sessione(&p, "SPINTA - Petto Spalle Tricipiti (Lun)", 0);
            aggiungi_esercizio(s, "Panca piana bilanciere",  "Petto",      4,  8, 90, "Progressivo ogni settimana");
            aggiungi_esercizio(s, "Panca inclinata manubri", "Petto alto", 3, 10, 75, "");
            aggiungi_esercizio(s, "Military press",          "Spalle",     4,  8, 90, "");
            aggiungi_esercizio(s, "Alzate laterali",         "Spalle",     3, 12, 60, "");
            aggiungi_esercizio(s, "French press",            "Tricipiti",  3, 10, 60, "");
            s = aggiungi_sessione(&p, "TRAZIONE - Schiena Bicipiti (Mer)", 2);
            aggiungi_esercizio(s, "Trazioni alla sbarra", "Dorsali",  4,  6, 90, "Aggiungere peso se possibile");
            aggiungi_esercizio(s, "Rematore bilanciere",  "Schiena",  4,  8, 90, "");
            aggiungi_esercizio(s, "Lat machine",          "Dorsali",  3, 10, 75, "");
            aggiungi_esercizio(s, "Curl con bilanciere",  "Bicipiti", 3, 10, 60, "");
            aggiungi_esercizio(s, "Curl martello",        "Bicipiti", 3, 12, 60, "");
            s = aggiungi_sessione(&p, "GAMBE - Quadricipiti Femorali Glutei (Ven)", 4);
            aggiungi_esercizio(s, "Squat con bilanciere", "Quadricipiti", 4,  8, 120, "Scendere a 90 gradi");
            aggiungi_esercizio(s, "Leg press",            "Gambe",        3, 10,  90, "");
            aggiungi_esercizio(s, "Stacco rumeno",        "Femorali",     4,  8,  90, "");
            aggiungi_esercizio(s, "Affondi bulgari",      "Glutei",       3, 10,  75, "Per lato");
            aggiungi_esercizio(s, "Calf raise",           "Polpacci",     4, 15,  60, "");
            break;

        default:
            strncpy(p.nome, "Programma Benessere Totale", DIM_NOME - 1);
            strncpy(p.descrizione, "Allenamento equilibrato per mantenere forma e salute", DIM_TESTO - 1);
            s = aggiungi_sessione(&p, "Corpo Intero A - Lunedi'", 0);
            aggiungi_esercizio(s, "Squat",            "Gambe",   3, 12, 60, "");
            aggiungi_esercizio(s, "Push-up",          "Petto",   3, 12, 60, "");
            aggiungi_esercizio(s, "Rematore manubri", "Schiena", 3, 12, 60, "");
            aggiungi_esercizio(s, "Plank",            "Core",    3, 40, 45, "40 secondi");
            s = aggiungi_sessione(&p, "Cardio libero - Mercoledi'", 2);
            aggiungi_esercizio(s, "Camminata/Jogging", "Cardio",    1, 40, 0, "40 min a ritmo confortevole");
            aggiungi_esercizio(s, "Stretching",        "Mobilita'", 1, 10, 0, "");
            s = aggiungi_sessione(&p, "Corpo Intero B - Venerdi'", 4);
            aggiungi_esercizio(s, "Affondi",      "Gambe",     3, 10, 60, "Per lato");
            aggiungi_esercizio(s, "Dip su sedia", "Tricipiti", 3, 12, 60, "");
            aggiungi_esercizio(s, "Superman",     "Lombari",   3, 15, 45, "");
            aggiungi_esercizio(s, "Bicicletta",   "Core",      3, 20, 45, "");
            break;
    }
    return p;
}

/* PROGRAMMA CON VARIANTE (0=base, 1=intensivo, 2=avanzato) */
ProgrammaAllenamento costruisci_programma_ex(const Utente *u, int variante)
{
    ProgrammaAllenamento p;
    Sessione *s;
    memset(&p, 0, sizeof(p));
    p.obiettivo        = u->obiettivo;
    p.livello          = determina_livello(u->livello_attivita);
    p.durata_settimane = 8;

/* DIMAGRIMENTO */
    if (u->obiettivo == OBIETTIVO_DIMAGRIMENTO) {

        if (variante == 1) {
            /* V1: HIIT Estremo */
            strncpy(p.nome, "HIIT Estremo", DIM_NOME-1);
            strncpy(p.descrizione,
                "Intervalli ad alta intensita' per bruciare il massimo delle calorie",
                DIM_TESTO-1);

            s = aggiungi_sessione(&p, "HIIT + Addominali - Lunedi'", 0);
            aggiungi_esercizio(s, "Sprint 20s / walk 40s", "Cardio",    10, 1, 40,
                               "10 intervalli totali");
            aggiungi_esercizio(s, "Crunch",               "Addominali", 4, 20, 30, "");
            aggiungi_esercizio(s, "Leg Raise",            "Addominali", 3, 15, 30, "");
            aggiungi_esercizio(s, "Burpees",              "Corpo int.", 3,  8, 60,
                               "Massima esplosivita'");

            s = aggiungi_sessione(&p, "Tabata + Core - Mercoledi'", 2);
            aggiungi_esercizio(s, "Tabata Squat Jump",   "Gambe",      8,  1, 10,
                               "20s lavoro 10s riposo");
            aggiungi_esercizio(s, "Tabata Push-up",      "Petto",      8,  1, 10,
                               "20s lavoro 10s riposo");
            aggiungi_esercizio(s, "Plank laterale",      "Core",       3, 30,  0,
                               "30s per lato");
            aggiungi_esercizio(s, "Mountain Climber",    "Core",       3, 20, 30, "");

            s = aggiungi_sessione(&p, "Circuito Total Body - Venerdi'", 4);
            aggiungi_esercizio(s, "Salto con la corda", "Cardio",     1, 180, 0,
                               "3 min continui");
            aggiungi_esercizio(s, "Affondi jump",       "Gambe",      4,  10, 45,
                               "Per lato");
            aggiungi_esercizio(s, "Dip su sedia",       "Tricipiti",  3,  12, 45, "");
            aggiungi_esercizio(s, "Superman",           "Lombari",    3,  15, 30, "");
            aggiungi_esercizio(s, "Stretching",         "Mobilita'",  1,  10,  0, "");

        } else if (variante == 2) {
            /* V2: Endurance + Resistance */
            strncpy(p.nome, "Endurance + Resistance", DIM_NOME-1);
            strncpy(p.descrizione,
                "Combinazione di resistenza muscolare e cardio steadystate",
                DIM_TESTO-1);

            s = aggiungi_sessione(&p, "Resistance Full Body - Lunedi'", 0);
            aggiungi_esercizio(s, "Corsa 20 min",          "Cardio",   1,  20,  0,
                               "Ritmo moderato");
            aggiungi_esercizio(s, "Squat Goblet",          "Gambe",    4,  15, 45, "");
            aggiungi_esercizio(s, "Push-up inclinato",     "Petto",    4,  12, 45, "");
            aggiungi_esercizio(s, "Rematore su sedia",     "Schiena",  3,  15, 45, "");

            s = aggiungi_sessione(&p, "Cardio Steadystate - Mercoledi'", 2);
            aggiungi_esercizio(s, "Camminata veloce",      "Cardio",   1,  45,  0,
                               "45 min a passo sostenuto");
            aggiungi_esercizio(s, "Stretching Globale",   "Mobilita'", 1,  10,  0, "");

            s = aggiungi_sessione(&p, "Lower + HIIT - Venerdi'", 4);
            aggiungi_esercizio(s, "Stacco Rumeno",        "Femorali",  4,  12, 60,
                               "Peso moderato");
            aggiungi_esercizio(s, "Affondi laterali",     "Glutei",    3,  10, 45,
                               "Per lato");
            aggiungi_esercizio(s, "Calf Raise",           "Polpacci",  3,  20, 30, "");
            aggiungi_esercizio(s, "Bicicletta",           "Cardio",    1,  15,  0,
                               "15 min a ritmo medio");

        } else {
            /* V0: default Bruciagrassi */
            return costruisci_programma(u);
        }

/* MASSA */
    } else if (u->obiettivo == OBIETTIVO_MASSA) {

        if (variante == 1) {
            /* V1: Full Body 4x/settimana */
            strncpy(p.nome, "Full Body 4x", DIM_NOME-1);
            strncpy(p.descrizione,
                "Corpo intero 4 volte a settimana con progressione lineare",
                DIM_TESTO-1);

            s = aggiungi_sessione(&p, "Full Body A - Lunedi'", 0);
            aggiungi_esercizio(s, "Squat",              "Quadric.",  5,  5, 120,
                               "Progressione lineare ogni allenamento");
            aggiungi_esercizio(s, "Panca Piana",        "Petto",     5,  5, 120, "");
            aggiungi_esercizio(s, "Rematore bilanciere","Schiena",   5,  5, 120, "");
            aggiungi_esercizio(s, "Press Militare",     "Spalle",    3,  8,  90, "");

            s = aggiungi_sessione(&p, "Full Body B - Martedi'", 1);
            aggiungi_esercizio(s, "Stacco da terra",   "Schiena",   1,  5, 120,
                               "Un singolo set pesante");
            aggiungi_esercizio(s, "Panca Inclinata",   "Petto alto",3,  8,  90, "");
            aggiungi_esercizio(s, "Trazioni",          "Dorsali",   3,  6, 120,
                               "Con zavorra se possibile");
            aggiungi_esercizio(s, "Curl bilanciere",   "Bicipiti",  3, 10,  60, "");

            s = aggiungi_sessione(&p, "Full Body A - Giovedi'", 3);
            aggiungi_esercizio(s, "Squat",              "Quadric.",  5,  5, 120,
                               "+2.5kg rispetto lunedi'");
            aggiungi_esercizio(s, "Panca Piana",        "Petto",     5,  5, 120, "");
            aggiungi_esercizio(s, "Rematore bilanciere","Schiena",   5,  5, 120, "");
            aggiungi_esercizio(s, "French Press",       "Tricipiti", 3, 10,  60, "");

            s = aggiungi_sessione(&p, "Full Body B - Venerdi'", 4);
            aggiungi_esercizio(s, "Stacco da terra",   "Schiena",   1,  5, 120, "");
            aggiungi_esercizio(s, "Panca Inclinata",   "Petto alto",3,  8,  90, "");
            aggiungi_esercizio(s, "Lat Machine",       "Dorsali",   3, 10,  90, "");
            aggiungi_esercizio(s, "Calf Raise",        "Polpacci",  4, 15,  60, "");

        } else if (variante == 2) {
            /* V2: Arnold Split (Chest/Back + Shoulder/Arms + Legs) */
            strncpy(p.nome, "Arnold Split", DIM_NOME-1);
            strncpy(p.descrizione,
                "Divisione 3 giorni ispirata all'Arnold Split classico",
                DIM_TESTO-1);

            s = aggiungi_sessione(&p, "Petto + Schiena - Lunedi'/Giovedi'", 0);
            aggiungi_esercizio(s, "Panca piana bilanciere","Petto",    4,  8, 90, "");
            aggiungi_esercizio(s, "Panca inclinata",       "Petto",    3, 10, 75, "");
            aggiungi_esercizio(s, "Rematore bilanciere",   "Schiena",  4,  8, 90, "");
            aggiungi_esercizio(s, "Lat Machine",           "Dorsali",  3, 10, 75, "");
            aggiungi_esercizio(s, "Trazioni",              "Dorsali",  3,  6, 90, "");

            s = aggiungi_sessione(&p, "Spalle + Braccia - Martedi'/Venerdi'", 1);
            aggiungi_esercizio(s, "Military Press",      "Spalle",    4,  8, 90, "");
            aggiungi_esercizio(s, "Alzate Laterali",     "Spalle",    4, 12, 60, "");
            aggiungi_esercizio(s, "Curl bilanciere",     "Bicipiti",  3, 10, 60, "");
            aggiungi_esercizio(s, "Curl martello",       "Bicipiti",  3, 12, 60, "");
            aggiungi_esercizio(s, "Pushdown cavo",       "Tricipiti", 3, 12, 60, "");
            aggiungi_esercizio(s, "French Press",        "Tricipiti", 3, 10, 60, "");

            s = aggiungi_sessione(&p, "Gambe - Mercoledi'/Sabato", 2);
            aggiungi_esercizio(s, "Squat bilanciere",   "Quadric.",  4,  8, 120, "");
            aggiungi_esercizio(s, "Leg Press",          "Gambe",     3, 10,  90, "");
            aggiungi_esercizio(s, "Stacco rumeno",      "Femorali",  4,  8,  90, "");
            aggiungi_esercizio(s, "Leg Curl",           "Femorali",  3, 12,  60, "");
            aggiungi_esercizio(s, "Calf Raise",         "Polpacci",  5, 15,  45, "");

        } else {
            return costruisci_programma(u);
        }

/* MANTENIMENTO / BENESSERE */
    } else {

        if (variante == 1) {
            /* V1: Functional Fitness */
            strncpy(p.nome, "Fitness Funzionale", DIM_NOME-1);
            strncpy(p.descrizione,
                "Movimenti funzionali per forza, mobilita' e coordinazione",
                DIM_TESTO-1);

            s = aggiungi_sessione(&p, "Mobilita' + Forza - Lunedi'", 0);
            aggiungi_esercizio(s, "Riscaldamento dinamico",  "Mobilita'",  1, 10,  0,
                               "10 min stretching attivo");
            aggiungi_esercizio(s, "Turkish Get-Up",          "Full body",  3,  3, 90,
                               "Per lato con manubrio");
            aggiungi_esercizio(s, "Squat Goblet",            "Gambe",      4, 12, 60, "");
            aggiungi_esercizio(s, "Push-up varianti",        "Petto",      3, 10, 45,
                               "Classico + inclinato + declinato");

            s = aggiungi_sessione(&p, "Cardio Funzionale - Mercoledi'", 2);
            aggiungi_esercizio(s, "Camminata collinare",    "Cardio",      1, 45,  0,
                               "O tapis roulant inclinato");
            aggiungi_esercizio(s, "Kettlebell Swing",       "Femorali",    3, 15, 45, "");
            aggiungi_esercizio(s, "Bear Crawl",             "Full body",   3,  5,  0,
                               "10m andata e ritorno");

            s = aggiungi_sessione(&p, "Equilibrio + Core - Venerdi'", 4);
            aggiungi_esercizio(s, "Stacco monopodalico",   "Femorali",    3, 10, 60,
                               "Per lato, no bilanciere");
            aggiungi_esercizio(s, "Plank varianti",        "Core",        4, 40, 30,
                               "Frontale + laterale sx/dx");
            aggiungi_esercizio(s, "Dead Bug",              "Core",        3, 10, 30, "");
            aggiungi_esercizio(s, "Stretching globale",    "Mobilita'",   1, 15,  0, "");

        } else if (variante == 2) {
            /* V2: Tonificazione Circuitale */
            strncpy(p.nome, "Tonificazione Circuitale", DIM_NOME-1);
            strncpy(p.descrizione,
                "Circuiti di tonificazione muscolare con poco riposo",
                DIM_TESTO-1);

            s = aggiungi_sessione(&p, "Circuito Upper - Lunedi'", 0);
            aggiungi_esercizio(s, "Push-up",           "Petto",      3, 15, 20,
                               "Eseguire in circuito senza pause");
            aggiungi_esercizio(s, "Rematore manubri",  "Schiena",    3, 15, 20, "");
            aggiungi_esercizio(s, "Alzate laterali",   "Spalle",     3, 15, 20, "");
            aggiungi_esercizio(s, "Curl manubri",      "Bicipiti",   3, 15, 20, "");
            aggiungi_esercizio(s, "Kick-back",         "Tricipiti",  3, 15, 60,
                               "Riposo 60s tra circuiti");

            s = aggiungi_sessione(&p, "Circuito Lower - Mercoledi'", 2);
            aggiungi_esercizio(s, "Squat sumo",        "Glutei",     3, 20, 20, "");
            aggiungi_esercizio(s, "Affondi alternati", "Gambe",      3, 16, 20,
                               "8 per lato");
            aggiungi_esercizio(s, "Hip Thrust",        "Glutei",     3, 15, 20, "");
            aggiungi_esercizio(s, "Calf Raise",        "Polpacci",   3, 20, 20, "");
            aggiungi_esercizio(s, "Leg Raise",         "Addominali", 3, 12, 60, "");

            s = aggiungi_sessione(&p, "Circuito Full + Cardio - Venerdi'", 4);
            aggiungi_esercizio(s, "Burpees",           "Full body",  3,  8, 30, "");
            aggiungi_esercizio(s, "Mountain Climber",  "Core",       3, 20, 30, "");
            aggiungi_esercizio(s, "Squat Jump",        "Gambe",      3, 10, 30, "");
            aggiungi_esercizio(s, "Stretching",        "Mobilita'",  1, 10,  0, "");

        } else {
            return costruisci_programma(u);
        }
    }

    return p;
}