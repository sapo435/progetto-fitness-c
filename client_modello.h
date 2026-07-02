#ifndef CLIENT_MODELLO_H
#define CLIENT_MODELLO_H

/**
 * @file client_modello.h
 * @brief Modello dati condiviso: costanti, enumerazioni e strutture
 *        usate da tutti i moduli dell'applicazione.
 */

#define MAX_ESERCIZI  10   /**< Numero massimo di esercizi per sessione. */
#define MAX_SESSIONI  10   /**< Numero massimo di sessioni per programma. */
#define MAX_PASTI      6   /**< Numero massimo di pasti per piano nutrizionale. */
#define MAX_STORICO   20   /**< Numero massimo di registrazioni peso in memoria. */
#define DIM_NOME      64   /**< Lunghezza massima per nomi, titoli, username. */
#define DIM_TESTO    256   /**< Lunghezza massima per descrizioni e note. */
#define DIM_TOKEN    128   /**< Lunghezza massima per singoli campi/token. */

/**
 * @brief Obiettivo fisico del cliente, determinato da BMI e peso desiderato.
 * @see determina_obiettivo()
 */
typedef enum {
    OBIETTIVO_DIMAGRIMENTO = 0, /**< Perdita di peso / riduzione BMI. */
    OBIETTIVO_MASSA = 1,        /**< Aumento di peso / massa muscolare. */
    OBIETTIVO_MANTENIMENTO = 2  /**< Mantenimento del peso attuale. */
} Obiettivo;

/**
 * @brief Livello di esperienza dedotto dal livello di attivita' fisica.
 * @see determina_livello()
 */
typedef enum {
    LIVELLO_PRINCIPIANTE = 0,
    LIVELLO_INTERMEDIO = 1,
    LIVELLO_AVANZATO = 2
} Livello;

/**
 * @brief Servizio scelto dal cliente in fase di registrazione:
 *        determina quali sezioni (schede/piani) gli vengono mostrate.
 */
typedef enum {
    SERVIZIO_SOLO_ALLENAMENTO = 0,
    SERVIZIO_SOLO_NUTRIZIONE = 1,
    SERVIZIO_ENTRAMBI = 2
} Servizio;

/** @brief Singola registrazione di peso nello storico del cliente. */
typedef struct {
    float peso_kg;   /**< Peso registrato in chilogrammi. */
    int   settimana;  /**< Settimana di programma in cui e' stata fatta la registrazione. */
} RegistrazionePeso;

/**
 * @brief Rappresenta un utente dell'applicazione (cliente o trainer),
 *        con dati anagrafici, metriche calcolate e storico pesi.
 */
typedef struct {
    char username[DIM_NOME], nome[DIM_NOME], cognome[DIM_NOME];
    char      sesso;              /**< 'M' o 'F'. */
    int       eta;
    float     altezza_cm, peso_kg, peso_obiettivo_kg;
    Obiettivo obiettivo;
    Servizio  servizio;
    float     livello_attivita;   /**< Fattore moltiplicativo per il calcolo del TDEE. */
    float     bmi, bmr, tdee_kcal;
    float     kcal_obiettivo, proteine_g, carboidrati_g, grassi_g;
    RegistrazionePeso storico_peso[MAX_STORICO];
    int       num_pesi, sessioni_completate, settimana_corrente;
} Utente;

/** @brief Singolo esercizio all'interno di una sessione di allenamento. */
typedef struct {
    char nome_esercizio[DIM_NOME], gruppo_muscolare[DIM_NOME];
    int  serie, ripetizioni, riposo_sec;
    char note[DIM_TESTO];
} Esercizio;

/** @brief Sessione di allenamento (es. "Giorno 1 - Petto e tricipiti"). */
typedef struct {
    char titolo[DIM_NOME];
    int  giorno_settimana;
    Esercizio esercizi[MAX_ESERCIZI];
    int num_esercizi;
} Sessione;

/** @brief Programma di allenamento completo, composto da piu' sessioni. */
typedef struct {
    char nome[DIM_NOME], descrizione[DIM_TESTO];
    Obiettivo obiettivo;
    Livello livello;
    int durata_settimane;
    Sessione sessioni[MAX_SESSIONI];
    int num_sessioni;
} ProgrammaAllenamento;

/** @brief Singolo pasto all'interno di un piano nutrizionale. */
typedef struct {
    char nome[DIM_NOME], alimenti[DIM_TESTO];
    float kcal, proteine_g, carboidrati_g, grassi_g;
    char note[DIM_TESTO];
} Pasto;

/** @brief Piano nutrizionale giornaliero completo, composto da piu' pasti. */
typedef struct {
    char etichetta[DIM_NOME];
    Pasto pasti[MAX_PASTI];
    int num_pasti;
} PianoNutrizionale;

#endif
