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
#include "app_db.h"

/* LAYOUT DINAMICO */
#define FINESTRA_W      GetScreenWidth()
#define FINESTRA_H      GetScreenHeight()
#define SCALA           ((float)GetScreenWidth() / 1100.0f)
#define SC(x)           ((int)((x) * SCALA))

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

/* PALETTE */
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

/* SCHERMATE */
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

/* STATO GLOBALE */
typedef struct {
    Schermata schermata;
    Schermata schermata_precedente;

    char  username[DIM_NOME];
    char  ruolo[DIM_NOME];
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

    char  testo[4096];
    float scroll_y;

    Schermata ultima_caricata;
    float pulse_t;

    /* STATO RICERCA TRAINER (Schede/Storico/Piani cliente)
     *  Indice: 0=schede, 1=storico, 2=piani.
     *  Vive qui (e non in variabili static delle funzioni) cosi'
     *  che il logout (memset di AppState) lo azzeri correttamente. */

    char trainer_cerca[3][DIM_NOME];
    int  trainer_cerca_att[3];
    char trainer_nome_trovato[3][DIM_NOME];
    int  trainer_id_trovato[3];
    int  trainer_caricato[3];

/* STATO SCHERMATA "GESTIONE" TRAINER (ricerca + varianti) */
    char trainer_gestione_cerca[DIM_NOME];
    int  trainer_gestione_cerca_att;
    char trainer_gestione_nome[DIM_NOME];
    int  trainer_gestione_id;
    int  trainer_gestione_var_scheda;
    int  trainer_gestione_var_piano;

} AppState;

/* CAMPI INPUT */
#define MAX_CAMPI 10

typedef struct {
    char testo[256];
    int  e_password;
    char etichetta[64];
} Campo;

extern Campo campi[MAX_CAMPI];
extern int   num_campi;
extern int   campo_attivo_idx;
extern int   reg_sesso;
extern int   reg_attivita;
extern int   reg_servizio;

/* DICHIARAZIONI gui_widgets.c */
void azzera_campi(void);
int  aggiungi_campo(const char *etichetta, int e_password);
void disegna_rettangolo_bordo(int x, int y, int w, int h, int r,
                               Color fill, Color bordo);
int  bottone(int x, int y, int w, int h, const char *testo,
             Color sf, Color ct);
int  bottone_accent(int x, int y, int w, int h, const char *testo);
void disegna_campo(int x, int y, int w, int h, int idx);
void disegna_sidebar(AppState *s);
void imposta_errore(AppState *s, const char *msg);
void imposta_ok(AppState *s, const char *msg);
void disegna_messaggi(AppState *s);
void disegna_header(const char *titolo, const char *sub);
void schermata_testo(AppState *s, const char *titolo, const char *sub);
void schermata_testo_inline(AppState *s, int y0);

/* DICHIARAZIONI gui_schermate.c */
void schermata_benvenuto(AppState *s);
void schermata_login(AppState *s);
void schermata_registra(AppState *s);
void schermata_home(AppState *s);
void schermata_report(AppState *s);
void schermata_feedback(AppState *s);
void schermata_trainer(AppState *s);
void schermata_trainer_schede(AppState *s);
void schermata_trainer_storico(AppState *s);
void schermata_trainer_piani(AppState *s);

#endif