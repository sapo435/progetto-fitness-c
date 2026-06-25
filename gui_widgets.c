/*
 * gui_widgets.c
 * -------------
 * Primitive di disegno riusabili:
 *   - gestione campi input
 *   - bottoni
 *   - sidebar con menu e logout
 *   - messaggi di errore/successo
 *   - header di pagina
 *   - area testo scrollabile (risposta server)
 *
 * Compilazione Windows (MinGW), insieme agli altri moduli:
 *   gcc gui_main.c gui_widgets.c gui_schermate.c
 *       client_logica.c client_api.c client_socket.c
 *       -o newfitness.exe -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32
 *       -I./raylib/include -L./raylib/lib
 */

#include "gui_core.h"

/* ================================================================
 *  VARIABILI GLOBALI — campi input
 * ================================================================ */

Campo campi[MAX_CAMPI];
int   num_campi       = 0;
int   campo_attivo_idx = -1;

/* ================================================================
 *  GESTIONE CAMPI INPUT
 * ================================================================ */

void azzera_campi(void)
{
    memset(campi, 0, sizeof(campi));
    num_campi        = 0;
    campo_attivo_idx = -1;
}

int aggiungi_campo(const char *etichetta, int e_password)
{
    int i = num_campi++;
    strncpy(campi[i].etichetta, etichetta, 63);
    campi[i].e_password = e_password;
    return i;
}

/* ================================================================
 *  PRIMITIVE DI DISEGNO
 * ================================================================ */

void disegna_rettangolo_bordo(int x, int y, int w, int h, int r,
                               Color fill, Color bordo)
{
    float ratio = (float)r / ((w < h ? w : h) * 0.5f);
    DrawRectangleRounded((Rectangle){x, y, w, h}, ratio, 8, fill);
    DrawRectangleRoundedLines((Rectangle){x, y, w, h}, ratio, 8, 1.5f, bordo);
}

/* Bottone normale con colori personalizzabili.
   Restituisce 1 se cliccato in questo frame. */
int bottone(int x, int y, int w, int h, const char *testo,
            Color colore_sfondo, Color colore_testo)
{
    Vector2 mouse = GetMousePosition();
    int hover = CheckCollisionPointRec(mouse, (Rectangle){x, y, w, h});

    Color sfondo = colore_sfondo;
    if (hover) {
        sfondo.r = (unsigned char)fmin(sfondo.r + 20, 255);
        sfondo.g = (unsigned char)fmin(sfondo.g + 20, 255);
        sfondo.b = (unsigned char)fmin(sfondo.b + 20, 255);
    }

    DrawRectangleRounded((Rectangle){x, y, w, h}, 0.2f, 8, sfondo);
    DrawRectangleRoundedLines((Rectangle){x, y, w, h}, 0.2f, 8, 1.0f,
                              hover ? COL_ACCENT : COL_BORDER);

    int fs = 17;
    int tw = MeasureText(testo, fs);
    DrawText(testo, x + (w - tw) / 2, y + (h - fs) / 2, fs, colore_testo);

    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

/* Bottone principale con sfondo accent (verde neon). */
int bottone_accent(int x, int y, int w, int h, const char *testo)
{
    Vector2 mouse = GetMousePosition();
    int hover = CheckCollisionPointRec(mouse, (Rectangle){x, y, w, h});
    Color sfondo = hover ? (Color){80, 255, 40, 255} : COL_ACCENT;

    DrawRectangleRounded((Rectangle){x, y, w, h}, 0.2f, 8, sfondo);

    int fs = 17;
    int tw = MeasureText(testo, fs);
    DrawText(testo, x + (w - tw) / 2, y + (h - fs) / 2, fs, COL_BG);

    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

/* Disegna un campo input e gestisce click + digitazione.
   idx è l'indice nell'array globale campi[]. */
void disegna_campo(int x, int y, int w, int h, int idx)
{
    Campo *c    = &campi[idx];
    int   attivo = (campo_attivo_idx == idx);

    Color bordo = attivo ? COL_ACCENT : COL_BORDER;
    disegna_rettangolo_bordo(x, y, w, h, RAGGIO, COL_INPUT_BG, bordo);

    DrawText(c->etichetta, x, y - 20, 13, COL_TEXT_DIM);

    /* Testo mostrato (asterischi per password) */
    char mostrato[256];
    if (c->e_password) {
        int n = strlen(c->testo);
        memset(mostrato, '*', n);
        mostrato[n] = '\0';
    } else {
        strncpy(mostrato, c->testo, 255);
    }

    /* Cursore lampeggiante ogni 0.5 s */
    if (attivo && ((int)(GetTime() * 2) % 2 == 0))
        strncat(mostrato, "|", 1);

    DrawText(mostrato, x + 12, y + (h - 18) / 2, 17, COL_TEXT);

    /* Gestione click: attiva il campo */
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        if (CheckCollisionPointRec(m, (Rectangle){x, y, w, h}))
            campo_attivo_idx = idx;
    }

    /* Gestione tasto: digita nel campo attivo */
    if (attivo) {
        int key = GetCharPressed();
        while (key > 0) {
            int n = strlen(c->testo);
            if (key >= 32 && n < 254) {
                c->testo[n]     = (char)key;
                c->testo[n + 1] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int n = strlen(c->testo);
            if (n > 0) c->testo[n - 1] = '\0';
        }
        if (IsKeyPressed(KEY_TAB))
            campo_attivo_idx = (campo_attivo_idx + 1) % num_campi;
    }
}

/* ================================================================
 *  SIDEBAR
 * ================================================================ */

void disegna_sidebar(AppState *s)
{
    DrawRectangle(0, 0, SIDEBAR_W, FINESTRA_H, COL_SIDEBAR_BG);
    DrawLine(SIDEBAR_W, 0, SIDEBAR_W, FINESTRA_H, COL_BORDER);

    /* Logo */
    DrawRectangle(0, 0, SIDEBAR_W, TITOLO_H, COL_SURFACE);
    DrawLine(0, TITOLO_H, SIDEBAR_W, TITOLO_H, COL_BORDER);
    DrawText("NEW",     PADDING, 14, 26, COL_ACCENT);
    DrawText("FITNESS", PADDING, 38, 16, COL_TEXT_DIM);

    if (s->token[0] == '\0') return;

    /* Indicatore sessione pulsante */
    s->pulse_t += GetFrameTime() * 2.5f;
    float alpha = sinf(s->pulse_t) * 0.5f + 0.5f;
    Color pulse_col = {57, 255, 20, (unsigned char)(80 + alpha * 175)};
    DrawCircle(SIDEBAR_W - 18, 32, 6, pulse_col);

    /* Info utente */
    int y = TITOLO_H + PADDING;
    DrawText("UTENTE",   PADDING, y, 11, COL_TEXT_DIM);  y += 18;
    DrawText(s->username, PADDING, y, 16, COL_TEXT);     y += 22;
    DrawRectangle(PADDING, y, 60, 20, COL_ACCENT_DIM);
    DrawText(s->ruolo, PADDING + 4, y + 3, 12, COL_ACCENT);
    y += 40;

    DrawLine(PADDING, y, SIDEBAR_W - PADDING, y, COL_BORDER);
    y += PADDING;

    /* Voci menu */
    if (strcmp(s->ruolo, "trainer") == 0) {
        DrawText("MENU TRAINER", PADDING, y, 11, COL_TEXT_DIM); y += 18;

        const char *voci[] = {"Lista clienti", "Schede", "Piani alimentari", "Storico pesi"};
        Schermata   dest[] = {SCHERMATA_TRAINER_LISTA, SCHERMATA_TRAINER_SCHEDE,
                              SCHERMATA_TRAINER_PIANI, SCHERMATA_TRAINER_STORICO};
        for (int i = 0; i < 4; i++) {
            if (bottone(PADDING, y, SIDEBAR_W - PADDING*2, 36, voci[i],
                        COL_SURFACE, COL_TEXT))
                { s->schermata = dest[i]; s->scroll_y = 0; }
            y += 44;
        }
    } else {
        DrawText("MENU", PADDING, y, 11, COL_TEXT_DIM); y += 18;

        const char *voci[] = {"Home", "Report", "Feedback",
                              "Storico pesi", "Schede", "Piani"};
        Schermata   dest[] = {SCHERMATA_HOME,    SCHERMATA_REPORT,  SCHERMATA_FEEDBACK,
                              SCHERMATA_STORICO, SCHERMATA_SCHEDE,  SCHERMATA_PIANI};
        for (int i = 0; i < 6; i++) {
            Color sf = (s->schermata == dest[i]) ? COL_CARD    : COL_SURFACE;
            Color ct = (s->schermata == dest[i]) ? COL_ACCENT  : COL_TEXT;
            if (bottone(PADDING, y, SIDEBAR_W - PADDING*2, 36, voci[i], sf, ct))
                { s->schermata = dest[i]; s->scroll_y = 0; }
            y += 44;
        }
    }

    /* Logout in fondo */
    if (bottone(PADDING, FINESTRA_H - 56, SIDEBAR_W - PADDING*2, 36,
                "Logout", COL_SURFACE, COL_DANGER)) {
        logout_server(s->token);
        memset(s, 0, sizeof(AppState));
        s->schermata = SCHERMATA_BENVENUTO;
    }
}

/* ================================================================
 *  MESSAGGI DI STATO
 * ================================================================ */

void imposta_errore(AppState *s, const char *msg)
{
    strncpy(s->msg_errore, msg, 255);
    s->msg_ok[0]  = '\0';
    s->msg_timer  = 4.0f;
}

void imposta_ok(AppState *s, const char *msg)
{
    strncpy(s->msg_ok, msg, 255);
    s->msg_errore[0] = '\0';
    s->msg_timer     = 3.5f;
}

/* Mostra il toast in sovrimpressione (errore rosso / ok verde).
   Da chiamare una volta per frame, dopo EndDrawing non ancora chiamato. */
void disegna_messaggi(AppState *s)
{
    if (s->msg_timer <= 0) return;

    s->msg_timer -= GetFrameTime();
    if (s->msg_timer < 0) {
        s->msg_errore[0] = '\0';
        s->msg_ok[0]     = '\0';
        return;
    }

    const char *msg = s->msg_errore[0] ? s->msg_errore
                    : s->msg_ok[0]     ? s->msg_ok
                    : NULL;
    if (!msg) return;

    Color col = s->msg_errore[0] ? COL_DANGER : COL_OK;
    int   tw  = MeasureText(msg, 15);
    int   bw  = tw + 32, bh = 40;
    int   bx  = SIDEBAR_W + (FINESTRA_W - SIDEBAR_W - bw) / 2;
    int   by  = FINESTRA_H - bh - 16;

    float a = (s->msg_timer < 1.0f) ? s->msg_timer : 1.0f;
    Color sfondo    = {20,    20,    20,    (unsigned char)(220 * a)};
    Color testo_col = {col.r, col.g, col.b, (unsigned char)(255 * a)};

    DrawRectangleRounded((Rectangle){bx, by, bw, bh}, 0.3f, 8, sfondo);
    DrawRectangleRoundedLines((Rectangle){bx, by, bw, bh}, 0.3f, 8, 1.5f, testo_col);
    DrawText(msg, bx + 16, by + (bh - 15) / 2, 15, testo_col);
}

/* ================================================================
 *  HEADER DI PAGINA
 * ================================================================ */

void disegna_header(const char *titolo, const char *sottotitolo)
{
    int x = SIDEBAR_W + PADDING;
    DrawText(titolo, x, PADDING, 28, COL_TEXT);
    if (sottotitolo && sottotitolo[0])
        DrawText(sottotitolo, x, PADDING + 36, 14, COL_TEXT_DIM);
    DrawLine(SIDEBAR_W, TITOLO_H, FINESTRA_W, TITOLO_H, COL_BORDER);
}

/* ================================================================
 *  AREA TESTO SCROLLABILE — risposta server
 * ================================================================ */

/* Schermata completa con header: usata per storico, schede, piani. */
void schermata_testo_server(AppState *s, const char *titolo,
                             const char *sottotitolo)
{
    disegna_header(titolo, sottotitolo);

    s->scroll_y -= GetMouseWheelMove() * 22;
    if (s->scroll_y < 0) s->scroll_y = 0;

    int x0     = SIDEBAR_W + PADDING;
    int y0     = TITOLO_H + 12;
    int area_h = FINESTRA_H - y0 - 8;

    BeginScissorMode(SIDEBAR_W, y0, FINESTRA_W - SIDEBAR_W, area_h);

    if (strlen(s->testo_server) == 0 ||
        strncmp(s->testo_server, "ERRORE", 6) == 0)
    {
        DrawText("Nessun dato disponibile.",
                 x0, y0 + 20 - (int)s->scroll_y, 16, COL_TEXT_DIM);
    } else {
        char  copia[4096];
        strncpy(copia, s->testo_server, sizeof(copia) - 1);
        char *riga = strtok(copia, "\n");
        int   y    = y0 + 10 - (int)s->scroll_y;
        while (riga) {
            /* Sostituisce '|' con spazio per leggibilita' */
            char pulita[256];
            strncpy(pulita, riga, 255);
            for (int i = 0; pulita[i]; i++)
                if (pulita[i] == '|') pulita[i] = ' ';
            DrawText(pulita, x0, y, 14, COL_TEXT);
            y   += 20;
            riga = strtok(NULL, "\n");
        }
    }

    EndScissorMode();
    DrawText("Scorri con la rotella del mouse",
             FINESTRA_W - 260, FINESTRA_H - 22, 12, COL_TEXT_DIM);
}

/* Variante inline: non ridisegna l'header, parte da y0 già calcolato.
   Usata da schermata_trainer_schede() che ha un suo form sopra. */
void schermata_testo_server_inline(AppState *s, int y0)
{
    s->scroll_y -= GetMouseWheelMove() * 22;
    if (s->scroll_y < 0) s->scroll_y = 0;

    int x0     = SIDEBAR_W + PADDING;
    int area_h = FINESTRA_H - y0 - 8;

    BeginScissorMode(SIDEBAR_W, y0, FINESTRA_W - SIDEBAR_W, area_h);

    char  copia[4096];
    strncpy(copia, s->testo_server, sizeof(copia) - 1);
    char *riga = strtok(copia, "\n");
    int   y    = y0 + 10 - (int)s->scroll_y;
    while (riga) {
        char pulita[256];
        strncpy(pulita, riga, 255);
        for (int i = 0; pulita[i]; i++)
            if (pulita[i] == '|') pulita[i] = ' ';
        DrawText(pulita, x0, y, 14, COL_TEXT);
        y   += 20;
        riga = strtok(NULL, "\n");
    }

    EndScissorMode();
}
