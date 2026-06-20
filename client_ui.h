#ifndef INTERFACCIA_H
#define INTERFACCIA_H

/*
 * interfaccia.h
 * -------------
 * Input da tastiera, stampe a video e flussi interattivi
 * (area cliente e area trainer).
 * La comunicazione col server e' nel modulo server.h/server.c.
 */

#include "modello.h"

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

/* ---- Flussi interattivi ---- */
void inserisci_dati(Utente *u);
void sezione_feedback(Utente *u, int id_server);
void area_trainer(int id_trainer);

#endif /* INTERFACCIA_H */
