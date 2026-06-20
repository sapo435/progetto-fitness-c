#ifndef CLIENT_UI_H
#define CLIENT_UI_H

/*
 * client_ui.h
 * -----------
 * Input, stampe e flussi interattivi (area cliente e trainer).
 */

#include "client_modello.h"

/* ---- Input ---- */
void  leggi_stringa(const char *etichetta, char *dest, int dim);
float leggi_float(const char *etichetta);
int   leggi_int(const char *etichetta);

/* ---- Output ---- */
void        linea(char c, int n);
const char *stringa_obiettivo(Obiettivo ob);
void        stampa_report(const Utente *u);
void        stampa_programma(const ProgrammaAllenamento *p);
void        stampa_piano(const PianoNutrizionale *piano);

/* ---- Flussi interattivi (richiedono il token di sessione) ---- */
void inserisci_dati(Utente *u);
void sezione_feedback(Utente *u, int id_server, const char *token);
void area_trainer(int id_trainer, const char *token);

#endif
