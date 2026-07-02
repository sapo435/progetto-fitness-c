#include "gui_core.h"

/**
 * @file gui_widgets.c
 * @brief Widget grafici generici riusati da tutte le schermate: campi
 *        di testo, bottoni, sidebar di navigazione, messaggi toast,
 *        header e area di testo a scorrimento.
 */

Campo campi[MAX_CAMPI];
int   num_campi        = 0;
int   campo_attivo_idx = -1;

void azzera_campi(void)
{
    memset(campi, 0, sizeof(campi));
    num_campi = 0;
    campo_attivo_idx = -1;
}

int aggiungi_campo(const char *etichetta, int e_password)
{
    int i = num_campi++;
    strncpy(campi[i].etichetta, etichetta, 63);
    campi[i].e_password = e_password;
    return i;
}

void disegna_rettangolo_bordo(int x, int y, int w, int h, int r,
                               Color fill, Color bordo)
{
    float ratio = (float)r / ((w < h ? w : h) * 0.5f);
    DrawRectangleRounded((Rectangle){x, y, w, h}, ratio, 8, fill);
    DrawRectangleRoundedLines((Rectangle){x, y, w, h}, ratio, 1.5f, bordo);
}

int bottone(int x, int y, int w, int h, const char *testo,
            Color sf, Color ct)
{
    Vector2 m = GetMousePosition();
    int hover = CheckCollisionPointRec(m, (Rectangle){x, y, w, h});
    if (hover) {
        sf.r = (unsigned char)fmin(sf.r + 20, 255);
        sf.g = (unsigned char)fmin(sf.g + 20, 255);
        sf.b = (unsigned char)fmin(sf.b + 20, 255);
    }
    DrawRectangleRounded((Rectangle){x, y, w, h}, 0.2f, 8, sf);
    DrawRectangleRoundedLines((Rectangle){x, y, w, h}, 0.2f, 1.0f,
                              hover ? COL_ACCENT : COL_BORDER);
    int fs = FONT_NORMALE;
    int tw = MeasureText(testo, fs);
    DrawText(testo, x + (w - tw) / 2, y + (h - fs) / 2, fs, ct);
    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

int bottone_accent(int x, int y, int w, int h, const char *testo)
{
    Vector2 m = GetMousePosition();
    int hover = CheckCollisionPointRec(m, (Rectangle){x, y, w, h});
    Color sf = hover ? (Color){80, 255, 40, 255} : COL_ACCENT;
    DrawRectangleRounded((Rectangle){x, y, w, h}, 0.2f, 8, sf);
    int fs = FONT_NORMALE;
    int tw = MeasureText(testo, fs);
    DrawText(testo, x + (w - tw) / 2, y + (h - fs) / 2, fs, COL_BG);
    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void disegna_campo(int x, int y, int w, int h, int idx)
{
    Campo *c = &campi[idx];
    int attivo = (campo_attivo_idx == idx);
    disegna_rettangolo_bordo(x, y, w, h, RAGGIO, COL_INPUT_BG,
                             attivo ? COL_ACCENT : COL_BORDER);
    DrawText(c->etichetta, x, y - SC(20), SC(13), COL_TEXT_DIM);

    char mostrato[256];
    if (c->e_password) {
        int n = strlen(c->testo);
        memset(mostrato, '*', n);
        mostrato[n] = '\0';
    } else {
        strncpy(mostrato, c->testo, 255);
    }
    if (attivo && ((int)(GetTime() * 2) % 2 == 0)) {
        int n = strlen(mostrato);
        if (n < 255) { mostrato[n] = '|'; mostrato[n+1] = '\0'; }
    }
    DrawText(mostrato, x + SC(12), y + (h - FONT_NORMALE) / 2,
             FONT_NORMALE, COL_TEXT);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mp = GetMousePosition();
        if (CheckCollisionPointRec(mp, (Rectangle){x, y, w, h}))
            campo_attivo_idx = idx;
    }
    if (attivo) {
        int key = GetCharPressed();
        while (key > 0) {
            int n = strlen(c->testo);
            if (key >= 32 && n < 254) {
                c->testo[n] = (char)key;
                c->testo[n+1] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int n = strlen(c->testo);
            if (n > 0) c->testo[n-1] = '\0';
        }
        if (IsKeyPressed(KEY_TAB))
            campo_attivo_idx = (campo_attivo_idx + 1) % num_campi;
    }
}

/* SIDEBAR */
void disegna_sidebar(AppState *s)
{
    DrawRectangle(0, 0, SIDEBAR_W, FINESTRA_H, COL_SIDEBAR_BG);
    DrawLine(SIDEBAR_W, 0, SIDEBAR_W, FINESTRA_H, COL_BORDER);
    DrawRectangle(0, 0, SIDEBAR_W, TITOLO_H, COL_SURFACE);
    DrawLine(0, TITOLO_H, SIDEBAR_W, TITOLO_H, COL_BORDER);
    DrawText("NEW",     PADDING, SC(14), SC(26), COL_ACCENT);
    DrawText("FITNESS", PADDING, SC(38), SC(16), COL_TEXT_DIM);

    if (s->id_utente <= 0) return;

    s->pulse_t += GetFrameTime() * 2.5f;
    float alpha = sinf(s->pulse_t) * 0.5f + 0.5f;
    Color pc = {57, 255, 20, (unsigned char)(80 + alpha * 175)};
    DrawCircle(SIDEBAR_W - SC(18), SC(32), SC(6), pc);

    int y = TITOLO_H + PADDING;
    DrawText("UTENTE",    PADDING, y, SC(11), COL_TEXT_DIM); y += SC(18);
    DrawText(s->username, PADDING, y, SC(16), COL_TEXT);     y += SC(22);
    DrawRectangle(PADDING, y, SC(60), SC(20), COL_ACCENT_DIM);
    DrawText(s->ruolo, PADDING + SC(4), y + SC(3), SC(12), COL_ACCENT);
    y += SC(40);
    DrawLine(PADDING, y, SIDEBAR_W - PADDING, y, COL_BORDER);
    y += PADDING;

    if (strcmp(s->ruolo, "trainer") == 0) {
        DrawText("MENU TRAINER", PADDING, y, SC(11), COL_TEXT_DIM); y += SC(18);
        const char *voci[] = {"Lista clienti", "Schede cliente",
                              "Piani cliente",  "Storico cliente"};
        Schermata   dest[] = {SCHERMATA_TRAINER_LISTA,   SCHERMATA_TRAINER_SCHEDE,
                              SCHERMATA_TRAINER_PIANI,   SCHERMATA_TRAINER_STORICO};
        for (int i = 0; i < 4; i++) {
            Color sf = (s->schermata == dest[i]) ? COL_CARD   : COL_SURFACE;
            Color ct = (s->schermata == dest[i]) ? COL_ACCENT : COL_TEXT;
            if (bottone(PADDING, y, SIDEBAR_W - PADDING*2, SC(36), voci[i], sf, ct))
                { s->schermata = dest[i]; s->scroll_y = 0; }
            y += SC(44);
        }
        y += SC(8);
        if (bottone(PADDING, y, SIDEBAR_W - PADDING*2, SC(36),
                    "Gestione", COL_SURFACE, COL_TEXT))
            s->schermata = SCHERMATA_TRAINER;
    } else {
        DrawText("MENU", PADDING, y, SC(11), COL_TEXT_DIM); y += SC(18);
        const char *voci[] = {"Home", "Report", "Feedback",
                              "Storico pesi", "Schede", "Piani"};
        Schermata   dest[] = {SCHERMATA_HOME,    SCHERMATA_REPORT,  SCHERMATA_FEEDBACK,
                              SCHERMATA_STORICO, SCHERMATA_SCHEDE,  SCHERMATA_PIANI};
        for (int i = 0; i < 6; i++) {
            /* Blocca sezioni non scelte */
            int bloccato = 0;
            if (i == 4 && s->utente.servizio == SERVIZIO_SOLO_NUTRIZIONE)
                bloccato = 1;
            if (i == 5 && s->utente.servizio == SERVIZIO_SOLO_ALLENAMENTO)
                bloccato = 1;

            if (bloccato) {
                DrawRectangleRounded(
                    (Rectangle){PADDING, y, SIDEBAR_W - PADDING * 2, SC(36)},
                    0.2f, 8, (Color){20, 20, 20, 255});
                char lbl[64];
                snprintf(lbl, sizeof(lbl), "%s [X]", voci[i]);
                int tw = MeasureText(lbl, FONT_NORMALE);
                DrawText(lbl,
                         PADDING + (SIDEBAR_W - PADDING * 2 - tw) / 2,
                         y + (SC(36) - FONT_NORMALE) / 2,
                         FONT_NORMALE, COL_TEXT_DIM);
            } else {
                Color sf = (s->schermata == dest[i]) ? COL_CARD   : COL_SURFACE;
                Color ct = (s->schermata == dest[i]) ? COL_ACCENT : COL_TEXT;
                if (bottone(PADDING, y, SIDEBAR_W - PADDING*2, SC(36), voci[i], sf, ct))
                    { s->schermata = dest[i]; s->scroll_y = 0; }
            }
            y += SC(44);
        }
    }

    /* Logout */
    if (bottone(PADDING, FINESTRA_H - SC(56), SIDEBAR_W - PADDING*2, SC(36),
                "Logout", COL_SURFACE, COL_DANGER)) {
        memset(s, 0, sizeof(AppState));
        s->schermata = SCHERMATA_BENVENUTO;
        s->schermata_precedente = SCHERMATA_BENVENUTO;
    }
}

/* MESSAGGI */
void imposta_errore(AppState *s, const char *msg)
{
    strncpy(s->msg_errore, msg, 255);
    s->msg_ok[0] = '\0';
    s->msg_timer = 4.0f;
}

void imposta_ok(AppState *s, const char *msg)
{
    strncpy(s->msg_ok, msg, 255);
    s->msg_errore[0] = '\0';
    s->msg_timer = 3.5f;
}

void disegna_messaggi(AppState *s)
{
    if (s->msg_timer <= 0) return;
    s->msg_timer -= GetFrameTime();
    if (s->msg_timer < 0) {
        s->msg_errore[0] = s->msg_ok[0] = '\0';
        return;
    }
    const char *msg = s->msg_errore[0] ? s->msg_errore
                    : s->msg_ok[0]     ? s->msg_ok : NULL;
    if (!msg) return;
    Color col = s->msg_errore[0] ? COL_DANGER : COL_OK;
    int fs = FONT_NORMALE;
    int tw = MeasureText(msg, fs);
    int bw = tw + SC(32), bh = SC(40);
    int bx = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W - bw) / 2;
    int by = FINESTRA_H - bh - SC(16);
    float a = (s->msg_timer < 1.0f) ? s->msg_timer : 1.0f;
    Color sfondo = {20, 20, 20, (unsigned char)(220*a)};
    Color tc     = {col.r, col.g, col.b, (unsigned char)(255*a)};
    DrawRectangleRounded((Rectangle){bx, by, bw, bh}, 0.3f, 8, sfondo);
    DrawRectangleRoundedLines((Rectangle){bx, by, bw, bh}, 0.3f, 1.5f, tc);
    DrawText(msg, bx + SC(16), by + (bh - fs) / 2, fs, tc);
}

/* HEADER E AREA TESTO */
void disegna_header(const char *titolo, const char *sub)
{
    int x = SIDEBAR_W + PADDING;
    DrawText(titolo, x, PADDING, FONT_GRANDE, COL_TEXT);
    if (sub && sub[0])
        DrawText(sub, x, PADDING + SC(36), FONT_PICCOLO, COL_TEXT_DIM);
    DrawLine(SIDEBAR_W, TITOLO_H, FINESTRA_W, TITOLO_H, COL_BORDER);
}

void schermata_testo(AppState *s, const char *titolo, const char *sub)
{
    disegna_header(titolo, sub);
    s->scroll_y -= GetMouseWheelMove() * SC(22);
    if (s->scroll_y < 0) s->scroll_y = 0;
    int x0 = SIDEBAR_W + PADDING;
    int y0 = TITOLO_H + SC(12);
    int ah = FINESTRA_H - y0 - SC(8);
    BeginScissorMode(SIDEBAR_W, y0, FINESTRA_W - SIDEBAR_W, ah);
    if (strlen(s->testo) == 0) {
        DrawText("Nessun dato disponibile.",
                 x0, y0 + SC(20) - (int)s->scroll_y, FONT_MEDIO, COL_TEXT_DIM);
    } else {
        char copia[4096];
        strncpy(copia, s->testo, sizeof(copia) - 1);
        char *riga = strtok(copia, "\n");
        int y = y0 + SC(10) - (int)s->scroll_y;
        while (riga) {
            DrawText(riga, x0, y, FONT_NORMALE, COL_TEXT);
            y += SC(22);
            riga = strtok(NULL, "\n");
        }
    }
    EndScissorMode();
    DrawText("Scorri con la rotella del mouse",
             FINESTRA_W - SC(260), FINESTRA_H - SC(22), FONT_PICCOLO, COL_TEXT_DIM);
}

void schermata_testo_inline(AppState *s, int y0)
{
    s->scroll_y -= GetMouseWheelMove() * SC(22);
    if (s->scroll_y < 0) s->scroll_y = 0;
    int x0 = SIDEBAR_W + PADDING;
    int ah = FINESTRA_H - y0 - SC(8);
    BeginScissorMode(SIDEBAR_W, y0, FINESTRA_W - SIDEBAR_W, ah);
    char copia[4096];
    strncpy(copia, s->testo, sizeof(copia) - 1);
    char *riga = strtok(copia, "\n");
    int y = y0 + SC(10) - (int)s->scroll_y;
    while (riga) {
        DrawText(riga, x0, y, FONT_NORMALE, COL_TEXT);
        y += SC(22);
        riga = strtok(NULL, "\n");
    }
    EndScissorMode();
}