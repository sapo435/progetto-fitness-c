#ifndef GUI_CORE_H
#define GUI_CORE_H

/**
 * @file gui_core.h
 * @brief Header condiviso dell'interfaccia grafica (Raylib): costanti
 *        di layout, palette colori, stato globale dell'applicazione
 *        e dichiarazioni di tutte le funzioni di gui_widgets.c e
 *        delle schermate.
 */

#include "raylib.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "client_modello.h"
#include "client_logica.h"
#include "app_db.h"

/*  LAYOUT DINAMICO
    Tutte le dimensioni sono espresse tramite la macro SC(), che le
    scala in base alla risoluzione corrente della finestra. */
#define FINESTRA_W      GetScreenWidth()
#define FINESTRA_H      GetScreenHeight()
#define SCALA           ((float)GetScreenWidth() / 1100.0f)
#define SC(x)           ((int)((x) * SCALA))  /**< Scala un valore in base alla risoluzione. */

#define SIDEBAR_W       SC(220)
#define TITOLO_H        SC(64)
#define PADDING         SC(24)
#define BTN_H           SC(48)
#define BTN_W           SC(320)
#define INPUT_H         SC(44)
#define INPUT_W         SC(380)
#define RAGGIO          SC(8)
#define FONT_GRANDE     SC(28)
#define FONT_MEDIO      SC(18)
#define FONT_NORMALE    SC(15)
#define FONT_PICCOLO    SC(12)

/*  PALETTE */
#define COL_BG          (Color){13,  13,  13,  255}
#define COL_SURFACE     (Color){26,  26,  26,  255}
#define COL_CARD        (Color){34,  34,  34,  255}
#define COL_ACCENT      (Color){57,  255, 20,  255}
#define COL_ACCENT_DIM  (Color){30,  130, 10,  255}
#define COL_TEXT        (Color){224, 224, 224, 255}
#define COL_TEXT_DIM    (Color){120, 120, 120, 255}
#define COL_DANGER      (Color){255, 80,  80,  255}
#define COL_OK          (Color){57,  255, 20,  255}
#define COL_BORDER      (Color){50,  50,  50,  255}
#define COL_INPUT_BG    (Color){20,  20,  20,  255}
#define COL_HOVER       (Color){45,  45,  45,  255}
#define COL_SIDEBAR_BG  (Color){16,  16,  16,  255}

/** @brief Identifica ciascuna schermata dell'applicazione. */
typedef enum {
    SCHERMATA_BENVENUTO = 0,
    SCHERMATA_LOGIN,
    SCHERMATA_REGISTRA,
    SCHERMATA_HOME,
    SCHERMATA_REPORT,
    SCHERMATA_FEEDBACK,
    SCHERMATA_STORICO,
    SCHERMATA_SCHEDE,
    SCHERMATA_PIANI,
    SCHERMATA_TRAINER,
    SCHERMATA_TRAINER_LISTA,
    SCHERMATA_TRAINER_SCHEDE,
    SCHERMATA_TRAINER_STORICO,
    SCHERMATA_TRAINER_PIANI,
} Schermata;

/**
 * @brief Stato globale dell'applicazione: sessione utente, schermata
 *        corrente, dati caricati e cache, messaggi a schermo, stato
 *        di ricerca lato trainer. Azzerato interamente al logout.
 */
typedef struct {
    Schermata schermata;              /**< Schermata attualmente mostrata. */
    Schermata schermata_precedente;

    char  username[DIM_NOME];
    char  ruolo[DIM_NOME];             /**< "cliente" o "trainer". */
    int   id_utente;
    int   id_cliente_sel;

    Utente               utente;
    ProgrammaAllenamento programma;
    PianoNutrizionale    piano;
    int                  programma_pronto;
    int                  piano_pronto;

    char  msg_errore[256];
    char  msg_ok[256];
    float msg_timer;

    char  testo[4096];                 /**< Buffer di testo condiviso dalle schermate a scorrimento. */
    float scroll_y;

    Schermata ultima_caricata;         /**< Ultima schermata che ha popolato testo[], per evitare ricariche inutili. */
    float pulse_t;                     /**< Timer per l'animazione pulsante nella sidebar. */

    /* Stato ricerca trainer per schede (0), storico (1) e piani (2). */
    char trainer_cerca[3][DIM_NOME];
    int  trainer_cerca_att[3];
    char trainer_nome_trovato[3][DIM_NOME];
    int  trainer_id_trovato[3];
    int  trainer_caricato[3];

    /* Stato schermata "Gestione" trainer (ricerca + varianti). */
    char trainer_gestione_cerca[DIM_NOME];
    int  trainer_gestione_cerca_att;
    char trainer_gestione_nome[DIM_NOME];
    int  trainer_gestione_id;
    int  trainer_gestione_var_scheda;
    int  trainer_gestione_var_piano;

} AppState;

/** @brief Numero massimo di campi input gestibili contemporaneamente. */
#define MAX_CAMPI 10

/** @brief Singolo campo di testo editabile (username, password, dati anagrafici...). */
typedef struct {
    char testo[256];
    int  e_password;    /**< 1 se il campo deve mostrare asterischi invece del testo. */
    char etichetta[64];
} Campo;

extern Campo campi[MAX_CAMPI];
extern int   num_campi;
extern int   campo_attivo_idx;
extern int   reg_sesso;
extern int   reg_attivita;
extern int   reg_servizio;
extern int   reg_ruolo;

/* DICHIARAZIONI gui_widgets.c */

/** @brief Azzera tutti i campi input e disattiva quello attivo. */
void azzera_campi(void);

/**
 * @brief Aggiunge un nuovo campo input all'array globale campi[].
 * @param etichetta Testo dell'etichetta mostrata sopra il campo.
 * @param e_password 1 se il campo deve mascherare l'input, 0 altrimenti.
 * @return Indice del campo appena creato in campi[].
 */
int  aggiungi_campo(const char *etichetta, int e_password);

/** @brief Disegna un rettangolo arrotondato con bordo. */
void disegna_rettangolo_bordo(int x, int y, int w, int h, int r,
                               Color fill, Color bordo);

/**
 * @brief Disegna un bottone cliccabile con effetto hover.
 * @return 1 se il bottone e' stato cliccato in questo frame, 0 altrimenti.
 */
int  bottone(int x, int y, int w, int h, const char *testo,
             Color sf, Color ct);

/**
 * @brief Disegna un bottone in stile "accent" (colore principale del tema).
 * @return 1 se il bottone e' stato cliccato in questo frame, 0 altrimenti.
 */
int  bottone_accent(int x, int y, int w, int h, const char *testo);

/**
 * @brief Disegna e gestisce l'input di un campo di testo editabile.
 * @param x Coordinata X del campo.
 * @param y Coordinata Y del campo.
 * @param w Larghezza del campo.
 * @param h Altezza del campo.
 * @param idx Indice del campo in campi[] da disegnare.
 */
void disegna_campo(int x, int y, int w, int h, int idx);

/**
 * @brief Disegna la sidebar di navigazione (menu diverso per cliente/trainer).
 * @param s Stato applicativo corrente.
 */
void disegna_sidebar(AppState *s);

/** @brief Imposta un messaggio di errore da mostrare temporaneamente a schermo. */
void imposta_errore(AppState *s, const char *msg);

/** @brief Imposta un messaggio di conferma da mostrare temporaneamente a schermo. */
void imposta_ok(AppState *s, const char *msg);

/** @brief Disegna il messaggio di errore/conferma corrente, se presente. */
void disegna_messaggi(AppState *s);

/** @brief Disegna il titolo e sottotitolo in cima a una schermata. */
void disegna_header(const char *titolo, const char *sub);

/**
 * @brief Disegna una schermata a scorrimento verticale con header,
 *        mostrando il contenuto di s->testo riga per riga.
 */
void schermata_testo(AppState *s, const char *titolo, const char *sub);

/**
 * @brief Come schermata_testo(), ma senza disegnare l'header (usata
 *        per aree di testo incluse dentro schermate gia' composte).
 * @param s Stato applicativo corrente.
 * @param y0 Coordinata Y da cui iniziare a disegnare il testo.
 */
void schermata_testo_inline(AppState *s, int y0);

/* DICHIARAZIONI schermate (gui_schermate_auth/cliente/trainer.c)  */

/** @brief Schermata iniziale con i bottoni "Accedi" e "Crea un account". */
void schermata_benvenuto(AppState *s);

/** @brief Schermata di login con username e password. */
void schermata_login(AppState *s);

/** @brief Schermata di registrazione di un nuovo cliente (2 step). */
void schermata_registra(AppState *s);

/** @brief Home del cliente: riepilogo metriche e accesso rapido alle sezioni. */
void schermata_home(AppState *s);

/** @brief Report dettagliato con metriche, programma e piano del cliente. */
void schermata_report(AppState *s);

/** @brief Form di feedback settimanale (peso, allenamento, dieta). */
void schermata_feedback(AppState *s);

/** @brief Area trainer per generare/sostituire schede e piani di un cliente. */
void schermata_trainer(AppState *s);

/** @brief Ricerca cliente e visualizzazione delle sue schede (lato trainer). */
void schermata_trainer_schede(AppState *s);

/** @brief Ricerca cliente e visualizzazione del suo storico pesi (lato trainer). */
void schermata_trainer_storico(AppState *s);

/** @brief Ricerca cliente e visualizzazione dei suoi piani (lato trainer). */
void schermata_trainer_piani(AppState *s);

#endif