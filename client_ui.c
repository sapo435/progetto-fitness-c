/*
 * client_ui.c
 * -----------
 * Input, output e flussi interattivi (area cliente e trainer).
 * Ogni chiamata al server include il token di sessione.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "client_ui.h"
#include "client_logica.h"
#include "client_api.h"

/* ======================== INPUT ======================== */

void leggi_stringa(const char *etichetta, char *dest, int dim)
{
    printf("  %s: ", etichetta);
    fflush(stdout);
    fgets(dest, dim, stdin);
    dest[strcspn(dest, "\n")] = '\0';
}

float leggi_float(const char *etichetta)
{
    float val;
    printf("  %s: ", etichetta);
    scanf("%f", &val);  getchar();
    return val;
}

int leggi_int(const char *etichetta)
{
    int val;
    printf("  %s: ", etichetta);
    scanf("%d", &val);  getchar();
    return val;
}

/* ======================== OUTPUT ======================== */

void linea(char c, int n)
{
    for (int i = 0; i < n; i++) putchar(c);
    putchar('\n');
}

const char *stringa_obiettivo(Obiettivo ob)
{
    switch (ob) {
        case OBIETTIVO_DIMAGRIMENTO: return "Dimagrimento";
        case OBIETTIVO_MASSA:        return "Massa muscolare";
        case OBIETTIVO_MANTENIMENTO: return "Mantenimento";
        case OBIETTIVO_BENESSERE:    return "Benessere generale";
        default:                     return "-";
    }
}

void stampa_report(const Utente *u)
{
    float sett = stima_settimane(u);

    linea('=', 62);
    printf("  REPORT: %s %s\n", u->nome, u->cognome);
    linea('=', 62);
    printf("  Eta': %d | Sesso: %s | Altezza: %.0f cm\n",
           u->eta, u->sesso == 'M' ? "M" : "F", u->altezza_cm);
    printf("  Peso: %.1f kg -> Obiettivo: %.1f kg (%s)\n",
           u->peso_kg, u->peso_obiettivo_kg, stringa_obiettivo(u->obiettivo));
    linea('-', 62);
    printf("  BMI: %.1f (%s)\n", u->bmi, categoria_bmi(u->bmi));
    printf("  BMR: %.0f | TDEE: %.0f | Kcal obiettivo: %.0f\n",
           u->bmr, u->tdee_kcal, u->kcal_obiettivo);
    printf("  Proteine: %.0fg | Carboidrati: %.0fg | Grassi: %.0fg\n",
           u->proteine_g, u->carboidrati_g, u->grassi_g);
    printf("  Acqua: %.0f ml/giorno\n", fabbisogno_acqua(u->peso_kg, 1.0f));
    if (sett > 0) printf("  Tempo stimato: %.0f settimane\n", sett);
    linea('=', 62);
}

void stampa_programma(const ProgrammaAllenamento *p)
{
    linea('=', 62);
    printf("  PROGRAMMA: %s\n", p->nome);
    printf("  %s | Durata: %d settimane\n", p->descrizione, p->durata_settimane);
    linea('=', 62);

    for (int i = 0; i < p->num_sessioni; i++) {
        const Sessione *s = &p->sessioni[i];
        printf("\n  [ %s ]\n", s->titolo);
        linea('-', 58);
        for (int j = 0; j < s->num_esercizi; j++) {
            const Esercizio *e = &s->esercizi[j];
            printf("  %-22s %-12s %dx%-3d %ds",
                   e->nome_esercizio, e->gruppo_muscolare,
                   e->serie, e->ripetizioni, e->riposo_sec);
            if (e->note[0]) printf("  %s", e->note);
            putchar('\n');
        }
    }
    linea('=', 62);
}

void stampa_piano(const PianoNutrizionale *piano)
{
    float tk = 0, tp = 0, tc = 0, tg = 0;

    linea('=', 62);
    printf("  PIANO: %s\n", piano->etichetta);
    linea('=', 62);

    for (int i = 0; i < piano->num_pasti; i++) {
        const Pasto *m = &piano->pasti[i];
        printf("\n  [%s] %s\n", m->nome, m->alimenti);
        printf("  %.0f kcal | P:%.0fg C:%.0fg G:%.0fg\n",
               m->kcal, m->proteine_g, m->carboidrati_g, m->grassi_g);
        if (m->note[0]) printf("  -> %s\n", m->note);
        tk += m->kcal; tp += m->proteine_g;
        tc += m->carboidrati_g; tg += m->grassi_g;
    }
    linea('-', 62);
    printf("  TOTALE: %.0f kcal | P:%.0fg C:%.0fg G:%.0fg\n", tk, tp, tc, tg);
    linea('=', 62);
}

/* ======================== INSERIMENTO DATI ======================== */

void inserisci_dati(Utente *u)
{
    int sc;
    memset(u, 0, sizeof(Utente));

    linea('*', 62);
    printf("            NEW FITNESS — BENVENUTO!\n");
    linea('*', 62);

    leggi_stringa("Nome",    u->nome,    DIM_NOME);
    leggi_stringa("Cognome", u->cognome, DIM_NOME);

    printf("\n  Sesso: 1.Maschile 2.Femminile\n");
    u->sesso = (leggi_int("Scelta") == 1) ? 'M' : 'F';

    u->eta               = leggi_int("Eta' (anni)");
    u->altezza_cm        = leggi_float("Altezza (cm)");
    u->peso_kg           = leggi_float("Peso attuale (kg)");
    u->peso_obiettivo_kg = leggi_float("Peso obiettivo (kg)");

    printf("\n  Attivita': 1.Sedentario 2.Leggera 3.Moderata 4.Alta 5.Estrema\n");
    sc = leggi_int("Scelta");
    float livelli[] = {1.2f, 1.375f, 1.55f, 1.725f, 1.9f};
    u->livello_attivita = (sc >= 1 && sc <= 5) ? livelli[sc-1] : 1.375f;

    printf("\n  Servizio: 1.Solo allenamento 2.Solo nutrizione 3.Entrambi\n");
    sc = leggi_int("Scelta");
    u->servizio = (sc == 1) ? SERVIZIO_SOLO_ALLENAMENTO :
                  (sc == 2) ? SERVIZIO_SOLO_NUTRIZIONE : SERVIZIO_ENTRAMBI;
}

/* ======================== FEEDBACK SETTIMANALE ======================== */

void sezione_feedback(Utente *u, int id_server, const char *token)
{
    int sc;
    char data[16];
    time_t t = time(NULL);
    strftime(data, sizeof(data), "%Y-%m-%d", localtime(&t));

    linea('=', 62);
    printf("  FEEDBACK — Settimana %d\n", u->settimana_corrente);
    linea('=', 62);

    printf("\n  Aggiornare il peso? 1.Si 2.No\n");
    if (leggi_int("Scelta") == 1) {
        float np = leggi_float("Nuovo peso (kg)");
        registra_peso(u, np);
        salva_storico_server(token, id_server, np, data);
        printf("  -> Peso: %.1f kg | BMI: %.1f (%s)\n",
               u->peso_kg, u->bmi, categoria_bmi(u->bmi));

        float var = variazione_media_peso(u);
        if (u->num_pesi >= 2) {
            if (u->obiettivo == OBIETTIVO_DIMAGRIMENTO && var > -0.1f)
                printf("  [!] Peso stabile. Riduci kcal o aggiungi cardio.\n");
            else if (u->obiettivo == OBIETTIVO_DIMAGRIMENTO && var < -1.0f)
                printf("  [!] Troppo veloce. Aumenta kcal per preservare muscolo.\n");
            else if (u->obiettivo == OBIETTIVO_MASSA && var < 0.1f)
                printf("  [!] Peso non sale. Aumenta kcal di 150.\n");
            else
                printf("  [OK] Progressi in linea (%.2f kg/sett).\n", var);
        }
    }

    printf("\n  Allenamento: 1.Facile 2.Giusto 3.Difficile 4.Non fatto\n");
    sc = leggi_int("Scelta");
    if (sc == 1) {
        printf("  -> Aumenta intensita': +1 serie, -15s riposo.\n");
        if (u->livello_attivita < 1.725f) {
            u->livello_attivita += 0.175f;
            calcola_metriche(u);
        }
    } else if (sc == 2) {
        printf("  -> Perfetto, continua cosi'!\n");
        u->sessioni_completate += 3;
    } else if (sc == 3) {
        printf("  -> Riduci: -1 serie, +15s riposo. Tecnica prima del carico.\n");
    } else {
        printf("  Motivo: 1.Tempo 2.Stanchezza 3.Motivazione\n");
        sc = leggi_int("Scelta");
        if (sc == 1) printf("  -> Prova HIIT 20-30 min.\n");
        else if (sc == 2) printf("  -> Aggiungi un giorno di riposo.\n");
        else printf("  -> Fissa un orario fisso per allenarti.\n");
    }

    printf("\n  Dieta: 1.Seguita 2.Parziale 3.Restrittiva 4.Abbondante\n");
    sc = leggi_int("Scelta");
    if (sc == 3) {
        u->kcal_obiettivo += 100.0f;
        calcola_macronutrienti(u->peso_kg, u->kcal_obiettivo, u->obiettivo,
                               &u->proteine_g, &u->grassi_g, &u->carboidrati_g);
        printf("  -> Kcal +100 (ora %.0f).\n", u->kcal_obiettivo);
    } else if (sc == 4) {
        u->kcal_obiettivo -= 50.0f;
        calcola_macronutrienti(u->peso_kg, u->kcal_obiettivo, u->obiettivo,
                               &u->proteine_g, &u->grassi_g, &u->carboidrati_g);
        printf("  -> Kcal -50 (ora %.0f).\n", u->kcal_obiettivo);
    }

    if (fabsf(u->peso_kg - u->peso_obiettivo_kg) < 1.0f) {
        linea('*', 62);
        printf("  COMPLIMENTI! Peso obiettivo raggiunto!\n");
        printf("  1.Nuovo obiettivo 2.Mantenimento\n");
        if (leggi_int("Scelta") == 1) {
            u->peso_obiettivo_kg = leggi_float("Nuovo peso obiettivo (kg)");
            u->obiettivo = determina_obiettivo(u);
        } else {
            u->obiettivo = OBIETTIVO_MANTENIMENTO;
        }
        calcola_metriche(u);
        linea('*', 62);
    }

    u->settimana_corrente++;
    printf("\n  Settimana completata. Dati salvati.\n");
}

/* ======================== AREA TRAINER ======================== */

void area_trainer(int id_trainer, const char *token)
{
    int sc, id_cliente;

    linea('=', 62);
    printf("  AREA TRAINER (ID: %d)\n", id_trainer);
    linea('=', 62);

    do {
        printf("\n  1.Lista clienti 2.Schede 3.Storico 4.Piani\n"
               "  5.Nuova scheda 6.Nuovo piano 0.Esci\n");
        sc = leggi_int("Scelta");

        switch (sc) {
            case 1:
                mostra_lista_utenti(token);
                break;
            case 2:
                id_cliente = leggi_int("ID cliente");
                mostra_schede_server(token, id_cliente);
                break;
            case 3:
                id_cliente = leggi_int("ID cliente");
                mostra_storico_server(token, id_cliente);
                break;
            case 4:
                id_cliente = leggi_int("ID cliente");
                mostra_piani_server(token, id_cliente);
                break;
            case 5: {
                char titolo[DIM_NOME];
                id_cliente = leggi_int("ID cliente");
                leggi_stringa("Titolo scheda", titolo, DIM_NOME);
                int id = salva_scheda_server(token, titolo, id_cliente);
                printf(id > 0 ? "  -> Scheda creata (ID: %d).\n"
                               : "  -> Errore.\n", id);
                break;
            }
            case 6: {
                char note[DIM_TESTO]; float kcal;
                id_cliente = leggi_int("ID cliente");
                kcal = leggi_float("Kcal target");
                leggi_stringa("Note", note, DIM_TESTO);
                printf(salva_piano_server(token, id_cliente, kcal, note)
                       ? "  -> Piano salvato.\n" : "  -> Errore.\n");
                break;
            }
            case 0: break;
            default: printf("  Scelta non valida.\n");
        }
    } while (sc != 0);
}
