/*
 * logica.c
 * --------
 * Implementazione di tutta la logica pura: metriche fisiologiche,
 * programmi di allenamento, piani nutrizionali e storico pesi.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "logica.h"


/* ==============================================================
 *  METRICHE FISIOLOGICHE
 * ============================================================== */

float calcola_bmi(float peso_kg, float altezza_cm)
{
    float altezza_m = altezza_cm / 100.0f;
    return peso_kg / (altezza_m * altezza_m);
}

const char *categoria_bmi(float bmi)
{
    if (bmi < 18.5f) return "Sottopeso";
    if (bmi < 25.0f) return "Normopeso";
    if (bmi < 30.0f) return "Sovrappeso";
    return "Obesita'";
}

float calcola_bmr(float peso_kg, float altezza_cm, int eta, char sesso)
{
    float bmr = 10.0f * peso_kg + 6.25f * altezza_cm - 5.0f * (float)eta;
    return (sesso == 'M') ? bmr + 5.0f : bmr - 161.0f;
}

float calcola_tdee(float bmr, float livello_attivita)
{
    return bmr * livello_attivita;
}

float calcola_kcal_obiettivo(float tdee, Obiettivo ob)
{
    switch (ob) {
        case OBIETTIVO_DIMAGRIMENTO: return tdee - 400.0f;
        case OBIETTIVO_MASSA:        return tdee + 250.0f;
        case OBIETTIVO_MANTENIMENTO: return tdee;
        case OBIETTIVO_BENESSERE:    return tdee - 100.0f;
        default:                     return tdee;
    }
}

void calcola_macronutrienti(float peso_kg, float kcal_ob, Obiettivo ob,
                            float *proteine, float *grassi, float *carboidrati)
{
    float fattore, kcal_usate;

    fattore      = (ob == OBIETTIVO_MASSA) ? 1.8f : 1.6f;
    *proteine    = peso_kg * fattore;
    *grassi      = (kcal_ob * 0.27f) / 9.0f;
    kcal_usate   = (*proteine) * 4.0f + (*grassi) * 9.0f;
    *carboidrati = (kcal_ob - kcal_usate) / 4.0f;
    if (*carboidrati < 0) *carboidrati = 0;
}

void calcola_metriche(Utente *u)
{
    u->bmr            = calcola_bmr(u->peso_kg, u->altezza_cm, u->eta, u->sesso);
    u->tdee_kcal      = calcola_tdee(u->bmr, u->livello_attivita);
    u->kcal_obiettivo = calcola_kcal_obiettivo(u->tdee_kcal, u->obiettivo);
    calcola_macronutrienti(u->peso_kg, u->kcal_obiettivo, u->obiettivo,
                           &u->proteine_g, &u->grassi_g, &u->carboidrati_g);
}

float stima_settimane(const Utente *u)
{
    float differenza, delta_giorno;

    differenza   = u->peso_kg - u->peso_obiettivo_kg;
    delta_giorno = u->tdee_kcal - u->kcal_obiettivo;
    if (fabsf(differenza) < 0.1f || fabsf(delta_giorno) < 1.0f)
        return 0.0f;
    return (fabsf(differenza) * 7700.0f) / fabsf(delta_giorno) / 7.0f;
}

float fabbisogno_acqua(float peso_kg, float ore_allenamento)
{
    return peso_kg * 35.0f + ore_allenamento * 500.0f;
}

Obiettivo determina_obiettivo(const Utente *u)
{
    float diff = u->peso_obiettivo_kg - u->peso_kg;

    if (u->bmi > 27.0f || diff < -3.0f)
        return OBIETTIVO_DIMAGRIMENTO;
    if (u->bmi < 20.0f || diff > 3.0f)
        return OBIETTIVO_MASSA;
    if (fabsf(diff) <= 1.0f)
        return OBIETTIVO_MANTENIMENTO;
    return OBIETTIVO_BENESSERE;
}

Livello determina_livello(float livello_attivita)
{
    if (livello_attivita <= 1.375f) return LIVELLO_PRINCIPIANTE;
    if (livello_attivita <= 1.550f) return LIVELLO_INTERMEDIO;
    return LIVELLO_AVANZATO;
}


/* ==============================================================
 *  PROGRAMMA DI ALLENAMENTO
 * ============================================================== */

void aggiungi_esercizio(Sessione *s, const char *nome,
                        const char *muscolo, int serie,
                        int rip, int riposo, const char *note)
{
    Esercizio *e;

    if (s->num_esercizi >= MAX_ESERCIZI) return;
    e = &s->esercizi[s->num_esercizi++];
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
    Sessione *s;

    if (p->num_sessioni >= MAX_SESSIONI) return NULL;
    s = &p->sessioni[p->num_sessioni++];
    memset(s, 0, sizeof(Sessione));
    strncpy(s->titolo, titolo, DIM_NOME - 1);
    s->giorno_settimana = giorno;
    return s;
}

ProgrammaAllenamento costruisci_programma(const Utente *u)
{
    ProgrammaAllenamento p;
    Sessione *s;
    Livello liv;

    memset(&p, 0, sizeof(ProgrammaAllenamento));
    liv          = determina_livello(u->livello_attivita);
    p.obiettivo  = u->obiettivo;
    p.livello    = liv;
    p.durata_settimane = 8;

    switch (u->obiettivo) {

        case OBIETTIVO_DIMAGRIMENTO:
            strncpy(p.nome, "Programma Bruciagrassi", DIM_NOME - 1);
            strncpy(p.descrizione,
                "Cardio + circuit training per massimizzare il dispendio calorico",
                DIM_TESTO - 1);

            s = aggiungi_sessione(&p, "Cardio + Core — Lunedi'", 0);
            aggiungi_esercizio(s, "Corsa/camminata veloce", "Cardio",       1, 30,  0, "30 min a ritmo sostenuto");
            aggiungi_esercizio(s, "Mountain Climber",       "Core",         3, 20, 45, "Velocita' controllata");
            aggiungi_esercizio(s, "Plank",                  "Core",         3, 45, 45, "45 secondi per serie");
            aggiungi_esercizio(s, "Burpees",                "Corpo intero", 3, 10, 60, "Massima esplosivita'");

            s = aggiungi_sessione(&p, "Circuit Training — Mercoledi'", 2);
            aggiungi_esercizio(s, "Squat a corpo libero", "Gambe",        4, 15, 45, "");
            aggiungi_esercizio(s, "Push-up",              "Petto",        4, 12, 45, "In ginocchio se necessario");
            aggiungi_esercizio(s, "Affondi alternati",    "Gambe/Glutei", 3, 12, 45, "Per lato");
            aggiungi_esercizio(s, "Jumping Jack",         "Cardio",       3, 30, 30, "");
            aggiungi_esercizio(s, "Russian Twist",        "Core",         3, 20, 45, "Con peso opzionale");

            s = aggiungi_sessione(&p, "HIIT + Stretching — Venerdi'", 4);
            aggiungi_esercizio(s, "Sprint 20s / riposo 40s", "Cardio",    8,  1, 40, "8 intervalli totali");
            aggiungi_esercizio(s, "Squat Jump",              "Gambe",     3, 10, 60, "Atterraggio morbido");
            aggiungi_esercizio(s, "Stretching globale",      "Mobilita'", 1, 10,  0, "10 min defaticamento");
            break;

        case OBIETTIVO_MASSA:
            strncpy(p.nome, "Programma Ipertrofia", DIM_NOME - 1);
            strncpy(p.descrizione,
                "Push/Pull/Legs per massimizzare la crescita muscolare",
                DIM_TESTO - 1);

            s = aggiungi_sessione(&p, "SPINTA — Petto, Spalle, Tricipiti (Lun)", 0);
            aggiungi_esercizio(s, "Panca piana bilanciere",  "Petto",      4,  8, 90, "Progressivo ogni settimana");
            aggiungi_esercizio(s, "Panca inclinata manubri", "Petto alto", 3, 10, 75, "");
            aggiungi_esercizio(s, "Military press",          "Spalle",     4,  8, 90, "");
            aggiungi_esercizio(s, "Alzate laterali",         "Spalle",     3, 12, 60, "");
            aggiungi_esercizio(s, "French press",            "Tricipiti",  3, 10, 60, "");
            aggiungi_esercizio(s, "Pushdown al cavo",        "Tricipiti",  3, 12, 60, "");

            s = aggiungi_sessione(&p, "TRAZIONE — Schiena, Bicipiti (Mer)", 2);
            aggiungi_esercizio(s, "Trazioni alla sbarra", "Dorsali",  4,  6, 90, "Aggiungere peso se possibile");
            aggiungi_esercizio(s, "Rematore bilanciere",  "Schiena",  4,  8, 90, "");
            aggiungi_esercizio(s, "Lat machine",          "Dorsali",  3, 10, 75, "");
            aggiungi_esercizio(s, "Curl con bilanciere",  "Bicipiti", 3, 10, 60, "");
            aggiungi_esercizio(s, "Curl martello",        "Bicipiti", 3, 12, 60, "");

            s = aggiungi_sessione(&p, "GAMBE — Quadricipiti, Femorali, Glutei (Ven)", 4);
            aggiungi_esercizio(s, "Squat con bilanciere", "Quadricipiti", 4,  8, 120, "Scendere a 90 gradi");
            aggiungi_esercizio(s, "Leg press",            "Gambe",        3, 10,  90, "");
            aggiungi_esercizio(s, "Stacco rumeno",        "Femorali",     4,  8,  90, "");
            aggiungi_esercizio(s, "Affondi bulgari",      "Glutei",       3, 10,  75, "Per lato");
            aggiungi_esercizio(s, "Calf raise",           "Polpacci",     4, 15,  60, "");
            break;

        default:
            strncpy(p.nome, "Programma Benessere Totale", DIM_NOME - 1);
            strncpy(p.descrizione,
                "Allenamento equilibrato per mantenere forma e salute",
                DIM_TESTO - 1);

            s = aggiungi_sessione(&p, "Corpo Intero A — Lunedi'", 0);
            aggiungi_esercizio(s, "Squat",            "Gambe",   3, 12, 60, "");
            aggiungi_esercizio(s, "Push-up",          "Petto",   3, 12, 60, "");
            aggiungi_esercizio(s, "Rematore manubri", "Schiena", 3, 12, 60, "");
            aggiungi_esercizio(s, "Plank",            "Core",    3, 40, 45, "40 secondi");

            s = aggiungi_sessione(&p, "Cardio libero — Mercoledi'", 2);
            aggiungi_esercizio(s, "Camminata/Jogging", "Cardio",    1, 40, 0, "40 min a ritmo confortevole");
            aggiungi_esercizio(s, "Stretching",        "Mobilita'", 1, 10, 0, "");

            s = aggiungi_sessione(&p, "Corpo Intero B — Venerdi'", 4);
            aggiungi_esercizio(s, "Affondi",      "Gambe",     3, 10, 60, "Per lato");
            aggiungi_esercizio(s, "Dip su sedia", "Tricipiti", 3, 12, 60, "");
            aggiungi_esercizio(s, "Superman",     "Lombari",   3, 15, 45, "");
            aggiungi_esercizio(s, "Bicicletta",   "Core",      3, 20, 45, "");
            break;
    }
    return p;
}


/* ==============================================================
 *  PIANO NUTRIZIONALE
 * ============================================================== */

void aggiungi_pasto(PianoNutrizionale *piano, const char *nome,
                    const char *alimenti, float kcal,
                    float prot, float carb, float gras,
                    const char *note)
{
    Pasto *m;

    if (piano->num_pasti >= MAX_PASTI) return;
    m = &piano->pasti[piano->num_pasti++];
    strncpy(m->nome,      nome,      DIM_NOME  - 1);
    strncpy(m->alimenti,  alimenti,  DIM_TESTO - 1);
    strncpy(m->note,      note,      DIM_TESTO - 1);
    m->kcal          = kcal;
    m->proteine_g    = prot;
    m->carboidrati_g = carb;
    m->grassi_g      = gras;
}

PianoNutrizionale costruisci_piano(const Utente *u)
{
    PianoNutrizionale piano;
    float k, pr, ca, fa;
    float col, sp1, pran, sp2, cen;

    memset(&piano, 0, sizeof(PianoNutrizionale));
    k    = u->kcal_obiettivo;
    pr   = u->proteine_g;
    ca   = u->carboidrati_g;
    fa   = u->grassi_g;
    col  = 0.25f;
    sp1  = 0.10f;
    pran = 0.35f;
    sp2  = 0.10f;
    cen  = 0.20f;

    switch (u->obiettivo) {

        case OBIETTIVO_DIMAGRIMENTO:
            strncpy(piano.etichetta, "Piano Dimagrimento", DIM_NOME - 1);
            aggiungi_pasto(&piano, "Colazione",
                "Yogurt greco 0% (200g), avena (50g), frutti rossi (100g), caffe'",
                k*col, pr*col, ca*col, fa*col,
                "Ricco di proteine per aumentare la sazieta'");
            aggiungi_pasto(&piano, "Spuntino mattina",
                "1 mela media, 10 mandorle",
                k*sp1, pr*sp1, ca*sp1, fa*sp1,
                "Evita i picchi glicemici");
            aggiungi_pasto(&piano, "Pranzo",
                "Petto di pollo (150g), insalata mista (200g), pane integrale (50g), olio EVO",
                k*pran, pr*pran, ca*pran, fa*pran,
                "Priorita' a verdure e proteine magre");
            aggiungi_pasto(&piano, "Spuntino pomeriggio",
                "Ricotta light (100g), 1 frutto di stagione",
                k*sp2, pr*sp2, ca*sp2, fa*sp2, "");
            aggiungi_pasto(&piano, "Cena",
                "Salmone (120g), verdure grigliate (300g), olio EVO",
                k*cen, pr*cen, ca*cen, fa*cen,
                "Pochi carboidrati la sera");
            break;

        case OBIETTIVO_MASSA:
            strncpy(piano.etichetta, "Piano Massa Muscolare", DIM_NOME - 1);
            aggiungi_pasto(&piano, "Colazione",
                "4 uova intere, pane integrale (80g), avocado (50g), latte (200ml)",
                k*col, pr*col, ca*col, fa*col,
                "Alta densita' calorica e proteica");
            aggiungi_pasto(&piano, "Pre-allenamento",
                "Banana, riso soffiato (30g), 1 misurino whey",
                k*sp1, pr*sp1, ca*sp1, fa*sp1,
                "30-60 min prima dell'allenamento");
            aggiungi_pasto(&piano, "Pranzo",
                "Pasta integrale (100g), tonno (160g), pomodoro, olio EVO, parmigiano (20g)",
                k*pran, pr*pran, ca*pran, fa*pran,
                "Il pasto piu' abbondante della giornata");
            aggiungi_pasto(&piano, "Post-allenamento",
                "1 misurino whey, banana, latte p.s. (300ml)",
                k*sp2, pr*sp2, ca*sp2, fa*sp2,
                "Entro 30 min dal termine dell'allenamento");
            aggiungi_pasto(&piano, "Cena",
                "Manzo magro (180g), riso (80g), broccoli (200g), olio EVO",
                k*cen, pr*cen, ca*cen, fa*cen,
                "Proteine lente per il recupero notturno");
            break;

        default:
            strncpy(piano.etichetta, "Piano Mantenimento / Benessere", DIM_NOME - 1);
            aggiungi_pasto(&piano, "Colazione",
                "Pane integrale (60g), ricotta (80g), marmellata senza zucchero, te'",
                k*col, pr*col, ca*col, fa*col, "");
            aggiungi_pasto(&piano, "Spuntino",
                "Frutta di stagione (150g), mandorle (15g)",
                k*sp1, pr*sp1, ca*sp1, fa*sp1, "");
            aggiungi_pasto(&piano, "Pranzo",
                "Riso (80g), legumi (100g), insalata mista, olio EVO",
                k*pran, pr*pran, ca*pran, fa*pran,
                "Ottima fonte di fibre e proteine vegetali");
            aggiungi_pasto(&piano, "Merenda",
                "Yogurt naturale (125g), 1 cucchiaio di miele",
                k*sp2, pr*sp2, ca*sp2, fa*sp2, "");
            aggiungi_pasto(&piano, "Cena",
                "Pesce al forno (150g), patate (150g), verdure di stagione",
                k*cen, pr*cen, ca*cen, fa*cen, "");
            break;
    }
    return piano;
}


/* ==============================================================
 *  STORICO PESI
 * ============================================================== */

void registra_peso(Utente *u, float nuovo_peso)
{
    RegistrazionePeso *r;

    if (u->num_pesi >= MAX_STORICO) {
        int i;
        for (i = 0; i < MAX_STORICO - 1; i++)
            u->storico_peso[i] = u->storico_peso[i + 1];
        u->num_pesi = MAX_STORICO - 1;
    }
    r = &u->storico_peso[u->num_pesi++];
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
    int confronto;

    if (n < 2) return 0.0f;
    confronto = (n >= 3) ? n - 3 : 0;
    return (u->storico_peso[n - 1].peso_kg - u->storico_peso[confronto].peso_kg)
           / (float)(u->storico_peso[n - 1].settimana
                     - u->storico_peso[confronto].settimana + 1);
}
