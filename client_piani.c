#include <stdio.h>
#include <string.h>
#include <math.h>
#include "client_logica.h"

/**
 * @file client_piani.c
 * @brief Costruzione dei piani nutrizionali: funzione helper
 *        (aggiungi_pasto) e le funzioni che generano il piano standard
 *        e le 3 varianti per obiettivo.
 */

/* PIANO NUTRIZIONALE */
void aggiungi_pasto(PianoNutrizionale *piano, const char *nome,
                    const char *alimenti, float kcal,
                    float prot, float carb, float gras,
                    const char *note)
{
    if (piano->num_pasti >= MAX_PASTI) return;
    Pasto *m = &piano->pasti[piano->num_pasti++];
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
    memset(&piano, 0, sizeof(PianoNutrizionale));

    float k   = u->kcal_obiettivo;
    float pr  = u->proteine_g;
    float ca  = u->carboidrati_g;
    float fa  = u->grassi_g;
    float col = 0.25f, sp1 = 0.10f, pran = 0.35f, sp2 = 0.10f, cen = 0.20f;

    switch (u->obiettivo) {
        case OBIETTIVO_DIMAGRIMENTO:
            strncpy(piano.etichetta, "Piano Dimagrimento", DIM_NOME - 1);
            aggiungi_pasto(&piano, "Colazione",
                "Yogurt greco 0% (200g) avena (50g) frutti rossi (100g) caffe'",
                k*col, pr*col, ca*col, fa*col, "Ricco di proteine per sazieta'");
            aggiungi_pasto(&piano, "Spuntino mattina",
                "1 mela media 10 mandorle",
                k*sp1, pr*sp1, ca*sp1, fa*sp1, "Evita i picchi glicemici");
            aggiungi_pasto(&piano, "Pranzo",
                "Petto di pollo (150g) insalata mista (200g) pane integrale (50g) olio EVO",
                k*pran, pr*pran, ca*pran, fa*pran, "Priorita' a verdure e proteine magre");
            aggiungi_pasto(&piano, "Spuntino pomeriggio",
                "Ricotta light (100g) 1 frutto di stagione",
                k*sp2, pr*sp2, ca*sp2, fa*sp2, "");
            aggiungi_pasto(&piano, "Cena",
                "Salmone (120g) verdure grigliate (300g) olio EVO",
                k*cen, pr*cen, ca*cen, fa*cen, "Pochi carboidrati la sera");
            break;

        case OBIETTIVO_MASSA:
            strncpy(piano.etichetta, "Piano Massa Muscolare", DIM_NOME - 1);
            aggiungi_pasto(&piano, "Colazione",
                "4 uova intere pane integrale (80g) avocado (50g) latte (200ml)",
                k*col, pr*col, ca*col, fa*col, "Alta densita' calorica e proteica");
            aggiungi_pasto(&piano, "Pre-allenamento",
                "Banana riso soffiato (30g) 1 misurino whey",
                k*sp1, pr*sp1, ca*sp1, fa*sp1, "30-60 min prima dell'allenamento");
            aggiungi_pasto(&piano, "Pranzo",
                "Pasta integrale (100g) tonno (160g) pomodoro olio EVO parmigiano (20g)",
                k*pran, pr*pran, ca*pran, fa*pran, "Il pasto piu' abbondante");
            aggiungi_pasto(&piano, "Post-allenamento",
                "1 misurino whey banana latte p.s. (300ml)",
                k*sp2, pr*sp2, ca*sp2, fa*sp2, "Entro 30 min dal termine");
            aggiungi_pasto(&piano, "Cena",
                "Manzo magro (180g) riso (80g) broccoli (200g) olio EVO",
                k*cen, pr*cen, ca*cen, fa*cen, "Proteine lente per il recupero");
            break;

        default:
            strncpy(piano.etichetta, "Piano Mantenimento / Benessere", DIM_NOME - 1);
            aggiungi_pasto(&piano, "Colazione",
                "Pane integrale (60g) ricotta (80g) marmellata senza zucchero te'",
                k*col, pr*col, ca*col, fa*col, "");
            aggiungi_pasto(&piano, "Spuntino",
                "Frutta di stagione (150g) mandorle (15g)",
                k*sp1, pr*sp1, ca*sp1, fa*sp1, "");
            aggiungi_pasto(&piano, "Pranzo",
                "Riso (80g) legumi (100g) insalata mista olio EVO",
                k*pran, pr*pran, ca*pran, fa*pran, "Ottima fonte di fibre e proteine vegetali");
            aggiungi_pasto(&piano, "Merenda",
                "Yogurt naturale (125g) 1 cucchiaio di miele",
                k*sp2, pr*sp2, ca*sp2, fa*sp2, "");
            aggiungi_pasto(&piano, "Cena",
                "Pesce al forno (150g) patate (150g) verdure di stagione",
                k*cen, pr*cen, ca*cen, fa*cen, "");
            break;
    }
    return piano;
}

/* PIANO NUTRIZIONALE CON VARIANTE (0=base, 1=alternativo, 2=avanzato) */
PianoNutrizionale costruisci_piano_ex(const Utente *u, int variante)
{
    PianoNutrizionale piano;
    memset(&piano, 0, sizeof(piano));

    float k   = u->kcal_obiettivo;
    float pr  = u->proteine_g;
    float ca  = u->carboidrati_g;
    float fa  = u->grassi_g;
    float col=0.25f, sp1=0.10f, pran=0.35f, sp2=0.10f, cen=0.20f;

    if (u->obiettivo == OBIETTIVO_DIMAGRIMENTO) {

        if (variante == 1) {
            strncpy(piano.etichetta, "Low Carb High Protein", DIM_NOME-1);
            /* Proteine aumentate, carboidrati ridotti */
            float pr2 = pr * 1.2f, ca2 = ca * 0.6f, fa2 = fa * 1.1f;
            aggiungi_pasto(&piano, "Colazione",
                "4 albumi + 2 uova intere strapazzati, verdure grigliate, caffe' senza zucchero",
                k*col, pr2*col, ca2*col, fa2*col,
                "Alta proteina al mattino");
            aggiungi_pasto(&piano, "Spuntino",
                "Ricotta light (150g), noci (20g)",
                k*sp1, pr2*sp1, ca2*sp1, fa2*sp1, "");
            aggiungi_pasto(&piano, "Pranzo",
                "Petto di tacchino (180g), insalata mista (300g), olio EVO, limone",
                k*pran, pr2*pran, ca2*pran, fa2*pran,
                "No carboidrati a pranzo");
            aggiungi_pasto(&piano, "Post-allenamento",
                "Yogurt greco 0% (200g), proteine in polvere (20g)",
                k*sp2, pr2*sp2, ca2*sp2, fa2*sp2, "");
            aggiungi_pasto(&piano, "Cena",
                "Merluzzo al vapore (200g), broccoli (300g), riso integrale (50g)",
                k*cen, pr2*cen, ca2*cen, fa2*cen,
                "Carboidrati solo la sera post-workout");

        } else if (variante == 2) {
            strncpy(piano.etichetta, "Deficit Moderato Mediterraneo", DIM_NOME-1);
            aggiungi_pasto(&piano, "Colazione",
                "Pane integrale (60g), ricotta (80g), pomodori, olio EVO, caffe'",
                k*col, pr*col, ca*col, fa*col, "");
            aggiungi_pasto(&piano, "Spuntino",
                "Frutta fresca (200g), mandorle (15g)",
                k*sp1, pr*sp1, ca*sp1, fa*sp1, "");
            aggiungi_pasto(&piano, "Pranzo",
                "Pasta integrale (80g), tonno (120g), pomodorini, basilico, olio EVO",
                k*pran, pr*pran, ca*pran, fa*pran,
                "Piatto mediterraneo bilanciato");
            aggiungi_pasto(&piano, "Spuntino",
                "Hummus (60g), carote e sedano crudi",
                k*sp2, pr*sp2, ca*sp2, fa*sp2, "");
            aggiungi_pasto(&piano, "Cena",
                "Spigola al forno (160g), verdure arrostite miste (350g), olio EVO",
                k*cen, pr*cen, ca*cen, fa*cen,
                "Leggero e digeribile");

        } else {
            return costruisci_piano(u);
        }

    } else if (u->obiettivo == OBIETTIVO_MASSA) {

        if (variante == 1) {
            strncpy(piano.etichetta, "Clean Bulk", DIM_NOME-1);
            float k2 = k * 1.05f; /* surplus pulito */
            float pr2 = pr * 1.1f;
            aggiungi_pasto(&piano, "Colazione",
                "Fiocchi d'avena (80g), latte p.s. (300ml), 2 uova intere, banana",
                k2*col, pr2*col, ca*col, fa*col,
                "Carboidrati complessi al mattino");
            aggiungi_pasto(&piano, "Pre-allenamento",
                "Riso (80g), petto di pollo (120g), verdure",
                k2*sp1, pr2*sp1, ca*sp1, fa*sp1,
                "60-90 min prima dell'allenamento");
            aggiungi_pasto(&piano, "Pranzo",
                "Pasta integrale (120g), manzo magro (150g), pomodoro, parmigiano",
                k2*pran, pr2*pran, ca*pran, fa*pran,
                "Pasto piu' abbondante");
            aggiungi_pasto(&piano, "Post-allenamento",
                "Shake proteico (30g whey) + banana + latte (200ml)",
                k2*sp2, pr2*sp2, ca*sp2, fa*sp2,
                "Entro 30 min dal termine");
            aggiungi_pasto(&piano, "Cena",
                "Salmone (180g), riso integrale (80g), broccoli (200g), olio EVO",
                k2*cen, pr2*cen, ca*cen, fa*cen,
                "Proteine a lento rilascio per il recupero");

        } else if (variante == 2) {
            strncpy(piano.etichetta, "High Protein Bulk", DIM_NOME-1);
            float pr2 = pr * 1.3f; /* 2g/kg di peso corporeo */
            float k2  = k * 1.08f;
            aggiungi_pasto(&piano, "Colazione",
                "6 albumi + 2 uova intere, avena (100g), latte (300ml), miele",
                k2*col, pr2*col, ca*col, fa*col,
                "35-40g proteina a colazione");
            aggiungi_pasto(&piano, "Spuntino mattina",
                "Cottage cheese (200g), frutta (150g), noci (25g)",
                k2*sp1, pr2*sp1, ca*sp1, fa*sp1, "");
            aggiungi_pasto(&piano, "Pranzo",
                "Riso (120g), manzo 90% magro (200g), spinaci saltati, olio EVO",
                k2*pran, pr2*pran, ca*pran, fa*pran,
                "Il pasto piu' proteico della giornata");
            aggiungi_pasto(&piano, "Post-allenamento",
                "Caseine shake (35g) + patate dolci bollite (200g)",
                k2*sp2, pr2*sp2, ca*sp2, fa*sp2,
                "Caseine per recupero prolungato");
            aggiungi_pasto(&piano, "Cena",
                "Petto di pollo (200g), quinoa (100g), verdure miste al vapore",
                k2*cen, pr2*cen, ca*cen, fa*cen,
                "Leggero ma ricco di proteine");

        } else {
            return costruisci_piano(u);
        }

    } else {

        if (variante == 1) {
            strncpy(piano.etichetta, "Piano Mediterraneo", DIM_NOME-1);
            aggiungi_pasto(&piano, "Colazione",
                "Yogurt greco intero (150g), granola (50g), miele, frutta fresca",
                k*col, pr*col, ca*col, fa*col, "");
            aggiungi_pasto(&piano, "Spuntino",
                "Olive (30g), pane integrale (40g), formaggio fresco (50g)",
                k*sp1, pr*sp1, ca*sp1, fa*sp1, "");
            aggiungi_pasto(&piano, "Pranzo",
                "Insalata di ceci (150g), tonno (100g), peperoni, olive, olio EVO",
                k*pran, pr*pran, ca*pran, fa*pran,
                "Legumi come proteina principale");
            aggiungi_pasto(&piano, "Merenda",
                "Frutta secca mista (30g), frutta fresca (150g)",
                k*sp2, pr*sp2, ca*sp2, fa*sp2, "");
            aggiungi_pasto(&piano, "Cena",
                "Sardine al limone (150g), couscous (80g), verdure grigliate, olio EVO",
                k*cen, pr*cen, ca*cen, fa*cen,
                "Omega-3 abbondanti");

        } else if (variante == 2) {
            strncpy(piano.etichetta, "Piano Flessibile Bilanciato", DIM_NOME-1);
            aggiungi_pasto(&piano, "Colazione",
                "A scelta: uova o cereali o yogurt + frutta. Caffe'.",
                k*col, pr*col, ca*col, fa*col,
                "Flessibilita' nella scelta");
            aggiungi_pasto(&piano, "Spuntino",
                "Frutta (150g) + proteine (yogurt o ricotta 100g)",
                k*sp1, pr*sp1, ca*sp1, fa*sp1, "");
            aggiungi_pasto(&piano, "Pranzo",
                "Piatto con: 1/2 verdure + 1/4 proteine (carne o pesce) + 1/4 cereali",
                k*pran, pr*pran, ca*pran, fa*pran,
                "Seguire le proporzioni del piatto sano");
            aggiungi_pasto(&piano, "Merenda",
                "Snack proteico: yogurt o frutta secca (20g)",
                k*sp2, pr*sp2, ca*sp2, fa*sp2, "");
            aggiungi_pasto(&piano, "Cena",
                "Proteina magra (150g) + verdure abbondanti + olio EVO",
                k*cen, pr*cen, ca*cen, fa*cen,
                "Leggero e vario");

        } else {
            return costruisci_piano(u);
        }
    }

    return piano;
}