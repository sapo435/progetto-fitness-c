#include "gui_core.h"
#include "lista_dinamica.h"

/**
 * @file gui_schermate_trainer.c
 * @brief Schermate dell'area trainer: gestione (genera schede/piani)
 *        e ricerca cliente per schede, storico e piani.
 */

/* AREA TRAINER - gestione schede e piani */
/* HELPER: formatta programma/piano in testo */
static void prog_to_buf(const ProgrammaAllenamento *p, char *buf, int sz)
{
    char r[300];
    snprintf(r,sizeof(r),"== %s ==\n%s\n",p->nome,p->descrizione);
    strncat(buf,r,sz-strlen(buf)-1);
    for (int i=0;i<p->num_sessioni;i++) {
        const Sessione *ss=&p->sessioni[i];
        strncat(buf,"\n",sz-strlen(buf)-1);
        snprintf(r,sizeof(r),">> %s\n",ss->titolo);
        strncat(buf,r,sz-strlen(buf)-1);
        strncat(buf,
            "   Esercizio              Muscolo      Serie Rip  Rec\n"
            "   --------------------------------------------------\n",sz-strlen(buf)-1);
        for (int j=0;j<ss->num_esercizi;j++) {
            const Esercizio *e=&ss->esercizi[j];
            snprintf(r,sizeof(r),"   %-22s %-12s %2dx%-3d %3ds\n",
                     e->nome_esercizio,e->gruppo_muscolare,
                     e->serie,e->ripetizioni,e->riposo_sec);
            strncat(buf,r,sz-strlen(buf)-1);
            if (e->note[0]) {
                snprintf(r,sizeof(r),"      -> %s\n",e->note);
                strncat(buf,r,sz-strlen(buf)-1);
            }
        }
    }
    strncat(buf,"\n",sz-strlen(buf)-1);
}

static void pn_to_buf(const PianoNutrizionale *p, char *buf, int sz)
{
    char r[300];
    snprintf(r,sizeof(r),"== %s ==\n\n",p->etichetta);
    strncat(buf,r,sz-strlen(buf)-1);
    float tk=0;
    for (int i=0;i<p->num_pasti;i++) {
        const Pasto *m=&p->pasti[i];
        snprintf(r,sizeof(r),">> %s\n",m->nome);
        strncat(buf,r,sz-strlen(buf)-1);
        strncat(buf,"   -----------------------------------------\n",sz-strlen(buf)-1);
        snprintf(r,sizeof(r),"   %s\n",m->alimenti);
        strncat(buf,r,sz-strlen(buf)-1);
        snprintf(r,sizeof(r),"   %.0f kcal | P:%.0fg C:%.0fg G:%.0fg\n",
                 m->kcal,m->proteine_g,m->carboidrati_g,m->grassi_g);
        strncat(buf,r,sz-strlen(buf)-1);
        if (m->note[0]) {
            snprintf(r,sizeof(r),"   Nota: %s\n",m->note);
            strncat(buf,r,sz-strlen(buf)-1);
        }
        strncat(buf,"\n",sz-strlen(buf)-1); tk+=m->kcal;
    }
    snprintf(r,sizeof(r),"TOTALE: %.0f kcal\n\n",tk);
    strncat(buf,r,sz-strlen(buf)-1);
}

/* Prepara dati cliente da DB (valori calcolati se mancanti) */
static int prepara_utente_db(int id, Utente *u)
{
    if (!app_get_utente(id, u)) return 0;
    u->bmi = calcola_bmi(u->peso_kg, u->altezza_cm);
    u->obiettivo = determina_obiettivo(u);
    if (u->livello_attivita < 0.1f) u->livello_attivita = 1.375f;
    calcola_metriche(u);
    return 1;
}

/* CARICA DATI CLIENTE per trainer (schede / piani) */
static void carica_schede_cliente(int id_cliente, char *buf, int buf_size)
{
    buf[0]='\0';
    Utente u;
    memset(&u, 0, sizeof(u));
    if (prepara_utente_db(id_cliente, &u) &&
        u.servizio != SERVIZIO_SOLO_NUTRIZIONE) {
        int var = app_get_variante_scheda(id_cliente);
        ProgrammaAllenamento prog = (var >= 0)
            ? costruisci_programma_ex(&u, var)
            : costruisci_programma(&u);
        prog_to_buf(&prog, buf, buf_size);
    }
    char db_s[2048]="";
    app_get_schede(id_cliente, db_s, sizeof(db_s));
    if (strlen(db_s)>0) {
        strncat(buf,"== Schede aggiuntive dal trainer ==\n",buf_size-strlen(buf)-1);
        strncat(buf,db_s,buf_size-strlen(buf)-1);
    }
    if (strlen(buf)==0)
        strncpy(buf,"Nessun dato disponibile.",buf_size-1);
}

static void carica_piani_cliente(int id_cliente, char *buf, int buf_size)
{
    buf[0]='\0';
    Utente u;
    memset(&u, 0, sizeof(u));
    if (prepara_utente_db(id_cliente, &u) &&
        u.servizio != SERVIZIO_SOLO_ALLENAMENTO) {
        int var = app_get_variante_piano(id_cliente);
        PianoNutrizionale pn = (var >= 0)
            ? costruisci_piano_ex(&u, var)
            : costruisci_piano(&u);
        pn_to_buf(&pn, buf, buf_size);
    }
    char db_p[2048]="";
    app_get_piani(id_cliente, db_p, sizeof(db_p));
    if (strlen(db_p)>0) {
        strncat(buf,"== Piani aggiuntivi dal trainer ==\n",buf_size-strlen(buf)-1);
        strncat(buf,db_p,buf_size-strlen(buf)-1);
    }
    if (strlen(buf)==0)
        strncpy(buf,"Nessun dato disponibile.",buf_size-1);
}

/* Campo di ricerca cliente con bottone Cerca. */
static int cerca_cliente_ui(AppState *s,
                             char *cerca, int cerca_size, int *cerca_att,
                             char *nome_trovato, int nome_size,
                             int *id_trovato, int *y_out)
{
    int x0=SIDEBAR_W+PADDING;
    int y=TITOLO_H+SC(24);
    int premuto=0;

    DrawText("Nome o username cliente",x0,y,SC(12),COL_TEXT_DIM); y+=SC(20);
    disegna_rettangolo_bordo(x0,y,SC(340),INPUT_H,RAGGIO,COL_INPUT_BG,
                             (*cerca_att)?COL_ACCENT:COL_BORDER);
    DrawText(cerca,x0+SC(12),y+(INPUT_H-SC(18))/2,SC(17),COL_TEXT);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m=GetMousePosition();
        *cerca_att=CheckCollisionPointRec(m,(Rectangle){x0,y,SC(340),INPUT_H});
        campo_attivo_idx=-1;
    }
    if (*cerca_att) {
        int k=GetCharPressed();
        while (k>0) {
            int n=strlen(cerca);
            if (k >= 32 && n < cerca_size - 1) {
                cerca[n] = k;
                cerca[n + 1] = '\0';
            }
            k=GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int n = strlen(cerca);
            if (n > 0) cerca[n - 1] = '\0';
        }
        if (IsKeyPressed(KEY_ENTER)) premuto=1;
    }
    if (bottone_accent(x0+SC(350),y,SC(120),INPUT_H,"Cerca")||premuto) {
        if (strlen(cerca)>0) {
            *id_trovato=app_cerca_cliente_id_per_nome(cerca,nome_trovato,nome_size);
            premuto=1;
        }
    }
    y+=INPUT_H+SC(8);
    if (*id_trovato>0) {
        char info[128];
        snprintf(info, sizeof(info), "Cliente trovato: %s", nome_trovato);
        DrawText(info,x0,y,SC(12),COL_ACCENT);
    } else if (strlen(cerca)>0&&nome_trovato[0]=='\0') {
        DrawText("Nessun cliente trovato.",x0,y,SC(12),COL_DANGER);
    }
    y+=SC(22);
    if (y_out)*y_out=y;
    return premuto;
}

/* AREA TRAINER - Gestione: ricerca cliente e generazione scheda/piano. */
void schermata_trainer(AppState *s)
{
    disegna_header("Area Trainer", s->username);
    int x0=SIDEBAR_W+PADDING;

    int y=TITOLO_H+SC(16);
    DrawText("1. Trova il cliente",x0,y,SC(14),COL_TEXT_DIM); y+=SC(22);

    int y_dopo=y;
    cerca_cliente_ui(s,
        s->trainer_gestione_cerca, sizeof(s->trainer_gestione_cerca),
        &s->trainer_gestione_cerca_att,
        s->trainer_gestione_nome, sizeof(s->trainer_gestione_nome),
        &s->trainer_gestione_id, &y_dopo);
    y=y_dopo+SC(10);
    s->id_cliente_sel = s->trainer_gestione_id;

/* SCHEDA */
    DrawLine(x0,y,FINESTRA_W-PADDING,y,COL_BORDER); y+=SC(14);
    DrawText("2a. Genera scheda allenamento (sostituisce la precedente)",
             x0,y,SC(14),COL_TEXT_DIM); y+=SC(24);

    const char *var_lbl[]={"Variante 1 - Base","Variante 2 - Intensivo","Variante 3 - Avanzato"};
    for (int i = 0; i < 3; i++) {
        Color sf=(s->trainer_gestione_var_scheda==i)?COL_ACCENT_DIM:COL_SURFACE;
        Color ct=(s->trainer_gestione_var_scheda==i)?COL_ACCENT:COL_TEXT;
        if (bottone(x0+i*SC(188),y,SC(178),SC(36),var_lbl[i],sf,ct))
            s->trainer_gestione_var_scheda=i;
    }
    y+=SC(48);

    if (bottone_accent(x0,y,SC(280),SC(42),"Genera e sostituisci scheda")) {
        int id_cl = s->trainer_gestione_id;
        if (id_cl>0) {
            Utente u_tmp;
            memset(&u_tmp, 0, sizeof(u_tmp));
            if (prepara_utente_db(id_cl,&u_tmp)) {
                ProgrammaAllenamento prog=
                    costruisci_programma_ex(&u_tmp,s->trainer_gestione_var_scheda);
                if (app_sostituisci_scheda_ex(id_cl,
                        s->trainer_gestione_var_scheda,prog.nome)>0) {
                    imposta_ok(s,"Scheda generata e sostituita!");
                    s->trainer_gestione_id=0;
                    s->trainer_gestione_nome[0]='\0';
                    memset(s->trainer_gestione_cerca, 0, sizeof(s->trainer_gestione_cerca));
                    s->id_cliente_sel=0;
                } else imposta_errore(s,"Errore salvataggio scheda.");
            } else imposta_errore(s,"Dati cliente non trovati.");
        } else imposta_errore(s,"Cerca prima un cliente valido.");
    }
    y+=SC(62);

/* PIANO */
    DrawLine(x0,y,FINESTRA_W-PADDING,y,COL_BORDER); y+=SC(14);
    DrawText("2b. Genera piano alimentare (sostituisce il precedente)",
             x0,y,SC(14),COL_TEXT_DIM); y+=SC(24);

    for (int i = 0; i < 3; i++) {
        Color sf=(s->trainer_gestione_var_piano==i)?COL_ACCENT_DIM:COL_SURFACE;
        Color ct=(s->trainer_gestione_var_piano==i)?COL_ACCENT:COL_TEXT;
        if (bottone(x0+i*SC(188),y,SC(178),SC(36),var_lbl[i],sf,ct))
            s->trainer_gestione_var_piano=i;
    }
    y+=SC(48);

    if (bottone_accent(x0,y,SC(280),SC(42),"Genera e sostituisci piano")) {
        int id_cl = s->trainer_gestione_id;
        if (id_cl>0) {
            Utente u_tmp;
            memset(&u_tmp, 0, sizeof(u_tmp));
            if (prepara_utente_db(id_cl,&u_tmp)) {
                PianoNutrizionale pn=
                    costruisci_piano_ex(&u_tmp,s->trainer_gestione_var_piano);
                if (app_sostituisci_piano_ex(id_cl,s->trainer_gestione_var_piano,
                                             u_tmp.kcal_obiettivo,pn.etichetta)) {
                    imposta_ok(s,"Piano generato e sostituito!");
                    s->trainer_gestione_id=0;
                    s->trainer_gestione_nome[0]='\0';
                    memset(s->trainer_gestione_cerca, 0, sizeof(s->trainer_gestione_cerca));
                    s->id_cliente_sel=0;
                } else imposta_errore(s,"Errore salvataggio piano.");
            } else imposta_errore(s,"Dati cliente non trovati.");
        } else imposta_errore(s,"Cerca prima un cliente valido.");
    }
}

/* Template: Schede / Storico / Piani cliente (trainer)
 *  idx: 0=schede, 1=storico, 2=piani - stato in AppState->trainer_* */
static void schermata_trainer_ricerca(AppState *s,
    const char *titolo_h, const char *sub_h, int idx,
    void (*carica_fn)(int, char*, int))
{
    disegna_header(titolo_h, sub_h);

    int y_dopo=0;
    int trovato_ora=cerca_cliente_ui(s,
        s->trainer_cerca[idx], sizeof(s->trainer_cerca[idx]),
        &s->trainer_cerca_att[idx],
        s->trainer_nome_trovato[idx], sizeof(s->trainer_nome_trovato[idx]),
        &s->trainer_id_trovato[idx], &y_dopo);

    if (trovato_ora) {
        memset(s->testo, 0, sizeof(s->testo));
        if (s->trainer_id_trovato[idx]>0 && carica_fn)
            carica_fn(s->trainer_id_trovato[idx], s->testo, sizeof(s->testo));
        else
            strncpy(s->testo,"Nessun cliente trovato.",sizeof(s->testo)-1);
        s->trainer_caricato[idx]=1;
        s->scroll_y=0;
        s->ultima_caricata=SCHERMATA_BENVENUTO;
    }

    if (s->trainer_caricato[idx]) {
        if (strlen(s->testo)==0)
            DrawText("Nessun dato disponibile.",
                     SIDEBAR_W+PADDING,y_dopo+SC(8),SC(14),COL_TEXT_DIM);
        else
            schermata_testo_inline(s,y_dopo);
    }
}

void schermata_trainer_schede(AppState *s) {
    schermata_trainer_ricerca(s,"Schede cliente",
        "Cerca per nome o username", 0, carica_schede_cliente);
}
void schermata_trainer_storico(AppState *s) {
    schermata_trainer_ricerca(s,"Storico cliente",
        "Cerca per nome o username", 1, carica_storico_dinamico_in_buffer);
}
void schermata_trainer_piani(AppState *s) {
    schermata_trainer_ricerca(s,"Piani cliente",
        "Cerca per nome o username", 2, carica_piani_cliente);
}