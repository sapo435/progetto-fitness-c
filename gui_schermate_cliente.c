#include "gui_core.h"
#include "lista_dinamica.h"

/**
 * @file gui_schermate_cliente.c
 * @brief Schermate dell'area cliente: home, report, feedback settimanale.
 */
/* HOME */
void schermata_home(AppState *s)
{
    disegna_header("Home", s->username);

    int cx  = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
    int y0  = TITOLO_H + SC(50);
    int bw2 = (FINESTRA_W - SIDEBAR_W - PADDING * 3) / 2;

    char saluto[128];
    snprintf(saluto, sizeof(saluto), "Benvenuto/a, %s!", s->username);
    DrawText(saluto, cx - MeasureText(saluto, SC(22)) / 2, y0, SC(22), COL_TEXT);
    y0 += SC(60);

    if (s->utente.peso_kg > 0) {
        char bmi[32], peso[32], kcal[32], trg[32];

        snprintf(bmi, sizeof(bmi), "BMI %.1f", s->utente.bmi);
        snprintf(peso, sizeof(peso), "%.1f kg", s->utente.peso_kg);
        snprintf(kcal, sizeof(kcal), "%.0f kcal/die", s->utente.kcal_obiettivo);

        float sett = stima_settimane(&s->utente);
        if (sett > 0)
            snprintf(trg, sizeof(trg), "~%.0f settimane", sett);
        else
            strncpy(trg, "Obiettivo!", sizeof(trg));

        const char *lab[] = {"BMI", "Peso", "Kcal obiettivo", "Traguardo"};
        const char *val[] = {bmi, peso, kcal, trg};
        int cw = (FINESTRA_W - SIDEBAR_W - PADDING * 5) / 4;

        for (int i = 0; i < 4; i++) {
            int cx2 = SIDEBAR_W + PADDING + i * (cw + PADDING);
            disegna_rettangolo_bordo(cx2, y0, cw, SC(80), RAGGIO,
                                     COL_CARD, COL_BORDER);
            DrawText(lab[i], cx2 + SC(12), y0 + SC(10), SC(12), COL_TEXT_DIM);
            DrawText(val[i], cx2 + SC(12), y0 + SC(34), SC(18), COL_ACCENT);
        }
        y0 += SC(110);
    }

    int bx1 = SIDEBAR_W + PADDING;
    int bx2 = bx1 + bw2 + PADDING;

    if (bottone_accent(bx1, y0, bw2, BTN_H, "Report e programma"))
        s->schermata = SCHERMATA_REPORT;

    if (bottone(bx2, y0, bw2, BTN_H,
                "Feedback settimanale", COL_SURFACE, COL_TEXT))
        s->schermata = SCHERMATA_FEEDBACK;

    y0 += BTN_H + PADDING;

    if (bottone(bx1, y0, bw2, BTN_H, "Storico pesi", COL_SURFACE, COL_TEXT)) {
        s->schermata = SCHERMATA_STORICO;
        s->scroll_y = 0;
    }
    if (bottone(bx2, y0, bw2, BTN_H, "Le mie schede", COL_SURFACE, COL_TEXT)) {
        s->schermata = SCHERMATA_SCHEDE;
        s->scroll_y = 0;
    }

    y0 += BTN_H + PADDING;

    if (bottone(bx1, y0, bw2, BTN_H, "Piani alimentari", COL_SURFACE, COL_TEXT)) {
        s->schermata = SCHERMATA_PIANI;
        s->scroll_y = 0;
    }
}

/* REPORT */
void schermata_report(AppState *s)
{
    disegna_header("Report", "Le tue metriche e il tuo programma");

    Utente *u = &s->utente;
    int x0 = SIDEBAR_W + PADDING;
    int y  = TITOLO_H + SC(16);
    int cw = (FINESTRA_W - SIDEBAR_W - PADDING * 5) / 4;
    char buf[128];

    /* Card metriche principali */
    const char *ml[] = {"BMI", "BMR (kcal)", "TDEE (kcal)", "Kcal obiettivo"};
    char mv[4][32];
    snprintf(mv[0], 32, "%.1f (%s)", u->bmi,
             u->bmi < 18.5f ? "Sottopeso" :
             u->bmi < 25 ? "Normopeso" :
             u->bmi < 30 ? "Sovrappeso" : "Obesita'");
    snprintf(mv[1], 32, "%.0f", u->bmr);
    snprintf(mv[2], 32, "%.0f", u->tdee_kcal);
    snprintf(mv[3], 32, "%.0f", u->kcal_obiettivo);

    for (int i = 0; i < 4; i++) {
        int cx2 = x0 + i * (cw + PADDING);
        disegna_rettangolo_bordo(cx2, y, cw, SC(70), RAGGIO, COL_CARD, COL_BORDER);
        DrawText(ml[i], cx2 + SC(10), y + SC(8), SC(11), COL_TEXT_DIM);
        DrawText(mv[i], cx2 + SC(10), y + SC(28), SC(15), COL_ACCENT);
    }
    y += SC(84);

    /* Card macronutrienti */
    const char *nl[] = {"Proteine", "Carboidrati", "Grassi", "Acqua"};
    char nv[4][24];
    snprintf(nv[0], 24, "%.0f g", u->proteine_g);
    snprintf(nv[1], 24, "%.0f g", u->carboidrati_g);
    snprintf(nv[2], 24, "%.0f g", u->grassi_g);
    snprintf(nv[3], 24, "%.0f ml", fabbisogno_acqua(u->peso_kg, 1.0f));

    for (int i = 0; i < 4; i++) {
        int cx2 = x0 + i * (cw + PADDING);
        disegna_rettangolo_bordo(cx2, y, cw, SC(60), RAGGIO, COL_CARD, COL_BORDER);
        DrawText(nl[i], cx2 + SC(10), y + SC(6), SC(11), COL_TEXT_DIM);
        DrawText(nv[i], cx2 + SC(10), y + SC(26), SC(17), COL_TEXT);
    }
    y += SC(76);

    /* Area scrollabile per programma e piano */
    s->scroll_y -= GetMouseWheelMove() * SC(24);
    if (s->scroll_y < 0) s->scroll_y = 0;

    int ah = FINESTRA_H - y - SC(8);
    BeginScissorMode(SIDEBAR_W, y, FINESTRA_W - SIDEBAR_W, ah);
    int ys = y - (int)s->scroll_y;

    /* Programma di allenamento */
    if (s->programma_pronto && u->servizio != SERVIZIO_SOLO_NUTRIZIONE) {
        ProgrammaAllenamento *p = &s->programma;

        DrawText(p->nome, x0, ys, SC(18), COL_ACCENT);
        ys += SC(28);
        DrawText(p->descrizione, x0, ys, SC(13), COL_TEXT_DIM);
        ys += SC(22);

        for (int i = 0; i < p->num_sessioni; i++) {
            Sessione *ss = &p->sessioni[i];

            DrawRectangle(x0, ys, FINESTRA_W - SIDEBAR_W - PADDING * 2,
                          SC(28), COL_SURFACE);
            DrawText(ss->titolo, x0 + SC(10), ys + SC(6), SC(14), COL_TEXT);
            ys += SC(32);

            for (int j = 0; j < ss->num_esercizi; j++) {
                Esercizio *e = &ss->esercizi[j];
                snprintf(buf, sizeof(buf), "  %-22s %-12s %dx%-3d %ds",
                         e->nome_esercizio, e->gruppo_muscolare,
                         e->serie, e->ripetizioni, e->riposo_sec);
                DrawText(buf, x0, ys, SC(13), COL_TEXT);
                ys += SC(18);

                if (e->note[0]) {
                    snprintf(buf, sizeof(buf), "    -> %s", e->note);
                    DrawText(buf, x0, ys, SC(12), COL_TEXT_DIM);
                    ys += SC(16);
                }
            }
            ys += SC(10);
        }
        ys += SC(16);
    }

    /* Piano nutrizionale */
    if (s->piano_pronto && u->servizio != SERVIZIO_SOLO_ALLENAMENTO) {
        PianoNutrizionale *p = &s->piano;

        DrawText(p->etichetta, x0, ys, SC(18), COL_ACCENT);
        ys += SC(28);
        float tk = 0;

        for (int i = 0; i < p->num_pasti; i++) {
            Pasto *m = &p->pasti[i];

            DrawRectangle(x0, ys, FINESTRA_W - SIDEBAR_W - PADDING * 2,
                          SC(24), COL_SURFACE);
            DrawText(m->nome, x0 + SC(10), ys + SC(4), SC(14), COL_TEXT);
            ys += SC(28);

            snprintf(buf, sizeof(buf), "  %s", m->alimenti);
            DrawText(buf, x0, ys, SC(12), COL_TEXT_DIM);
            ys += SC(16);

            snprintf(buf, sizeof(buf), "  %.0f kcal | P:%.0fg C:%.0fg G:%.0fg",
                     m->kcal, m->proteine_g, m->carboidrati_g, m->grassi_g);
            DrawText(buf, x0, ys, SC(13), COL_ACCENT);
            ys += SC(18);

            if (m->note[0]) {
                snprintf(buf, sizeof(buf), "  %s", m->note);
                DrawText(buf, x0, ys, SC(12), COL_TEXT_DIM);
                ys += SC(16);
            }
            tk += m->kcal;
            ys += SC(6);
        }

        snprintf(buf, sizeof(buf), "TOTALE: %.0f kcal", tk);
        DrawText(buf, x0, ys, SC(15), COL_TEXT);
    }

    EndScissorMode();
    DrawText("Scorri con la rotella del mouse",
             FINESTRA_W - SC(260), FINESTRA_H - SC(22),
             SC(12), COL_TEXT_DIM);
}

/* FEEDBACK */
void schermata_feedback(AppState *s)
{
    static int all_sc = -1, diet_sc = -1, motivo_sc = -1;
    static int agg_peso = -1, peso_att = 0;
    static char peso_txt[32] = "";

    disegna_header("Feedback", "Settimana corrente");

    Utente *u = &s->utente;
    int x0 = SIDEBAR_W + PADDING;
    int cx = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W) / 2;
    int y  = TITOLO_H + SC(20);

    /* Aggiornamento peso */
    DrawText("Aggiornare il peso?", x0, y, SC(15), COL_TEXT_DIM);
    y += SC(28);

    const char *sp[] = {"Si", "No"};
    for (int i = 0; i < 2; i++) {
        Color sf = (agg_peso == i) ? COL_ACCENT_DIM : COL_SURFACE;
        Color ct = (agg_peso == i) ? COL_ACCENT : COL_TEXT;
        if (bottone(x0 + i * SC(96), y, SC(88), SC(34), sp[i], sf, ct))
            agg_peso = i;
    }
    y += SC(48);

    if (agg_peso == 0) {
        DrawText("Nuovo peso (kg)", x0, y, SC(13), COL_TEXT_DIM);
        y += SC(20);

        disegna_rettangolo_bordo(x0, y, SC(200), INPUT_H, RAGGIO,
                                 COL_INPUT_BG,
                                 peso_att ? COL_ACCENT : COL_BORDER);
        DrawText(peso_txt, x0 + SC(12), y + (INPUT_H - SC(18)) / 2,
                 SC(17), COL_TEXT);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = GetMousePosition();
            peso_att = CheckCollisionPointRec(
                m, (Rectangle){x0, y, SC(200), INPUT_H});
            campo_attivo_idx = -1;
        }

        if (peso_att) {
            int k = GetCharPressed();
            while (k > 0) {
                int n = strlen(peso_txt);
                if ((k >= '0' && k <= '9') || k == '.') {
                    if (n < 30) {
                        peso_txt[n] = k;
                        peso_txt[n + 1] = '\0';
                    }
                }
                k = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                int n = strlen(peso_txt);
                if (n > 0) peso_txt[n - 1] = '\0';
            }
        }
        y += INPUT_H + SC(14);
    }

    /* Valutazione allenamento */
    DrawLine(x0, y, FINESTRA_W - PADDING, y, COL_BORDER);
    y += SC(16);
    DrawText("Come e' stato l'allenamento?", x0, y, SC(15), COL_TEXT_DIM);
    y += SC(28);

    const char *all[] = {"Facile", "Giusto", "Difficile", "Non fatto"};
    for (int i = 0; i < 4; i++) {
        Color sf = (all_sc == i) ? COL_ACCENT_DIM : COL_SURFACE;
        Color ct = (all_sc == i) ? COL_ACCENT : COL_TEXT;
        if (bottone(x0 + i * SC(140), y, SC(132), SC(34), all[i], sf, ct))
            all_sc = i;
    }
    y += SC(48);

    if (all_sc == 3) {
        DrawText("Motivo:", x0, y, SC(13), COL_TEXT_DIM);
        y += SC(20);

        const char *mot[] = {"Tempo", "Stanchezza", "Motivazione"};
        for (int i = 0; i < 3; i++) {
            Color sf = (motivo_sc == i) ? COL_ACCENT_DIM : COL_SURFACE;
            Color ct = (motivo_sc == i) ? COL_ACCENT : COL_TEXT;
            if (bottone(x0 + i * SC(144), y, SC(136), SC(34), mot[i], sf, ct))
                motivo_sc = i;
        }
        y += SC(48);
    }

    /* Valutazione dieta */
    DrawLine(x0, y, FINESTRA_W - PADDING, y, COL_BORDER);
    y += SC(16);
    DrawText("Come hai seguito la dieta?", x0, y, SC(15), COL_TEXT_DIM);
    y += SC(28);

    const char *diet[] = {"Seguita", "Parziale", "Restrittiva", "Abbondante"};
    for (int i = 0; i < 4; i++) {
        Color sf = (diet_sc == i) ? COL_ACCENT_DIM : COL_SURFACE;
        Color ct = (diet_sc == i) ? COL_ACCENT : COL_TEXT;
        if (bottone(x0 + i * SC(140), y, SC(132), SC(34), diet[i], sf, ct))
            diet_sc = i;
    }
    y += SC(60);

    /* Salva feedback */
    if (bottone_accent(cx - SC(130), y, SC(260), BTN_H, "SALVA FEEDBACK")) {

        /* Aggiorna peso */
        if (agg_peso == 0 && strlen(peso_txt) > 0) {
            float np = atof(peso_txt);
            if (np > 20.0f) {
                registra_peso(u, np);
                char data[16];
                time_t t = time(NULL);
                strftime(data, sizeof(data), "%Y-%m-%d", localtime(&t));
                app_salva_storico(s->id_utente, np, data);
            }
        }

        /* Aggiusta kcal in base alla dieta */
        if (diet_sc == 2) {
            u->kcal_obiettivo += 100.0f;
            calcola_macronutrienti(u->peso_kg, u->kcal_obiettivo,
                                   u->obiettivo, &u->proteine_g,
                                   &u->grassi_g, &u->carboidrati_g);
        } else if (diet_sc == 3) {
            u->kcal_obiettivo -= 50.0f;
            calcola_macronutrienti(u->peso_kg, u->kcal_obiettivo,
                                   u->obiettivo, &u->proteine_g,
                                   &u->grassi_g, &u->carboidrati_g);
        }

        /* Aggiusta livello attivita' */
        if (all_sc == 0 && u->livello_attivita < 1.725f) {
            u->livello_attivita += 0.175f;
            calcola_metriche(u);
        }

        u->settimana_corrente++;
        if (all_sc == 1)
            u->sessioni_completate += 3;

        /* Aggiorna programma e piano con i dati correnti, rispettando
           l'eventuale variante assegnata dal trainer (come fa il login). */
        int var_s = app_get_variante_scheda(s->id_utente);
        s->programma = (var_s >= 0)
            ? costruisci_programma_ex(u, var_s)
            : costruisci_programma(u);

        int var_p = app_get_variante_piano(s->id_utente);
        s->piano = (var_p >= 0)
            ? costruisci_piano_ex(u, var_p)
            : costruisci_piano(u);

        s->programma_pronto = 1;
        s->piano_pronto     = 1;

        s->ultima_caricata = SCHERMATA_BENVENUTO;

        /* Reset stato feedback */
        all_sc = -1;
        diet_sc = -1;
        motivo_sc = -1;
        agg_peso = -1;
        memset(peso_txt, 0, sizeof(peso_txt));
        peso_att = 0;

        imposta_ok(s, "Feedback salvato!");
        s->schermata = SCHERMATA_HOME;
    }
}