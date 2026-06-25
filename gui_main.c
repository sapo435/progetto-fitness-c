/*
 * gui_main.c
 * ----------
 * Punto di ingresso dell'applicazione New Fitness (GUI Raylib).
 * Contiene solo il main() con il game loop e il dispatch
 * delle schermate tramite switch.
 *
 * Compilazione Windows (MinGW):
 *   gcc gui_main.c gui_widgets.c gui_schermate.c
 *       client_logica.c client_api.c client_socket.c
 *       -o newfitness.exe -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32
 *       -I./raylib/include -L./raylib/lib
 */

#include "gui_core.h"

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(FINESTRA_W, FINESTRA_H, "New Fitness");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL); /* disabilita chiusura con ESC */

    AppState stato;
    memset(&stato, 0, sizeof(stato));
    stato.schermata = SCHERMATA_BENVENUTO;

    while (!WindowShouldClose()) {

        BeginDrawing();
        ClearBackground(COL_BG);

        /* Sidebar: visibile solo dopo il login */
        if (stato.token[0] != '\0')
            disegna_sidebar(&stato);

        /* Dispatch alla schermata corrente */
        switch (stato.schermata) {

            case SCHERMATA_BENVENUTO:
                schermata_benvenuto(&stato);
                break;

            case SCHERMATA_LOGIN:
                schermata_login(&stato);
                break;

            case SCHERMATA_REGISTRA:
                schermata_registra(&stato);
                break;

            case SCHERMATA_HOME:
                schermata_home(&stato);
                break;

            case SCHERMATA_REPORT:
                schermata_report(&stato);
                break;

            case SCHERMATA_FEEDBACK:
                schermata_feedback(&stato);
                break;

            /* --- Schermate con caricamento dati dal server --- */

            case SCHERMATA_STORICO: {
                static int caricato = 0;
                if (!caricato) {
                    mostra_storico_buf(stato.token, stato.id_server,
                                      stato.testo_server,
                                      sizeof(stato.testo_server));
                    caricato = 1;
                }
                schermata_testo_server(&stato,
                                       "Storico pesi",
                                       "Andamento del tuo peso");
                if (stato.schermata != SCHERMATA_STORICO) caricato = 0;
                break;
            }

            case SCHERMATA_SCHEDE: {
                static int caricato = 0;
                if (!caricato) {
                    mostra_schede_buf(stato.token, stato.id_server,
                                     stato.testo_server,
                                     sizeof(stato.testo_server));
                    caricato = 1;
                }
                schermata_testo_server(&stato,
                                       "Schede allenamento",
                                       "Le tue schede");
                if (stato.schermata != SCHERMATA_SCHEDE) caricato = 0;
                break;
            }

            case SCHERMATA_PIANI: {
                static int caricato = 0;
                if (!caricato) {
                    mostra_piani_buf(stato.token, stato.id_server,
                                    stato.testo_server,
                                    sizeof(stato.testo_server));
                    caricato = 1;
                }
                schermata_testo_server(&stato,
                                       "Piani alimentari",
                                       "I tuoi piani");
                if (stato.schermata != SCHERMATA_PIANI) caricato = 0;
                break;
            }

            /* --- Area trainer --- */

            case SCHERMATA_TRAINER:
                schermata_trainer(&stato);
                break;

            case SCHERMATA_TRAINER_LISTA:
                schermata_trainer_lista(&stato);
                break;

            case SCHERMATA_TRAINER_SCHEDE:
                schermata_trainer_schede(&stato);
                break;

            case SCHERMATA_TRAINER_STORICO: {
                static int caricato = 0;
                if (!caricato) {
                    mostra_storico_buf(stato.token, 0,
                                      stato.testo_server,
                                      sizeof(stato.testo_server));
                    caricato = 1;
                }
                schermata_testo_server(&stato,
                                       "Storico pesi (trainer)",
                                       "Seleziona il cliente dalla sidebar");
                if (stato.schermata != SCHERMATA_TRAINER_STORICO) caricato = 0;
                break;
            }

            case SCHERMATA_TRAINER_PIANI: {
                static int caricato = 0;
                if (!caricato) {
                    mostra_piani_buf(stato.token, 0,
                                    stato.testo_server,
                                    sizeof(stato.testo_server));
                    caricato = 1;
                }
                schermata_testo_server(&stato,
                                       "Piani alimentari (trainer)",
                                       "");
                if (stato.schermata != SCHERMATA_TRAINER_PIANI) caricato = 0;
                break;
            }

            default:
                break;
        }

        /* Toast errore/successo sovrapposto */
        disegna_messaggi(&stato);

        EndDrawing();
    }

    /* Logout sicuro prima di chiudere */
    if (stato.token[0] != '\0')
        logout_server(stato.token);

    CloseWindow();
    return 0;
}
