/**
 * @file gui_schermate_auth.c
 * @brief Schermate di autenticazione: benvenuto, login, registrazione.
 */
#include "gui_core.h"
#include "lista_dinamica.h"

int reg_sesso    = 0;
int reg_attivita = 1;
int reg_servizio = 2;
int reg_ruolo    = 0;   /* 0 = cliente, 1 = trainer */

/* BENVENUTO */
void schermata_benvenuto(AppState *s)
{
    int cx = FINESTRA_W / 2;
    int cy = FINESTRA_H / 2;

    DrawText("NEW FITNESS",
             cx - MeasureText("NEW FITNESS", SC(52)) / 2,
             cy - SC(130), SC(52), COL_ACCENT);

    DrawText("La tua palestra digitale",
             cx - MeasureText("La tua palestra digitale", SC(18)) / 2,
             cy - SC(66), SC(18), COL_TEXT_DIM);

    DrawLine(cx - SC(160), cy - SC(36), cx + SC(160), cy - SC(36), COL_BORDER);

    int bx = cx - BTN_W / 2;

    if (bottone_accent(bx, cy - SC(14), BTN_W, BTN_H, "ACCEDI"))
        s->schermata = SCHERMATA_LOGIN;

    if (bottone(bx, cy + SC(50), BTN_W, BTN_H,
                "Crea un account", COL_SURFACE, COL_TEXT))
        s->schermata = SCHERMATA_REGISTRA;
}

/* LOGIN */
void schermata_login(AppState *s)
{
    static int init = 0;

    if (!init) {
        azzera_campi();
        aggiungi_campo("Username", 0);
        aggiungi_campo("Password", 1);
        init = 1;
    }

    disegna_header("Accedi", "Inserisci le tue credenziali");

    int cx = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
    int y0 = TITOLO_H + SC(80);

    disegna_campo(cx - INPUT_W / 2, y0, INPUT_W, INPUT_H, 0);
    disegna_campo(cx - INPUT_W / 2, y0 + SC(80), INPUT_W, INPUT_H, 1);

    if (bottone_accent(cx - BTN_W / 2, y0 + SC(160), BTN_W, BTN_H, "ACCEDI")
        || IsKeyPressed(KEY_ENTER)) {

        char ruolo[DIM_NOME] = "";
        int id = app_login(campi[0].testo, campi[1].testo,
                           ruolo, sizeof(ruolo));

        if (id < 0) {
            imposta_errore(s, "Username o password errati.");
        } else {
            s->id_utente = id;
            strncpy(s->username, campi[0].testo, DIM_NOME - 1);
            strncpy(s->ruolo, ruolo, DIM_NOME - 1);


            app_get_utente(id, &s->utente);
            strncpy(s->utente.username, campi[0].testo, DIM_NOME - 1);


            if (s->utente.bmr > 0 && s->utente.tdee_kcal > 0)
                s->utente.livello_attivita = s->utente.tdee_kcal / s->utente.bmr;
            else
                s->utente.livello_attivita = 1.375f;

            s->utente.bmi = calcola_bmi(s->utente.peso_kg, s->utente.altezza_cm);
            s->utente.obiettivo = determina_obiettivo(&s->utente);
            calcola_metriche(&s->utente);


            int var_s = app_get_variante_scheda(s->id_utente);
            if (var_s >= 0)
                s->programma = costruisci_programma_ex(&s->utente, var_s);
            else
                s->programma = costruisci_programma(&s->utente);
            s->programma_pronto = 1;

            int var_p = app_get_variante_piano(s->id_utente);
            if (var_p >= 0)
                s->piano = costruisci_piano_ex(&s->utente, var_p);
            else
                s->piano = costruisci_piano(&s->utente);
            s->piano_pronto = 1;

            init = 0;
            azzera_campi();
            s->schermata = (strcmp(ruolo, "trainer") == 0)
                           ? SCHERMATA_TRAINER : SCHERMATA_HOME;
        }
    }

    if (bottone(cx - BTN_W / 2, y0 + SC(224), BTN_W, SC(36),
                "Torna indietro", COL_SURFACE, COL_TEXT_DIM)) {
        init = 0;
        azzera_campi();
        s->schermata = SCHERMATA_BENVENUTO;
    }
}

/* REGISTRAZIONE */
static void disegna_toggle(int tx, int *ty)
{
    DrawText("Sesso", tx, *ty - SC(4), SC(13), COL_TEXT_DIM);
    *ty += SC(16);

    const char *sessi[] = {"Maschile", "Femminile"};
    for (int i = 0; i < 2; i++) {
        Color sf = (reg_sesso == i) ? COL_ACCENT_DIM : COL_SURFACE;
        Color ct = (reg_sesso == i) ? COL_ACCENT : COL_TEXT;
        if (bottone(tx + i * SC(110), *ty, SC(100), SC(34), sessi[i], sf, ct))
            reg_sesso = i;
    }
    *ty += SC(54);

    DrawText("Livello attivita'", tx, *ty - SC(4), SC(13), COL_TEXT_DIM);
    *ty += SC(16);

    const char *att[] = {"Sedent.", "Leggera", "Moder.", "Alta", "Estrema"};
    for (int i = 0; i < 5; i++) {
        Color sf = (reg_attivita == i) ? COL_ACCENT_DIM : COL_SURFACE;
        Color ct = (reg_attivita == i) ? COL_ACCENT : COL_TEXT;
        if (bottone(tx + i * SC(82), *ty, SC(76), SC(34), att[i], sf, ct))
            reg_attivita = i;
    }
    *ty += SC(54);

    DrawText("Servizio", tx, *ty - SC(4), SC(13), COL_TEXT_DIM);
    *ty += SC(16);

    const char *serv[] = {"Allenamento", "Nutrizione", "Entrambi"};
    for (int i = 0; i < 3; i++) {
        Color sf = (reg_servizio == i) ? COL_ACCENT_DIM : COL_SURFACE;
        Color ct = (reg_servizio == i) ? COL_ACCENT : COL_TEXT;
        if (bottone(tx + i * SC(140), *ty, SC(132), SC(34), serv[i], sf, ct))
            reg_servizio = i;
    }
    *ty += SC(70);
}

static void compila_utente(Utente *u)
{
    float livelli[] = {1.2f, 1.375f, 1.55f, 1.725f, 1.9f};

    strncpy(u->nome, campi[0].testo, DIM_NOME - 1);
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

/* Il trainer non ha dati fisici da registrare: bastano nome e cognome
   per identificarlo. Gli altri campi di Utente restano a zero, dato
   che non vengono mai letti per un account con ruolo "trainer".
   L'username, gia' impostato allo step 1, va preservato: il memset
   lo azzererebbe altrimenti, lasciando le credenziali senza username. */
static void compila_trainer(Utente *u)
{
    char username_tmp[DIM_NOME];
    strncpy(username_tmp, u->username, DIM_NOME - 1);
    username_tmp[DIM_NOME - 1] = '\0';

    memset(u, 0, sizeof(*u));

    strncpy(u->username, username_tmp, DIM_NOME - 1);
    strncpy(u->nome, campi[0].testo, DIM_NOME - 1);
    strncpy(u->cognome, campi[1].testo, DIM_NOME - 1);
}

void schermata_registra(AppState *s)
{
    static int step = 0;
    static char pwd[DIM_NOME] = "";

    if (step == 0) {
        static int init = 0;
        if (!init) {
            azzera_campi();
            aggiungi_campo("Username", 0);
            aggiungi_campo("Password", 1);
            aggiungi_campo("Conferma password", 1);
            init = 1;
        }

        disegna_header("Nuovo account", "Step 1/2 - Credenziali");

        int cx = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
        int y0 = TITOLO_H + SC(50);

        /* Scelta del tipo di account */
        DrawText("Tipo di account", cx - INPUT_W / 2, y0 - SC(20), SC(13), COL_TEXT_DIM);
        const char *ruoli[] = {"Cliente", "Trainer"};
        for (int i = 0; i < 2; i++) {
            Color sf = (reg_ruolo == i) ? COL_ACCENT_DIM : COL_SURFACE;
            Color ct = (reg_ruolo == i) ? COL_ACCENT : COL_TEXT;
            if (bottone(cx - INPUT_W / 2 + i * SC(190), y0, SC(180), SC(38), ruoli[i], sf, ct))
                reg_ruolo = i;
        }
        y0 += SC(56);

        disegna_campo(cx - INPUT_W / 2, y0, INPUT_W, INPUT_H, 0);
        disegna_campo(cx - INPUT_W / 2, y0 + SC(80), INPUT_W, INPUT_H, 1);
        disegna_campo(cx - INPUT_W / 2, y0 + SC(160), INPUT_W, INPUT_H, 2);

        if (bottone_accent(cx - BTN_W / 2, y0 + SC(248), BTN_W, BTN_H,
                           "AVANTI ->")) {
            if (strlen(campi[0].testo) < 3) {
                imposta_errore(s, "Username troppo corto.");
            } else if (strcmp(campi[1].testo, campi[2].testo) != 0) {
                imposta_errore(s, "Password non coincidono.");
            } else if (strlen(campi[1].testo) < 4) {
                imposta_errore(s, "Password troppo corta.");
            } else {
                strncpy(s->utente.username, campi[0].testo, DIM_NOME - 1);
                strncpy(pwd, campi[1].testo, DIM_NOME - 1);
                step = 1;
                init = 0;
                azzera_campi();
                aggiungi_campo("Nome", 0);
                aggiungi_campo("Cognome", 0);
                if (reg_ruolo == 0) {
                    /* Solo il cliente ha bisogno di dati fisici */
                    aggiungi_campo("Eta' (anni)", 0);
                    aggiungi_campo("Altezza (cm)", 0);
                    aggiungi_campo("Peso attuale (kg)", 0);
                    aggiungi_campo("Peso obiettivo (kg)", 0);
                }
            }
        }

        if (bottone(cx - BTN_W / 2, y0 + SC(314), BTN_W, SC(36),
                    "Indietro", COL_SURFACE, COL_TEXT_DIM)) {
            step = 0;
            init = 0;
            azzera_campi();
            s->schermata = SCHERMATA_BENVENUTO;
        }
        return;
    }

    /* ================================================================
     *  Step 2/2 - Trainer: solo nome e cognome, nessun dato fisico
     * ================================================================ */
    if (reg_ruolo == 1) {
        disegna_header("Nuovo account trainer", "Step 2/2 - Dati anagrafici");

        int cx = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
        int y0 = TITOLO_H + SC(70);

        disegna_campo(cx - INPUT_W / 2, y0, INPUT_W, INPUT_H, 0);
        disegna_campo(cx - INPUT_W / 2, y0 + SC(80), INPUT_W, INPUT_H, 1);

        if (bottone_accent(cx - BTN_W / 2, y0 + SC(168), BTN_W, BTN_H, "REGISTRATI")) {
            if (strlen(campi[0].testo) < 1 || strlen(campi[1].testo) < 1) {
                imposta_errore(s, "Nome e cognome obbligatori.");
            } else {
                compila_trainer(&s->utente);
                int id = app_registra(&s->utente, s->utente.username, pwd, "trainer");

                if (id < 0) {
                    imposta_errore(s, "Errore. Username gia' usato?");
                } else {
                    s->id_utente = id;
                    strncpy(s->username, s->utente.username, DIM_NOME - 1);
                    strncpy(s->ruolo, "trainer", DIM_NOME - 1);

                    step = 0;
                    reg_ruolo = 0;
                    azzera_campi();
                    imposta_ok(s, "Account trainer creato!");
                    s->schermata = SCHERMATA_TRAINER;
                }
            }
        }

        if (bottone(cx - BTN_W / 2, y0 + SC(232), BTN_W, SC(36),
                    "<- Indietro", COL_SURFACE, COL_TEXT_DIM)) {
            step = 0;
            azzera_campi();
            aggiungi_campo("Username", 0);
            aggiungi_campo("Password", 1);
            aggiungi_campo("Conferma password", 1);
        }
        return;
    }

    /* ================================================================
     *  Step 2/2 - Cliente: dati fisici completi + preferenze
     * ================================================================ */
    disegna_header("Nuovo account", "Step 2/2 - Dati personali");

    int col1 = SIDEBAR_W + PADDING;
    int col2 = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2 + PADDING;
    int y0   = TITOLO_H + SC(60);

    for (int i = 0; i < num_campi; i++)
        disegna_campo(col1, y0 + i * SC(72), INPUT_W / 2 - SC(8), INPUT_H, i);

    int tx = col2;
    int ty = y0;
    disegna_toggle(tx, &ty);

    if (bottone_accent(tx, ty, SC(200), BTN_H, "REGISTRATI")) {
        if (strlen(campi[0].testo) < 1 || strlen(campi[1].testo) < 1) {
            imposta_errore(s, "Nome e cognome obbligatori.");
        } else {
            compila_utente(&s->utente);

            int id = app_registra(&s->utente, s->utente.username, pwd, "cliente");

            if (id < 0) {
                imposta_errore(s, "Errore. Username gia' usato?");
            } else {
                /* Salva peso iniziale */
                char oggi[16];
                time_t t = time(NULL);
                strftime(oggi, sizeof(oggi), "%Y-%m-%d", localtime(&t));
                app_salva_storico(id, s->utente.peso_kg, oggi);

                s->id_utente = id;
                strncpy(s->username, s->utente.username, DIM_NOME - 1);
                strncpy(s->ruolo, "cliente", DIM_NOME - 1);

                if (s->utente.servizio != SERVIZIO_SOLO_NUTRIZIONE) {
                    s->programma = costruisci_programma(&s->utente);
                    s->programma_pronto = 1;
                }
                if (s->utente.servizio != SERVIZIO_SOLO_ALLENAMENTO) {
                    s->piano = costruisci_piano(&s->utente);
                    s->piano_pronto = 1;
                }

                step = 0;
                azzera_campi();
                imposta_ok(s, "Account creato!");
                s->schermata = SCHERMATA_REPORT;
            }
        }
    }

    if (bottone(tx + SC(216), ty, SC(120), BTN_H,
                "<- Indietro", COL_SURFACE, COL_TEXT_DIM)) {
        step = 0;
        azzera_campi();
        aggiungi_campo("Username", 0);
        aggiungi_campo("Password", 1);
        aggiungi_campo("Conferma password", 1);
    }
}