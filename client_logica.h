#ifndef LOGICA_H
#define LOGICA_H

/**
 * @file client_logica.h
 * @brief Header unico per la logica applicativa: calcolo metriche
 *        fisiologiche, costruzione di programmi di allenamento e
 *        piani nutrizionali, gestione dello storico pesi.
 */

#include "client_modello.h"

/* ---- Metriche fisiologiche ---- */

/**
 * @brief Calcola il BMI (Body Mass Index).
 * @param peso_kg Peso corporeo in chilogrammi.
 * @param altezza_cm Altezza in centimetri.
 * @return Il valore del BMI.
 */
float calcola_bmi(float peso_kg, float altezza_cm);

/**
 * @brief Restituisce la categoria di peso corrispondente a un BMI.
 * @param bmi Valore del BMI.
 * @return Stringa costante: "Sottopeso", "Normopeso", "Sovrappeso" o "Obesita'".
 */
const char *categoria_bmi(float bmi);

/**
 * @brief Calcola il BMR (metabolismo basale) con la formula di Mifflin-St Jeor.
 * @param peso_kg Peso corporeo in chilogrammi.
 * @param altezza_cm Altezza in centimetri.
 * @param eta Eta' in anni.
 * @param sesso 'M' o 'F'.
 * @return BMR in kcal/giorno.
 */
float calcola_bmr(float peso_kg, float altezza_cm, int eta, char sesso);

/**
 * @brief Calcola il TDEE (dispendio energetico totale giornaliero).
 * @param bmr Metabolismo basale in kcal.
 * @param livello_attivita Fattore moltiplicativo (1.2 - 1.9).
 * @return TDEE in kcal/giorno.
 */
float calcola_tdee(float bmr, float livello_attivita);

/**
 * @brief Calcola le kcal obiettivo giornaliere in base all'obiettivo del cliente.
 * @param tdee Dispendio energetico totale in kcal.
 * @param ob Obiettivo del cliente (dimagrimento/massa/mantenimento).
 * @return Kcal obiettivo giornaliere.
 */
float calcola_kcal_obiettivo(float tdee, Obiettivo ob);

/**
 * @brief Calcola la ripartizione dei macronutrienti (proteine, grassi, carboidrati).
 * @param peso_kg Peso corporeo in chilogrammi.
 * @param kcal_ob Kcal obiettivo giornaliere.
 * @param ob Obiettivo del cliente.
 * @param[out] proteine Grammi di proteine giornaliere.
 * @param[out] grassi Grammi di grassi giornalieri.
 * @param[out] carboidrati Grammi di carboidrati giornalieri.
 */
void calcola_macronutrienti(float peso_kg, float kcal_ob, Obiettivo ob,
                            float *proteine, float *grassi, float *carboidrati);

/**
 * @brief Ricalcola tutte le metriche di un utente (BMR, TDEE, kcal, macro)
 *        a partire dai suoi dati anagrafici e dall'obiettivo corrente.
 * @param[in,out] u Utente di cui ricalcolare le metriche.
 */
void calcola_metriche(Utente *u);

/**
 * @brief Stima il numero di settimane necessarie a raggiungere il peso obiettivo.
 * @param u Utente di cui stimare il tempo.
 * @return Settimane stimate, oppure 0 se l'obiettivo e' gia' raggiunto.
 */
float stima_settimane(const Utente *u);

/**
 * @brief Calcola il fabbisogno idrico giornaliero.
 * @param peso_kg Peso corporeo in chilogrammi.
 * @param ore_allenamento Ore di allenamento previste nella giornata.
 * @return Fabbisogno idrico in millilitri.
 */
float fabbisogno_acqua(float peso_kg, float ore_allenamento);

/**
 * @brief Determina l'obiettivo del cliente in base a BMI e differenza
 *        tra peso attuale e peso desiderato.
 * @param u Utente da valutare.
 * @return OBIETTIVO_DIMAGRIMENTO, OBIETTIVO_MASSA o OBIETTIVO_MANTENIMENTO.
 */
Obiettivo determina_obiettivo(const Utente *u);

/**
 * @brief Determina il livello di esperienza a partire dal livello di attivita'.
 * @param livello_attivita Fattore moltiplicativo (1.2 - 1.9).
 * @return LIVELLO_PRINCIPIANTE, LIVELLO_INTERMEDIO o LIVELLO_AVANZATO.
 */
Livello determina_livello(float livello_attivita);

/* ---- Programma di allenamento ---- */

/**
 * @brief Aggiunge un esercizio a una sessione di allenamento.
 * @param[in,out] s Sessione a cui aggiungere l'esercizio.
 * @param nome Nome dell'esercizio.
 * @param muscolo Gruppo muscolare coinvolto.
 * @param serie Numero di serie.
 * @param rip Numero di ripetizioni per serie.
 * @param riposo Tempo di riposo in secondi tra le serie.
 * @param note Note aggiuntive (puo' essere stringa vuota).
 */
void aggiungi_esercizio(Sessione *s, const char *nome,
                        const char *muscolo, int serie,
                        int rip, int riposo, const char *note);

/**
 * @brief Aggiunge una nuova sessione a un programma di allenamento.
 * @param[in,out] p Programma a cui aggiungere la sessione.
 * @param titolo Titolo della sessione.
 * @param giorno Giorno della settimana associato (0-6).
 * @return Puntatore alla sessione appena creata.
 */
Sessione *aggiungi_sessione(ProgrammaAllenamento *p,
                            const char *titolo, int giorno);

/**
 * @brief Costruisce il programma di allenamento standard (variante 0)
 *        in base all'obiettivo dell'utente.
 * @param u Utente per cui generare il programma.
 * @return Programma di allenamento generato.
 */
ProgrammaAllenamento costruisci_programma(const Utente *u);

/**
 * @brief Costruisce un programma di allenamento con variante specifica.
 * @param u Utente per cui generare il programma.
 * @param variante 0 = base, 1 = intensivo/alternativo, 2 = avanzato/split.
 * @return Programma di allenamento generato.
 */
ProgrammaAllenamento costruisci_programma_ex(const Utente *u, int variante);

/* ---- Piano nutrizionale ---- */

/**
 * @brief Aggiunge un pasto a un piano nutrizionale.
 * @param[in,out] piano Piano a cui aggiungere il pasto.
 * @param nome Nome del pasto (es. "Colazione").
 * @param alimenti Descrizione degli alimenti.
 * @param kcal Kcal del pasto.
 * @param prot Grammi di proteine.
 * @param carb Grammi di carboidrati.
 * @param gras Grammi di grassi.
 * @param note Note aggiuntive (puo' essere stringa vuota).
 */
void aggiungi_pasto(PianoNutrizionale *piano, const char *nome,
                    const char *alimenti, float kcal,
                    float prot, float carb, float gras,
                    const char *note);

/**
 * @brief Costruisce il piano nutrizionale standard (variante 0)
 *        in base all'obiettivo dell'utente.
 * @param u Utente per cui generare il piano.
 * @return Piano nutrizionale generato.
 */
PianoNutrizionale costruisci_piano(const Utente *u);

/**
 * @brief Costruisce un piano nutrizionale con variante specifica.
 * @param u Utente per cui generare il piano.
 * @param variante 0 = base, 1 = alternativo, 2 = avanzato.
 * @return Piano nutrizionale generato.
 */
PianoNutrizionale costruisci_piano_ex(const Utente *u, int variante);

/* ---- Storico pesi ---- */

/**
 * @brief Registra un nuovo peso nello storico dell'utente e ricalcola
 *        le sue metriche (BMI, obiettivo, kcal, macro).
 * @param[in,out] u Utente su cui registrare il peso.
 * @param nuovo_peso Nuovo peso in chilogrammi.
 */
void registra_peso(Utente *u, float nuovo_peso);

/**
 * @brief Calcola la variazione media di peso settimanale sulle ultime
 *        registrazioni dello storico.
 * @param u Utente di cui calcolare la variazione.
 * @return Variazione media in kg/settimana (negativa se in calo).
 */
float variazione_media_peso(const Utente *u);

#endif /* LOGICA_H */
