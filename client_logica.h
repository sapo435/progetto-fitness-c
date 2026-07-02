#ifndef LOGICA_H
#define LOGICA_H

#include "client_modello.h"

/* Metriche fisiologiche */
float       calcola_bmi(float peso_kg, float altezza_cm);
const char *categoria_bmi(float bmi);
float       calcola_bmr(float peso_kg, float altezza_cm, int eta, char sesso);
float       calcola_tdee(float bmr, float livello_attivita);
float       calcola_kcal_obiettivo(float tdee, Obiettivo ob);
void        calcola_macronutrienti(float peso_kg, float kcal_ob, Obiettivo ob,
                                   float *proteine, float *grassi, float *carboidrati);
void        calcola_metriche(Utente *u);
float       stima_settimane(const Utente *u);
float       fabbisogno_acqua(float peso_kg, float ore_allenamento);
Obiettivo   determina_obiettivo(const Utente *u);
Livello     determina_livello(float livello_attivita);

/* Programma di allenamento */
void      aggiungi_esercizio(Sessione *s, const char *nome,
                             const char *muscolo, int serie,
                             int rip, int riposo, const char *note);
Sessione *aggiungi_sessione(ProgrammaAllenamento *p,
                            const char *titolo, int giorno);

/* Programma standard (variante 0) */
ProgrammaAllenamento costruisci_programma(const Utente *u);

/* Programmi con variante 0-2:
 *   0 = base, 1 = intensivo/alternativo, 2 = avanzato/split
 */
ProgrammaAllenamento costruisci_programma_ex(const Utente *u, int variante);

/* Piano nutrizionale */
void aggiungi_pasto(PianoNutrizionale *piano, const char *nome,
                    const char *alimenti, float kcal,
                    float prot, float carb, float gras,
                    const char *note);

/* Piano standard (variante 0) */
PianoNutrizionale costruisci_piano(const Utente *u);

/* Piani con variante 0-2 */
PianoNutrizionale costruisci_piano_ex(const Utente *u, int variante);

/* Storico pesi */
void  registra_peso(Utente *u, float nuovo_peso);
float variazione_media_peso(const Utente *u);

#endif /* LOGICA_H */