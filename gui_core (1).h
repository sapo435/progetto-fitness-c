/*
 * gui_core.h
 * ----------
 * Tipi, costanti e dichiarazioni condivisi tra
 * gui_widgets.c, gui_schermate.c e gui_main.c
 */

#ifndef GUI_CORE_H
#define GUI_CORE_H

#include "raylib.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "client_modello.h"
#include "client_logica.h"
#include "client_api.h"

/* ================================================================
 *  COSTANTI DI LAYOUT
 * ================================================================ */

#define FINESTRA_W   1100
#define FINESTRA_H   700
#define SIDEBAR_W    220
#define TITOLO_H     64
#define PADDING      24
#define BTN_H        48
#define BTN_W        320
#define INPUT_H      44
#define INPUT_W      380
#define RAGGIO       8

/* ================================================================
 *  PALETTE
 * ================================================================ */

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

/* ================================================================
 *  ENUMERAZIONI
 * ================================================================ */

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
    SCHERMATA_TRAINER_NUOVA_SCHEDA,
    SCHERMATA_TRAINER_NUOVO_PIANO,
} Schermata;

/* ================================================================
 *  STATO GLOBALE DELL'APPLICAZIONE
 * ================================================================ */

typedef struct {
    Schermata schermata;

    /* Sessione */
    char  token[DIM_TOKEN];
    char  username[DIM_NOME];
    char  ruolo[DIM_NOME];
    int   id_server;

    /* Dati utente calcolati */
    Utente               utente;
    ProgrammaAllenamento programma;
    PianoNutrizionale    piano;
    int                  programma_pronto;
    int                  piano_pronto;

    /* Messaggi di stato */
    char  msg_errore[256];
    char  msg_ok[256];
    float msg_timer;

    /* Testo risposta server (storico, schede, piani) */
    char  testo_server[4096];

    /* Scroll area testo */
    float scroll_y;

    /* Indicatore pulsazione sidebar */
    float pulse_t;

} AppState;

/* ================================================================
 *  CAMPI INPUT
 * ================================================================ */

#define MAX_CAMPI 10

typedef struct {
    char testo[256];
    int  attivo;
    int  e_password;
    char etichetta[64];
} Campo;

/* Variabili globali dei campi — definite in gui_widgets.c */
extern Campo campi[MAX_CAMPI];
extern int   num_campi;
extern int   campo_attivo_idx;

/* Variabili di stato registrazione — definite in gui_schermate.c */
extern int reg_sesso;
extern int reg_attivita;
extern int reg_servizio;

/* ================================================================
 *  DICHIARAZIONI — gui_widgets.c
 * ================================================================ */

void azzera_campi(void);
int  aggiungi_campo(const char *etichetta, int e_password);

void disegna_rettangolo_bordo(int x, int y, int w, int h, int r,
                               Color fill, Color bordo);
int  bottone(int x, int y, int w, int h, const char *testo,
             Color colore_sfondo, Color colore_testo);
int  bottone_accent(int x, int y, int w, int h, const char *testo);
void disegna_campo(int x, int y, int w, int h, int idx);

void disegna_sidebar(AppState *s);
void imposta_errore(AppState *s, const char *msg);
void imposta_ok(AppState *s, const char *msg);
void disegna_messaggi(AppState *s);
void disegna_header(const char *titolo, const char *sottotitolo);
void schermata_testo_server(AppState *s, const char *titolo,
                             const char *sottotitolo);
void schermata_testo_server_inline(AppState *s, int y0);

/* ================================================================
 *  DICHIARAZIONI — gui_schermate.c
 * ================================================================ */

void schermata_benvenuto(AppState *s);
void schermata_login(AppState *s);
void schermata_registra(AppState *s);
void schermata_home(AppState *s);
void schermata_report(AppState *s);
void schermata_feedback(AppState *s);
void schermata_trainer(AppState *s);
void schermata_trainer_lista(AppState *s);
void schermata_trainer_schede(AppState *s);

#endif /* GUI_CORE_H */
