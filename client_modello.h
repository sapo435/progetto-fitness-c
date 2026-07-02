#ifndef CLIENT_MODELLO_H
#define CLIENT_MODELLO_H

/*
 * client_modello.h
 * Modello dati condiviso: costanti, enumerazioni e strutture.
 */

#define MAX_ESERCIZI  10
#define MAX_SESSIONI  10
#define MAX_PASTI      6
#define MAX_STORICO   20
#define DIM_NOME      64
#define DIM_TESTO    256
#define DIM_TOKEN    128

typedef enum {
    OBIETTIVO_DIMAGRIMENTO = 0, OBIETTIVO_MASSA = 1,
    OBIETTIVO_MANTENIMENTO = 2
} Obiettivo;

typedef enum {
    LIVELLO_PRINCIPIANTE = 0, LIVELLO_INTERMEDIO = 1, LIVELLO_AVANZATO = 2
} Livello;

typedef enum {
    SERVIZIO_SOLO_ALLENAMENTO = 0, SERVIZIO_SOLO_NUTRIZIONE = 1,
    SERVIZIO_ENTRAMBI = 2
} Servizio;

typedef struct { float peso_kg; int settimana; } RegistrazionePeso;

typedef struct {
    char username[DIM_NOME], nome[DIM_NOME], cognome[DIM_NOME];
    char      sesso;
    int       eta;
    float     altezza_cm, peso_kg, peso_obiettivo_kg;
    Obiettivo obiettivo;
    Servizio  servizio;
    float     livello_attivita, bmi, bmr, tdee_kcal;
    float     kcal_obiettivo, proteine_g, carboidrati_g, grassi_g;
    RegistrazionePeso storico_peso[MAX_STORICO];
    int       num_pesi, sessioni_completate, settimana_corrente;
} Utente;

typedef struct {
    char nome_esercizio[DIM_NOME], gruppo_muscolare[DIM_NOME];
    int  serie, ripetizioni, riposo_sec;
    char note[DIM_TESTO];
} Esercizio;

typedef struct {
    char titolo[DIM_NOME];
    int  giorno_settimana;
    Esercizio esercizi[MAX_ESERCIZI];
    int num_esercizi;
} Sessione;

typedef struct {
    char nome[DIM_NOME], descrizione[DIM_TESTO];
    Obiettivo obiettivo; Livello livello;
    int durata_settimane;
    Sessione sessioni[MAX_SESSIONI];
    int num_sessioni;
} ProgrammaAllenamento;

typedef struct {
    char nome[DIM_NOME], alimenti[DIM_TESTO];
    float kcal, proteine_g, carboidrati_g, grassi_g;
    char note[DIM_TESTO];
} Pasto;

typedef struct {
    char etichetta[DIM_NOME];
    Pasto pasti[MAX_PASTI];
    int num_pasti;
} PianoNutrizionale;

#endif