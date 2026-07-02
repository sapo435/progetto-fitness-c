#include "gui_core.h"
#include "lista_dinamica.h"

/**
 * @file gui_main.c
 * @brief Entry point dell'applicazione: inizializza database e finestra,
 *        gestisce il loop principale e lo smistamento tra le schermate.
 */

static void aggiungi_riga(char *t, int sz, const char *r)
{ strncat(t, r, sz - strlen(t) - 1); }

/* Formatta programma in testo */
static void formato_programma(const ProgrammaAllenamento *p, char *t, int sz)
{
    char r[300];
    snprintf(r, sizeof(r), "== %s ==\n%s\n", p->nome, p->descrizione);
    aggiungi_riga(t, sz, r);
    for (int i = 0; i < p->num_sessioni; i++) {
        const Sessione *ss = &p->sessioni[i];
        aggiungi_riga(t, sz, "\n");
        snprintf(r, sizeof(r), ">> %s\n", ss->titolo);
        aggiungi_riga(t, sz, r);
        aggiungi_riga(t, sz,
            "   Esercizio              Muscolo      Serie Rip  Rec\n"
            "   --------------------------------------------------\n");
        for (int j = 0; j < ss->num_esercizi; j++) {
            const Esercizio *e = &ss->esercizi[j];
            snprintf(r, sizeof(r),
                     "   %-22s %-12s %2dx%-3d %3ds\n",
                     e->nome_esercizio, e->gruppo_muscolare,
                     e->serie, e->ripetizioni, e->riposo_sec);
            aggiungi_riga(t, sz, r);
            if (e->note[0]) {
                snprintf(r, sizeof(r), "      -> %s\n", e->note);
                aggiungi_riga(t, sz, r);
            }
        }
    }
    aggiungi_riga(t, sz, "\n");
}

/* Formatta piano in testo */
static void formato_piano(const PianoNutrizionale *p, char *t, int sz)
{
    char r[300];
    snprintf(r, sizeof(r), "== %s ==\n\n", p->etichetta);
    aggiungi_riga(t, sz, r);
    float tk = 0;
    for (int i = 0; i < p->num_pasti; i++) {
        const Pasto *m = &p->pasti[i];
        snprintf(r, sizeof(r), ">> %s\n", m->nome);
        aggiungi_riga(t, sz, r);
        aggiungi_riga(t, sz, "   -----------------------------------------\n");
        snprintf(r, sizeof(r), "   %s\n", m->alimenti);
        aggiungi_riga(t, sz, r);
        snprintf(r, sizeof(r),
                 "   %.0f kcal  |  P:%.0fg  C:%.0fg  G:%.0fg\n",
                 m->kcal, m->proteine_g, m->carboidrati_g, m->grassi_g);
        aggiungi_riga(t, sz, r);
        if (m->note[0]) {
            snprintf(r, sizeof(r), "   Nota: %s\n", m->note);
            aggiungi_riga(t, sz, r);
        }
        aggiungi_riga(t, sz, "\n");
        tk += m->kcal;
    }
    snprintf(r, sizeof(r), "TOTALE GIORNALIERO: %.0f kcal\n\n", tk);
    aggiungi_riga(t, sz, r);
}

int main(void)
{
    if (!app_db_init("palestra.db")) {
        fprintf(stderr, "Errore: impossibile aprire il database.\n");
        return 1;
    }
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1100, 700, "New Fitness");
    SetWindowState(FLAG_WINDOW_MAXIMIZED);
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    AppState stato;
    memset(&stato, 0, sizeof(stato));
    stato.schermata       = SCHERMATA_BENVENUTO;
    stato.ultima_caricata = SCHERMATA_BENVENUTO;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(COL_BG);
        if (stato.id_utente > 0) disegna_sidebar(&stato);

        switch (stato.schermata) {

        case SCHERMATA_BENVENUTO: schermata_benvenuto(&stato); break;
        case SCHERMATA_LOGIN:     schermata_login(&stato);     break;
        case SCHERMATA_REGISTRA:  schermata_registra(&stato);  break;
        case SCHERMATA_HOME:      schermata_home(&stato);      break;
        case SCHERMATA_REPORT:    schermata_report(&stato);    break;
        case SCHERMATA_FEEDBACK:  schermata_feedback(&stato);  break;


        case SCHERMATA_STORICO: {
            if (stato.ultima_caricata != SCHERMATA_STORICO) {
                memset(stato.testo, 0, sizeof(stato.testo));
                carica_storico_dinamico_in_buffer(stato.id_utente,
                                                   stato.testo, sizeof(stato.testo));
                stato.ultima_caricata = SCHERMATA_STORICO;
                stato.scroll_y = 0;
            }
            schermata_testo(&stato, "Storico pesi", "Andamento del peso nel tempo");
            break;
        }


        case SCHERMATA_SCHEDE: {
            if (stato.ultima_caricata != SCHERMATA_SCHEDE) {
                memset(stato.testo, 0, sizeof(stato.testo));
                char *t = stato.testo;
                int   sz = sizeof(stato.testo);

                if (stato.utente.servizio != SERVIZIO_SOLO_NUTRIZIONE) {
                    int var = app_get_variante_scheda(stato.id_utente);
                    ProgrammaAllenamento prog_show;
                    if (var >= 0) {
                        /* Usa la variante scelta dal trainer */
                        prog_show = costruisci_programma_ex(&stato.utente, var);
                    } else if (stato.programma_pronto) {
                        /* Nessuna assegnazione -> programma base */
                        prog_show = stato.programma;
                    } else {
                        goto schede_db;
                    }
                    formato_programma(&prog_show, t, sz);
                }
                schede_db:;
                /* Eventuali schede manuali aggiuntive dal trainer */
                char db_s[2048] = "";
                app_get_schede(stato.id_utente, db_s, sizeof(db_s));
                if (strlen(db_s) > 0) {
                    aggiungi_riga(t, sz, "== Schede aggiuntive dal trainer ==\n");
                    aggiungi_riga(t, sz, db_s);
                }
                stato.ultima_caricata = SCHERMATA_SCHEDE;
                stato.scroll_y = 0;
            }
            schermata_testo(&stato, "Schede allenamento", "Il tuo programma personalizzato");
            break;
        }


        case SCHERMATA_PIANI: {
            if (stato.ultima_caricata != SCHERMATA_PIANI) {
                memset(stato.testo, 0, sizeof(stato.testo));
                char *t = stato.testo;
                int   sz = sizeof(stato.testo);

                if (stato.utente.servizio != SERVIZIO_SOLO_ALLENAMENTO) {
                    int var = app_get_variante_piano(stato.id_utente);
                    PianoNutrizionale pn_show;
                    if (var >= 0) {
                        /* Usa la variante scelta dal trainer */
                        pn_show = costruisci_piano_ex(&stato.utente, var);
                    } else if (stato.piano_pronto) {
                        /* Nessuna assegnazione -> piano base */
                        pn_show = stato.piano;
                    } else {
                        goto piani_db;
                    }
                    formato_piano(&pn_show, t, sz);
                }
                piani_db:;
                char db_p[2048] = "";
                app_get_piani(stato.id_utente, db_p, sizeof(db_p));
                if (strlen(db_p) > 0) {
                    aggiungi_riga(t, sz, "== Piani aggiuntivi dal trainer ==\n");
                    aggiungi_riga(t, sz, db_p);
                }
                stato.ultima_caricata = SCHERMATA_PIANI;
                stato.scroll_y = 0;
            }
            schermata_testo(&stato, "Piani alimentari", "Il tuo piano nutrizionale");
            break;
        }

        case SCHERMATA_TRAINER: schermata_trainer(&stato); break;

        /* Lista clienti: array dinamico. */
        case SCHERMATA_TRAINER_LISTA: {
            memset(stato.testo, 0, sizeof(stato.testo));
            int n_clienti = 0;
            ClienteInfo *clienti = carica_lista_clienti_dinamica(&n_clienti);
            for (int i = 0; i < n_clienti; i++) {
                char riga[256];
                snprintf(riga, sizeof(riga),
                         "%-15s %-15s  %d anni  %c  %.1fkg  %.0fcm\n",
                         clienti[i].nome, clienti[i].cognome, clienti[i].eta,
                         clienti[i].sesso, clienti[i].peso_kg, clienti[i].altezza_cm);
                aggiungi_riga(stato.testo, sizeof(stato.testo), riga);
            }
            libera_lista_clienti_dinamica(clienti);
            stato.scroll_y = 0;
            schermata_testo(&stato, "Lista clienti",
                            "Usa il nome nelle schermate sottostanti per cercare");
            break;
        }

        case SCHERMATA_TRAINER_SCHEDE:  schermata_trainer_schede(&stato);  break;
        case SCHERMATA_TRAINER_STORICO: schermata_trainer_storico(&stato); break;
        case SCHERMATA_TRAINER_PIANI:   schermata_trainer_piani(&stato);   break;

        default: break;
        }

        disegna_messaggi(&stato);
        EndDrawing();
    }
    app_db_close();
    CloseWindow();
    return 0;
}