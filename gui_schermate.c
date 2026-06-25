/*
 * gui_schermate.c
 * ---------------
 * Implementazione di tutte le schermate dell'applicazione:
 *   - Benvenuto
 *   - Login
 *   - Registrazione (2 step)
 *   - Home cliente
 *   - Report (metriche + programma + piano)
 *   - Feedback settimanale
 *   - Area Trainer + schermate trainer
 */

#include "gui_core.h"


/* ================================================================
 *  VARIABILI DI STATO CONDIVISE TRA SCHERMATE
 *  (usate anche da gui_widgets.c per i toggle sesso/attivita'/servizio)
 * ================================================================ */

int reg_sesso     = 0;   /* 0 = Maschile, 1 = Femminile      */
int reg_attivita  = 1;   /* 0..4 -> livelli attivita'         */
int reg_servizio  = 2;   /* 0=allenamento, 1=nutrizione, 2=entrambi */

/* ================================================================
 *  SCHERMATA: BENVENUTO
 * ================================================================ */

void schermata_benvenuto(AppState *s)
{
    int cx = FINESTRA_W / 2;
    int cy = FINESTRA_H / 2;

    DrawText("NEW FITNESS",
             cx - MeasureText("NEW FITNESS", 52) / 2,
             cy - 130, 52, COL_ACCENT);
    DrawText("La tua palestra digitale",
             cx - MeasureText("La tua palestra digitale", 18) / 2,
             cy - 66, 18, COL_TEXT_DIM);

    DrawLine(cx - 160, cy - 36, cx + 160, cy - 36, COL_BORDER);

    int bx = cx - BTN_W / 2;
    if (bottone_accent(bx, cy - 14, BTN_W, BTN_H, "ACCEDI"))
        s->schermata = SCHERMATA_LOGIN;

    if (bottone(bx, cy + 50, BTN_W, BTN_H,
                "Crea un account", COL_SURFACE, COL_TEXT))
        s->schermata = SCHERMATA_REGISTRA;

    DrawText("v1.0  —  Progetto universitario",
             cx - MeasureText("v1.0  —  Progetto universitario", 12) / 2,
             FINESTRA_H - 30, 12, COL_TEXT_DIM);
}

/* ================================================================
 *  SCHERMATA: LOGIN
 * ================================================================ */

void schermata_login(AppState *s)
{
    static int inizializzato = 0;
    if (!inizializzato) {
        azzera_campi();
        aggiungi_campo("Username", 0);
        aggiungi_campo("Password", 1);
        inizializzato = 1;
    }

    disegna_header("Accedi", "Inserisci le tue credenziali");

    int cx = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
    int y0 = TITOLO_H + 80;

    disegna_campo(cx - INPUT_W / 2, y0,      INPUT_W, INPUT_H, 0);
    disegna_campo(cx - INPUT_W / 2, y0 + 80, INPUT_W, INPUT_H, 1);

    if (bottone_accent(cx - BTN_W / 2, y0 + 160, BTN_W, BTN_H, "ACCEDI") ||
        IsKeyPressed(KEY_ENTER))
    {
        char ruolo[DIM_NOME];
        char token[DIM_TOKEN];
        int  id = login_server(campi[0].testo, campi[1].testo,
                               ruolo, sizeof(ruolo),
                               token, sizeof(token));
        if (id < 0) {
            imposta_errore(s, "Username o password errati.");
        } else {
            s->id_server = id;
            strncpy(s->username, campi[0].testo, DIM_NOME - 1);
            strncpy(s->ruolo,    ruolo,           DIM_NOME - 1);
            strncpy(s->token,    token,           DIM_TOKEN - 1);
            inizializzato = 0;
            azzera_campi();
            s->schermata = (strcmp(ruolo, "trainer") == 0)
                           ? SCHERMATA_TRAINER : SCHERMATA_HOME;
        }
    }

    if (bottone(cx - BTN_W / 2, y0 + 224, BTN_W, 36,
                "Torna indietro", COL_SURFACE, COL_TEXT_DIM)) {
        inizializzato = 0;
        azzera_campi();
        s->schermata = SCHERMATA_BENVENUTO;
    }
}

/* ================================================================
 *  SCHERMATA: REGISTRAZIONE (2 step)
 * ================================================================ */

/* Helper: disegna i tre gruppi di toggle (sesso, attivita', servizio). */
static void disegna_toggle_dati(int tx, int *ty)
{
    /* Sesso */
    DrawText("Sesso", tx, *ty - 4, 13, COL_TEXT_DIM); *ty += 16;
    const char *sessi[] = {"Maschile", "Femminile"};
    for (int i = 0; i < 2; i++) {
        Color sf = (reg_sesso == i) ? COL_ACCENT_DIM : COL_SURFACE;
        Color ct = (reg_sesso == i) ? COL_ACCENT     : COL_TEXT;
        if (bottone(tx + i * 110, *ty, 100, 34, sessi[i], sf, ct))
            reg_sesso = i;
    }
    *ty += 54;

    /* Livello attivita' */
    DrawText("Livello attivita'", tx, *ty - 4, 13, COL_TEXT_DIM); *ty += 16;
    const char *att[] = {"Sedent.", "Leggera", "Moder.", "Alta", "Estrema"};
    for (int i = 0; i < 5; i++) {
        Color sf = (reg_attivita == i) ? COL_ACCENT_DIM : COL_SURFACE;
        Color ct = (reg_attivita == i) ? COL_ACCENT     : COL_TEXT;
        if (bottone(tx + i * 82, *ty, 76, 34, att[i], sf, ct))
            reg_attivita = i;
    }
    *ty += 54;

    /* Servizio */
    DrawText("Servizio desiderato", tx, *ty - 4, 13, COL_TEXT_DIM); *ty += 16;
    const char *serv[] = {"Allenamento", "Nutrizione", "Entrambi"};
    for (int i = 0; i < 3; i++) {
        Color sf = (reg_servizio == i) ? COL_ACCENT_DIM : COL_SURFACE;
        Color ct = (reg_servizio == i) ? COL_ACCENT     : COL_TEXT;
        if (bottone(tx + i * 140, *ty, 132, 34, serv[i], sf, ct))
            reg_servizio = i;
    }
    *ty += 70;
}

/* Compila l'Utente dai campi testo e dai toggle. */
static void compila_utente_da_form(Utente *u)
{
    float livelli[] = {1.2f, 1.375f, 1.55f, 1.725f, 1.9f};
    strncpy(u->nome,    campi[0].testo, DIM_NOME - 1);
    strncpy(u->cognome, campi[1].testo, DIM_NOME - 1);
    u->eta               = atoi(campi[2].testo);
    u->altezza_cm        = atof(campi[3].testo);
    u->peso_kg           = atof(campi[4].testo);
    u->peso_obiettivo_kg = atof(campi[5].testo);
    u->sesso             = (reg_sesso == 0) ? 'M' : 'F';
    u->livello_attivita  = livelli[reg_attivita];
    u->servizio          = (Servizio)reg_servizio;
    u->bmi               = calcola_bmi(u->peso_kg, u->altezza_cm);
    u->obiettivo         = determina_obiettivo(u);
    calcola_metriche(u);
}

void schermata_registra(AppState *s)
{
    static int  step = 0;          /* 0 = credenziali, 1 = dati fisici */
    static char pwd_salvata[DIM_NOME] = "";

    /* ---- STEP 0: credenziali ---- */
    if (step == 0) {
        static int init = 0;
        if (!init) {
            azzera_campi();
            aggiungi_campo("Username",          0);
            aggiungi_campo("Password",          1);
            aggiungi_campo("Conferma password", 1);
            init = 1;
        }

        disegna_header("Nuovo account", "Step 1 / 2  —  Credenziali");

        int cx = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
        int y0 = TITOLO_H + 70;

        disegna_campo(cx - INPUT_W / 2, y0,       INPUT_W, INPUT_H, 0);
        disegna_campo(cx - INPUT_W / 2, y0 + 80,  INPUT_W, INPUT_H, 1);
        disegna_campo(cx - INPUT_W / 2, y0 + 160, INPUT_W, INPUT_H, 2);

        if (bottone_accent(cx - BTN_W / 2, y0 + 248, BTN_W, BTN_H, "AVANTI ->")) {
            if (strlen(campi[0].testo) < 3) {
                imposta_errore(s, "Username troppo corto (min 3 caratteri).");
            } else if (strcmp(campi[1].testo, campi[2].testo) != 0) {
                imposta_errore(s, "Le password non coincidono.");
            } else if (strlen(campi[1].testo) < 4) {
                imposta_errore(s, "Password troppo corta (min 4 caratteri).");
            } else {
                strncpy(s->utente.username, campi[0].testo, DIM_NOME - 1);
                strncpy(pwd_salvata,        campi[1].testo, DIM_NOME - 1);
                step = 1;
                init = 0;
                azzera_campi();
                aggiungi_campo("Nome",                0);
                aggiungi_campo("Cognome",             0);
                aggiungi_campo("Eta' (anni)",         0);
                aggiungi_campo("Altezza (cm)",        0);
                aggiungi_campo("Peso attuale (kg)",   0);
                aggiungi_campo("Peso obiettivo (kg)", 0);
            }
        }

        if (bottone(cx - BTN_W / 2, y0 + 314, BTN_W, 36,
                    "Torna indietro", COL_SURFACE, COL_TEXT_DIM)) {
            step = 0; init = 0;
            azzera_campi();
            s->schermata = SCHERMATA_BENVENUTO;
        }
        return;
    }

    /* ---- STEP 1: dati fisici ---- */
    disegna_header("Nuovo account", "Step 2 / 2  —  Dati personali");

    int cx   = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
    int col1 = SIDEBAR_W + PADDING;
    int col2 = cx + PADDING;
    int y0   = TITOLO_H + 60;

    /* Colonna sinistra: campi testo */
    disegna_campo(col1, y0,        INPUT_W / 2 - 8, INPUT_H, 0); /* nome */
    disegna_campo(col1, y0 +  72,  INPUT_W / 2 - 8, INPUT_H, 1); /* cognome */
    disegna_campo(col1, y0 + 144,  INPUT_W / 2 - 8, INPUT_H, 2); /* eta */
    disegna_campo(col1, y0 + 216,  INPUT_W / 2 - 8, INPUT_H, 3); /* altezza */
    disegna_campo(col1, y0 + 288,  INPUT_W / 2 - 8, INPUT_H, 4); /* peso */
    disegna_campo(col1, y0 + 360,  INPUT_W / 2 - 8, INPUT_H, 5); /* peso obiettivo */

    /* Colonna destra: toggle */
    int tx = col2, ty = y0;
    disegna_toggle_dati(tx, &ty);

    /* Pulsanti azione */
    if (bottone_accent(tx, ty, 200, BTN_H, "REGISTRATI")) {
        if (strlen(campi[0].testo) < 1 || strlen(campi[1].testo) < 1) {
            imposta_errore(s, "Nome e cognome obbligatori.");
        } else {
            compila_utente_da_form(&s->utente);

            int id = registra_utente_server(&s->utente,
                                            s->utente.username,
                                            pwd_salvata);
            if (id < 0) {
                imposta_errore(s, "Errore registrazione. Username gia' usato?");
            } else {
                s->id_server = id;

                /* Login automatico */
                char ruolo[DIM_NOME], token[DIM_TOKEN];
                int lid = login_server(s->utente.username, pwd_salvata,
                                       ruolo, sizeof(ruolo),
                                       token, sizeof(token));
                if (lid > 0) {
                    s->id_server = lid;
                    strncpy(s->username, s->utente.username, DIM_NOME - 1);
                    strncpy(s->ruolo,    ruolo,               DIM_NOME - 1);
                    strncpy(s->token,    token,               DIM_TOKEN - 1);

                    /* Peso iniziale */
                    char data_oggi[16];
                    time_t t = time(NULL);
                    strftime(data_oggi, sizeof(data_oggi), "%Y-%m-%d", localtime(&t));
                    salva_storico_server(s->token, s->id_server,
                                        s->utente.peso_kg, data_oggi);

                    /* Programma e piano */
                    Utente *u = &s->utente;
                    if (u->servizio != SERVIZIO_SOLO_NUTRIZIONE) {
                        s->programma        = costruisci_programma(u);
                        s->programma_pronto = 1;
                    }
                    if (u->servizio != SERVIZIO_SOLO_ALLENAMENTO) {
                        s->piano        = costruisci_piano(u);
                        s->piano_pronto = 1;
                        salva_piano_server(s->token, s->id_server,
                                           u->kcal_obiettivo,
                                           u->obiettivo == OBIETTIVO_DIMAGRIMENTO ? "Dimagrimento" :
                                           u->obiettivo == OBIETTIVO_MASSA        ? "Massa muscolare" :
                                           u->obiettivo == OBIETTIVO_MANTENIMENTO ? "Mantenimento" :
                                                                                    "Benessere");
                    }

                    step = 0;
                    azzera_campi();
                    imposta_ok(s, "Account creato con successo!");
                    s->schermata = SCHERMATA_REPORT;
                } else {
                    imposta_ok(s, "Registrato! Fai login.");
                    step = 0;
                    azzera_campi();
                    s->schermata = SCHERMATA_LOGIN;
                }
            }
        }
    }

    if (bottone(tx + 216, ty, 120, BTN_H, "<- Indietro", COL_SURFACE, COL_TEXT_DIM)) {
        step = 0;
        azzera_campi();
        aggiungi_campo("Username",          0);
        aggiungi_campo("Password",          1);
        aggiungi_campo("Conferma password", 1);
    }
}

/* ================================================================
 *  SCHERMATA: HOME CLIENTE
 * ================================================================ */

void schermata_home(AppState *s)
{
    disegna_header("Home", s->username);

    int cx  = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
    int y0  = TITOLO_H + 50;
    int bw2 = (FINESTRA_W - SIDEBAR_W - PADDING * 3) / 2;

    /* Saluto */
    char saluto[128];
    snprintf(saluto, sizeof(saluto), "Benvenuto/a, %s!", s->username);
    DrawText(saluto, cx - MeasureText(saluto, 22) / 2, y0, 22, COL_TEXT);
    y0 += 60;

    /* Card metriche (visibili solo se i dati sono stati inseriti) */
    if (s->utente.peso_kg > 0) {
        char bmi_txt[32], peso_txt[32], kcal_txt[32], traguardo_txt[32];
        snprintf(bmi_txt,      sizeof(bmi_txt),      "BMI %.1f",          s->utente.bmi);
        snprintf(peso_txt,     sizeof(peso_txt),      "%.1f kg",           s->utente.peso_kg);
        snprintf(kcal_txt,     sizeof(kcal_txt),      "%.0f kcal/die",     s->utente.kcal_obiettivo);
        float sett = stima_settimane(&s->utente);
        if (sett > 0) snprintf(traguardo_txt, sizeof(traguardo_txt), "~%.0f settimane", sett);
        else          strncpy(traguardo_txt, "Obiettivo!", sizeof(traguardo_txt));

        const char *lab[] = {"BMI", "Peso", "Kcal obiettivo", "Traguardo"};
        const char *val[] = {bmi_txt, peso_txt, kcal_txt, traguardo_txt};
        int cw = (FINESTRA_W - SIDEBAR_W - PADDING * 5) / 4;

        for (int i = 0; i < 4; i++) {
            int cx2 = SIDEBAR_W + PADDING + i * (cw + PADDING);
            disegna_rettangolo_bordo(cx2, y0, cw, 80, RAGGIO, COL_CARD, COL_BORDER);
            DrawText(lab[i], cx2 + 12, y0 + 10, 12, COL_TEXT_DIM);
            DrawText(val[i], cx2 + 12, y0 + 34, 18, COL_ACCENT);
        }
        y0 += 110;
    }

    /* Bottoni navigazione rapida */
    int bx1 = SIDEBAR_W + PADDING;
    int bx2 = bx1 + bw2 + PADDING;

    if (bottone_accent(bx1, y0, bw2, BTN_H, "Report e programma"))
        s->schermata = SCHERMATA_REPORT;
    if (bottone(bx2, y0, bw2, BTN_H, "Feedback settimanale", COL_SURFACE, COL_TEXT))
        s->schermata = SCHERMATA_FEEDBACK;

    y0 += BTN_H + PADDING;

    if (bottone(bx1, y0, bw2, BTN_H, "Storico pesi", COL_SURFACE, COL_TEXT))
        { s->schermata = SCHERMATA_STORICO; s->scroll_y = 0; }
    if (bottone(bx2, y0, bw2, BTN_H, "Le mie schede", COL_SURFACE, COL_TEXT))
        { s->schermata = SCHERMATA_SCHEDE;  s->scroll_y = 0; }

    y0 += BTN_H + PADDING;

    if (bottone(bx1, y0, bw2, BTN_H, "Piani alimentari", COL_SURFACE, COL_TEXT))
        { s->schermata = SCHERMATA_PIANI; s->scroll_y = 0; }
}

/* ================================================================
 *  SCHERMATA: REPORT
 * ================================================================ */

void schermata_report(AppState *s)
{
    static int fase              = 0; /* 0=mostra, 1=form dati */
    static int form_inizializzato = 0;

    /* Nessun dato: mostra bottone per inserirli */
    if (s->utente.peso_kg < 1.0f && fase == 0) {
        disegna_header("Report", "Inserisci i tuoi dati per generare il report");
        int cx = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
        DrawText("Non hai ancora dati inseriti.",
                 cx - 160, FINESTRA_H / 2 - 40, 18, COL_TEXT_DIM);
        if (bottone_accent(cx - 120, FINESTRA_H / 2 + 10, 240, BTN_H, "Inserisci dati")) {
            fase               = 1;
            form_inizializzato = 0;
        }
        return;
    }

    /* Form raccolta dati */
    if (s->utente.peso_kg < 1.0f && fase == 1) {
        if (!form_inizializzato) {
            azzera_campi();
            aggiungi_campo("Nome",                0);
            aggiungi_campo("Cognome",             0);
            aggiungi_campo("Eta' (anni)",         0);
            aggiungi_campo("Altezza (cm)",        0);
            aggiungi_campo("Peso attuale (kg)",   0);
            aggiungi_campo("Peso obiettivo (kg)", 0);
            form_inizializzato = 1;
        }

        disegna_header("Report", "I tuoi dati");

        int cx   = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
        int col1 = SIDEBAR_W + PADDING;
        int col2 = cx + PADDING;
        int y0   = TITOLO_H + 60;

        disegna_campo(col1, y0,        INPUT_W/2 - 8, INPUT_H, 0);
        disegna_campo(col1, y0 +  72,  INPUT_W/2 - 8, INPUT_H, 1);
        disegna_campo(col1, y0 + 144,  INPUT_W/2 - 8, INPUT_H, 2);
        disegna_campo(col1, y0 + 216,  INPUT_W/2 - 8, INPUT_H, 3);
        disegna_campo(col1, y0 + 288,  INPUT_W/2 - 8, INPUT_H, 4);
        disegna_campo(col1, y0 + 360,  INPUT_W/2 - 8, INPUT_H, 5);

        int tx = col2, ty = y0;
        disegna_toggle_dati(tx, &ty);

        if (bottone_accent(col2, ty, 200, BTN_H, "GENERA REPORT")) {
            compila_utente_da_form(&s->utente);
            Utente *u = &s->utente;

            if (u->servizio != SERVIZIO_SOLO_NUTRIZIONE) {
                s->programma        = costruisci_programma(u);
                s->programma_pronto = 1;
            }
            if (u->servizio != SERVIZIO_SOLO_ALLENAMENTO) {
                s->piano        = costruisci_piano(u);
                s->piano_pronto = 1;
            }
            fase               = 0;
            form_inizializzato = 0;
            azzera_campi();
        }
        return;
    }

    /* ---- Visualizzazione report ---- */
    disegna_header("Report", "Le tue metriche e il tuo programma");

    Utente *u  = &s->utente;
    int x0     = SIDEBAR_W + PADDING;
    int y      = TITOLO_H + 16;
    int cw     = (FINESTRA_W - SIDEBAR_W - PADDING * 5) / 4;
    char buf[128];

    /* Riga 1: metriche principali */
    const char *met_lab[] = {"BMI", "BMR (kcal)", "TDEE (kcal)", "Kcal obiettivo"};
    char met_val[4][32];
    snprintf(met_val[0], 32, "%.1f (%s)", u->bmi,
             u->bmi < 18.5f ? "Sottopeso" : u->bmi < 25 ? "Normopeso" :
             u->bmi < 30    ? "Sovrappeso" : "Obesita'");
    snprintf(met_val[1], 32, "%.0f", u->bmr);
    snprintf(met_val[2], 32, "%.0f", u->tdee_kcal);
    snprintf(met_val[3], 32, "%.0f", u->kcal_obiettivo);

    for (int i = 0; i < 4; i++) {
        int cx2 = x0 + i * (cw + PADDING);
        disegna_rettangolo_bordo(cx2, y, cw, 70, RAGGIO, COL_CARD, COL_BORDER);
        DrawText(met_lab[i], cx2 + 10, y +  8, 11, COL_TEXT_DIM);
        DrawText(met_val[i], cx2 + 10, y + 28, 15, COL_ACCENT);
    }
    y += 84;

    /* Riga 2: macronutrienti */
    const char *mac_lab[] = {"Proteine", "Carboidrati", "Grassi", "Acqua"};
    char mac_val[4][24];
    snprintf(mac_val[0], 24, "%.0f g",  u->proteine_g);
    snprintf(mac_val[1], 24, "%.0f g",  u->carboidrati_g);
    snprintf(mac_val[2], 24, "%.0f g",  u->grassi_g);
    snprintf(mac_val[3], 24, "%.0f ml", fabbisogno_acqua(u->peso_kg, 1.0f));

    for (int i = 0; i < 4; i++) {
        int cx2 = x0 + i * (cw + PADDING);
        disegna_rettangolo_bordo(cx2, y, cw, 60, RAGGIO, COL_CARD, COL_BORDER);
        DrawText(mac_lab[i], cx2 + 10, y +  6, 11, COL_TEXT_DIM);
        DrawText(mac_val[i], cx2 + 10, y + 26, 17, COL_TEXT);
    }
    y += 76;

    /* Area scroll: programma + piano */
    s->scroll_y -= GetMouseWheelMove() * 24;
    if (s->scroll_y < 0) s->scroll_y = 0;

    int area_h = FINESTRA_H - y - 8;
    BeginScissorMode(SIDEBAR_W, y, FINESTRA_W - SIDEBAR_W, area_h);
    int ys = y - (int)s->scroll_y;

    if (s->programma_pronto) {
        ProgrammaAllenamento *p = &s->programma;
        DrawText(p->nome,        x0, ys, 18, COL_ACCENT);    ys += 28;
        DrawText(p->descrizione, x0, ys, 13, COL_TEXT_DIM);  ys += 22;

        for (int i = 0; i < p->num_sessioni; i++) {
            Sessione *sess = &p->sessioni[i];
            DrawRectangle(x0, ys, FINESTRA_W - SIDEBAR_W - PADDING*2, 28, COL_SURFACE);
            DrawText(sess->titolo, x0 + 10, ys + 6, 14, COL_TEXT);
            ys += 32;

            for (int j = 0; j < sess->num_esercizi; j++) {
                Esercizio *e = &sess->esercizi[j];
                snprintf(buf, sizeof(buf), "  %-22s %-12s %dx%-3d %ds",
                         e->nome_esercizio, e->gruppo_muscolare,
                         e->serie, e->ripetizioni, e->riposo_sec);
                DrawText(buf, x0, ys, 13, COL_TEXT); ys += 18;
                if (e->note[0]) {
                    snprintf(buf, sizeof(buf), "    -> %s", e->note);
                    DrawText(buf, x0, ys, 12, COL_TEXT_DIM); ys += 16;
                }
            }
            ys += 10;
        }
        ys += 16;
    }

    if (s->piano_pronto) {
        PianoNutrizionale *p = &s->piano;
        DrawText(p->etichetta, x0, ys, 18, COL_ACCENT); ys += 28;

        float tk = 0;
        for (int i = 0; i < p->num_pasti; i++) {
            Pasto *m = &p->pasti[i];
            DrawRectangle(x0, ys, FINESTRA_W - SIDEBAR_W - PADDING*2, 24, COL_SURFACE);
            DrawText(m->nome, x0 + 10, ys + 4, 14, COL_TEXT); ys += 28;
            snprintf(buf, sizeof(buf), "  %s", m->alimenti);
            DrawText(buf, x0, ys, 12, COL_TEXT_DIM); ys += 16;
            snprintf(buf, sizeof(buf), "  %.0f kcal | P:%.0fg  C:%.0fg  G:%.0fg",
                     m->kcal, m->proteine_g, m->carboidrati_g, m->grassi_g);
            DrawText(buf, x0, ys, 13, COL_ACCENT); ys += 18;
            if (m->note[0]) {
                snprintf(buf, sizeof(buf), "  %s", m->note);
                DrawText(buf, x0, ys, 12, COL_TEXT_DIM); ys += 16;
            }
            tk += m->kcal;
            ys += 6;
        }
        snprintf(buf, sizeof(buf), "TOTALE: %.0f kcal", tk);
        DrawText(buf, x0, ys, 15, COL_TEXT);
    }

    EndScissorMode();
    DrawText("Scorri con la rotella del mouse",
             FINESTRA_W - 260, FINESTRA_H - 22, 12, COL_TEXT_DIM);
}

/* ================================================================
 *  SCHERMATA: FEEDBACK SETTIMANALE
 * ================================================================ */

void schermata_feedback(AppState *s)
{
    static int  allenamento_sc   = -1;
    static int  dieta_sc         = -1;
    static int  motivo_sc        = -1;
    static int  aggiorna_peso    = -1;
    static char nuovo_peso_txt[32] = "";
    static int  cp_peso_attivo   = 0;
    static char cp_peso_testo[32] = "";

    /* Se mancano i dati base, raccoglili con un form semplificato */
    static int fase_dati   = 0;
    static int form_fb_ini = 0;

    if (fase_dati == 0 && s->utente.peso_kg < 1.0f) {
        if (!form_fb_ini) {
            azzera_campi();
            aggiungi_campo("Nome",                0);
            aggiungi_campo("Cognome",             0);
            aggiungi_campo("Eta' (anni)",         0);
            aggiungi_campo("Altezza (cm)",        0);
            aggiungi_campo("Peso attuale (kg)",   0);
            aggiungi_campo("Peso obiettivo (kg)", 0);
            form_fb_ini = 1;
        }
        disegna_header("Feedback", "Prima inserisci i tuoi dati");
        int cx   = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
        int col1 = SIDEBAR_W + PADDING;
        int y0   = TITOLO_H + 60;
        for (int i = 0; i < 6; i++)
            disegna_campo(col1, y0 + i * 68, INPUT_W/2 - 8, INPUT_H, i);
        if (bottone_accent(cx - 100, y0 + 6*68 + 10, 200, BTN_H, "CONFERMA")) {
            Utente *u = &s->utente;
            strncpy(u->nome,    campi[0].testo, DIM_NOME - 1);
            strncpy(u->cognome, campi[1].testo, DIM_NOME - 1);
            u->eta               = atoi(campi[2].testo);
            u->altezza_cm        = atof(campi[3].testo);
            u->peso_kg           = atof(campi[4].testo);
            u->peso_obiettivo_kg = atof(campi[5].testo);
            u->sesso             = 'M';
            u->livello_attivita  = 1.375f;
            u->bmi               = calcola_bmi(u->peso_kg, u->altezza_cm);
            u->obiettivo         = determina_obiettivo(u);
            calcola_metriche(u);
            u->settimana_corrente = 1;
            u->num_pesi           = 0;
            registra_peso(u, u->peso_kg);
            fase_dati   = 1;
            form_fb_ini = 0;
            azzera_campi();
        }
        return;
    }

    disegna_header("Feedback", "Settimana corrente");

    Utente *u  = &s->utente;
    int x0     = SIDEBAR_W + PADDING;
    int cx     = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
    int y      = TITOLO_H + 20;

    /* --- Peso --- */
    DrawText("Aggiornare il peso?", x0, y, 15, COL_TEXT_DIM); y += 28;
    const char *sp[] = {"Si", "No"};
    for (int i = 0; i < 2; i++) {
        Color sf = (aggiorna_peso == i) ? COL_ACCENT_DIM : COL_SURFACE;
        Color ct = (aggiorna_peso == i) ? COL_ACCENT     : COL_TEXT;
        if (bottone(x0 + i * 96, y, 88, 34, sp[i], sf, ct))
            aggiorna_peso = i;
    }
    y += 48;

    if (aggiorna_peso == 0) {
        DrawText("Nuovo peso (kg)", x0, y, 13, COL_TEXT_DIM); y += 20;
        disegna_rettangolo_bordo(x0, y, 200, INPUT_H, RAGGIO, COL_INPUT_BG,
                                 cp_peso_attivo ? COL_ACCENT : COL_BORDER);
        DrawText(cp_peso_testo, x0 + 12, y + (INPUT_H-18)/2, 17, COL_TEXT);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = GetMousePosition();
            cp_peso_attivo = CheckCollisionPointRec(m, (Rectangle){x0, y, 200, INPUT_H});
            campo_attivo_idx = -1;
        }
        if (cp_peso_attivo) {
            int k = GetCharPressed();
            while (k > 0) {
                int n = strlen(cp_peso_testo);
                if ((k >= '0' && k <= '9') || k == '.') {
                    if (n < 30) { cp_peso_testo[n] = k; cp_peso_testo[n+1] = '\0'; }
                }
                k = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                int n = strlen(cp_peso_testo);
                if (n > 0) cp_peso_testo[n-1] = '\0';
            }
        }
        strncpy(nuovo_peso_txt, cp_peso_testo, 31);
        y += INPUT_H + 14;
    }

    /* --- Allenamento --- */
    DrawLine(x0, y, FINESTRA_W - PADDING, y, COL_BORDER); y += 16;
    DrawText("Come e' stato l'allenamento?", x0, y, 15, COL_TEXT_DIM); y += 28;
    const char *all_lbl[] = {"Facile", "Giusto", "Difficile", "Non fatto"};
    for (int i = 0; i < 4; i++) {
        Color sf = (allenamento_sc == i) ? COL_ACCENT_DIM : COL_SURFACE;
        Color ct = (allenamento_sc == i) ? COL_ACCENT     : COL_TEXT;
        if (bottone(x0 + i * 140, y, 132, 34, all_lbl[i], sf, ct))
            allenamento_sc = i;
    }
    y += 48;

    if (allenamento_sc == 3) {
        DrawText("Motivo:", x0, y, 13, COL_TEXT_DIM); y += 20;
        const char *mot[] = {"Tempo", "Stanchezza", "Motivazione"};
        for (int i = 0; i < 3; i++) {
            Color sf = (motivo_sc == i) ? COL_ACCENT_DIM : COL_SURFACE;
            Color ct = (motivo_sc == i) ? COL_ACCENT     : COL_TEXT;
            if (bottone(x0 + i * 144, y, 136, 34, mot[i], sf, ct))
                motivo_sc = i;
        }
        y += 48;
    }

    /* --- Dieta --- */
    DrawLine(x0, y, FINESTRA_W - PADDING, y, COL_BORDER); y += 16;
    DrawText("Come hai seguito la dieta?", x0, y, 15, COL_TEXT_DIM); y += 28;
    const char *diet_lbl[] = {"Seguita", "Parziale", "Restrittiva", "Abbondante"};
    for (int i = 0; i < 4; i++) {
        Color sf = (dieta_sc == i) ? COL_ACCENT_DIM : COL_SURFACE;
        Color ct = (dieta_sc == i) ? COL_ACCENT     : COL_TEXT;
        if (bottone(x0 + i * 140, y, 132, 34, diet_lbl[i], sf, ct))
            dieta_sc = i;
    }
    y += 60;

    /* --- Salva --- */
    if (bottone_accent(cx - 130, y, 260, BTN_H, "SALVA FEEDBACK")) {
        if (aggiorna_peso == 0 && strlen(nuovo_peso_txt) > 0) {
            float np = atof(nuovo_peso_txt);
            if (np > 20.0f) {
                registra_peso(u, np);
                char data[16];
                time_t t = time(NULL);
                strftime(data, sizeof(data), "%Y-%m-%d", localtime(&t));
                salva_storico_server(s->token, s->id_server, np, data);
            }
        }

        if (dieta_sc == 2) {
            u->kcal_obiettivo += 100.0f;
            calcola_macronutrienti(u->peso_kg, u->kcal_obiettivo, u->obiettivo,
                                   &u->proteine_g, &u->grassi_g, &u->carboidrati_g);
        } else if (dieta_sc == 3) {
            u->kcal_obiettivo -= 50.0f;
            calcola_macronutrienti(u->peso_kg, u->kcal_obiettivo, u->obiettivo,
                                   &u->proteine_g, &u->grassi_g, &u->carboidrati_g);
        }

        if (allenamento_sc == 0 && u->livello_attivita < 1.725f) {
            u->livello_attivita += 0.175f;
            calcola_metriche(u);
        }
        u->settimana_corrente++;
        if (allenamento_sc == 1) u->sessioni_completate += 3;

        /* Reset stato */
        allenamento_sc = -1; dieta_sc = -1; motivo_sc = -1;
        aggiorna_peso  = -1;
        memset(nuovo_peso_txt,  0, sizeof(nuovo_peso_txt));
        memset(cp_peso_testo,   0, sizeof(cp_peso_testo));
        cp_peso_attivo = 0;

        imposta_ok(s, "Feedback salvato! Continua cosi'.");
        s->schermata = SCHERMATA_HOME;
    }
}

/* ================================================================
 *  SCHERMATA: AREA TRAINER
 * ================================================================ */

void schermata_trainer(AppState *s)
{
    disegna_header("Area Trainer", s->username);

    int x0 = SIDEBAR_W + PADDING;
    int y  = TITOLO_H + 40;

    DrawText("Seleziona un'operazione dalla sidebar oppure usa i form qui sotto.",
             x0, y, 15, COL_TEXT_DIM);
    y += 50;

    /* Campo ID cliente */
    static char id_str[16]   = "";
    static int  id_attivo    = 0;
    int id_cliente;

    DrawText("ID cliente", x0, y - 4, 13, COL_TEXT_DIM); y += 16;
    disegna_rettangolo_bordo(x0, y, 200, INPUT_H, RAGGIO, COL_INPUT_BG,
                             id_attivo ? COL_ACCENT : COL_BORDER);
    DrawText(id_str, x0 + 12, y + (INPUT_H-18)/2, 17, COL_TEXT);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        id_attivo = CheckCollisionPointRec(m, (Rectangle){x0, y, 200, INPUT_H});
        campo_attivo_idx = -1;
    }
    if (id_attivo) {
        int k = GetCharPressed();
        while (k > 0) {
            int n = strlen(id_str);
            if (k >= '0' && k <= '9' && n < 14) { id_str[n] = k; id_str[n+1] = '\0'; }
            k = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) { int n = strlen(id_str); if (n > 0) id_str[n-1] = '\0'; }
    }
    id_cliente = atoi(id_str);
    y += INPUT_H + 24;

    /* --- Nuova scheda --- */
    DrawLine(x0, y, FINESTRA_W - PADDING, y, COL_BORDER); y += 16;
    DrawText("Nuova scheda allenamento", x0, y, 15, COL_TEXT_DIM); y += 28;

    static char titolo_scheda[DIM_NOME] = "";
    static int  titolo_attivo           = 0;

    DrawText("Titolo scheda", x0, y - 18, 12, COL_TEXT_DIM);
    disegna_rettangolo_bordo(x0, y, 400, INPUT_H, RAGGIO, COL_INPUT_BG,
                             titolo_attivo ? COL_ACCENT : COL_BORDER);
    DrawText(titolo_scheda, x0 + 12, y + (INPUT_H-18)/2, 17, COL_TEXT);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        titolo_attivo = CheckCollisionPointRec(m, (Rectangle){x0, y, 400, INPUT_H});
        campo_attivo_idx = -1;
    }
    if (titolo_attivo) {
        int k = GetCharPressed();
        while (k > 0) {
            int n = strlen(titolo_scheda);
            if (k >= 32 && n < DIM_NOME-1) { titolo_scheda[n] = k; titolo_scheda[n+1] = '\0'; }
            k = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) { int n = strlen(titolo_scheda); if (n > 0) titolo_scheda[n-1] = '\0'; }
    }
    y += INPUT_H + 10;

    if (bottone_accent(x0, y, 200, 38, "Crea scheda")) {
        if (id_cliente > 0 && strlen(titolo_scheda) > 0) {
            int id = salva_scheda_server(s->token, titolo_scheda, id_cliente);
            if (id > 0) imposta_ok(s, "Scheda creata con successo!");
            else        imposta_errore(s, "Errore creazione scheda.");
        } else imposta_errore(s, "Inserisci ID cliente e titolo.");
    }

    y += 60;

    /* --- Nuovo piano alimentare --- */
    DrawLine(x0, y, FINESTRA_W - PADDING, y, COL_BORDER); y += 16;
    DrawText("Nuovo piano alimentare", x0, y, 15, COL_TEXT_DIM); y += 28;

    static char kcal_str[16]       = "";
    static char note_piano[DIM_TESTO] = "";
    static int  kcal_attivo        = 0;
    static int  note_attivo        = 0;

    DrawText("Kcal target", x0, y - 18, 12, COL_TEXT_DIM);
    disegna_rettangolo_bordo(x0, y, 160, INPUT_H, RAGGIO, COL_INPUT_BG,
                             kcal_attivo ? COL_ACCENT : COL_BORDER);
    DrawText(kcal_str, x0 + 12, y + (INPUT_H-18)/2, 17, COL_TEXT);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        kcal_attivo = CheckCollisionPointRec(m, (Rectangle){x0, y, 160, INPUT_H});
        campo_attivo_idx = -1;
    }
    if (kcal_attivo) {
        int k = GetCharPressed();
        while (k > 0) {
            int n = strlen(kcal_str);
            if ((k >= '0' && k <= '9') || k == '.') { if (n < 14) { kcal_str[n] = k; kcal_str[n+1] = '\0'; } }
            k = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) { int n = strlen(kcal_str); if (n > 0) kcal_str[n-1] = '\0'; }
    }

    DrawText("Note dieta", x0 + 176, y - 18, 12, COL_TEXT_DIM);
    disegna_rettangolo_bordo(x0 + 176, y, 320, INPUT_H, RAGGIO, COL_INPUT_BG,
                             note_attivo ? COL_ACCENT : COL_BORDER);
    DrawText(note_piano, x0 + 176 + 12, y + (INPUT_H-18)/2, 17, COL_TEXT);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        note_attivo = CheckCollisionPointRec(m, (Rectangle){x0+176, y, 320, INPUT_H});
        campo_attivo_idx = -1;
    }
    if (note_attivo) {
        int k = GetCharPressed();
        while (k > 0) {
            int n = strlen(note_piano);
            if (k >= 32 && n < DIM_TESTO-1) { note_piano[n] = k; note_piano[n+1] = '\0'; }
            k = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) { int n = strlen(note_piano); if (n > 0) note_piano[n-1] = '\0'; }
    }

    y += INPUT_H + 10;

    if (bottone_accent(x0, y, 200, 38, "Salva piano")) {
        float kc = atof(kcal_str);
        if (id_cliente > 0 && kc > 0) {
            if (salva_piano_server(s->token, id_cliente, kc, note_piano))
                imposta_ok(s, "Piano salvato!");
            else
                imposta_errore(s, "Errore salvataggio piano.");
        } else imposta_errore(s, "Inserisci ID cliente e kcal.");
    }
}

/* ================================================================
 *  SCHERMATA: LISTA CLIENTI (trainer)
 * ================================================================ */

void schermata_trainer_lista(AppState *s)
{
    static int caricato = 0;
    if (!caricato) {
        mostra_lista_utenti_buf(s->token,
                                s->testo_server, sizeof(s->testo_server));
        caricato = 1;
    }
    schermata_testo_server(s, "Lista clienti", "Tutti gli utenti registrati");
    if (s->schermata != SCHERMATA_TRAINER_LISTA) caricato = 0;
}

/* ================================================================
 *  SCHERMATA: SCHEDE DI UN CLIENTE (trainer)
 * ================================================================ */

void schermata_trainer_schede(AppState *s)
{
    static int  caricato   = 0;
    static char id_str[16] = "";
    static int  id_attivo  = 0;

    int x0 = SIDEBAR_W + PADDING;
    disegna_header("Schede trainer", "Schede di un cliente specifico");

    int y = TITOLO_H + 20;
    DrawText("ID cliente", x0, y, 13, COL_TEXT_DIM); y += 20;

    disegna_rettangolo_bordo(x0, y, 160, INPUT_H, RAGGIO, COL_INPUT_BG,
                             id_attivo ? COL_ACCENT : COL_BORDER);
    DrawText(id_str, x0 + 12, y + (INPUT_H-18)/2, 17, COL_TEXT);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        id_attivo = CheckCollisionPointRec(m, (Rectangle){x0, y, 160, INPUT_H});
        campo_attivo_idx = -1;
    }
    if (id_attivo) {
        int k = GetCharPressed();
        while (k > 0) {
            int n = strlen(id_str);
            if (k >= '0' && k <= '9' && n < 14) { id_str[n] = k; id_str[n+1] = '\0'; }
            k = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) { int n = strlen(id_str); if (n > 0) id_str[n-1] = '\0'; }
    }
    y += INPUT_H + 10;

    if (bottone_accent(x0, y, 160, 38, "Carica")) {
        int id = atoi(id_str);
        mostra_schede_buf(s->token, id,
                          s->testo_server, sizeof(s->testo_server));
        caricato     = 1;
        s->scroll_y  = 0;
    }
    y += 54;

    if (caricato) schermata_testo_server_inline(s, y);
    if (s->schermata != SCHERMATA_TRAINER_SCHEDE) caricato = 0;
}
