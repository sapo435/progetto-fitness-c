cmake_minimum_required(VERSION 3.16)
project(progetto_fitness_c C)
set(CMAKE_C_STANDARD 11)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
add_subdirectory(raylib-src)

add_executable(newfitness
        # --- entry point ---
        gui_main.c

        # --- GUI: widget generici (bottoni, campi, sidebar) ---
        gui_widgets.c

        # --- GUI: schermate, divise per area ---
        gui_schermate_auth.c       # benvenuto, login, registrazione
        gui_schermate_cliente.c    # home, report, feedback
        gui_schermate_trainer.c    # gestione, ricerca cliente

        # --- logica applicativa, divisa per dominio ---
        client_metriche.c          # BMI, BMR, TDEE, macronutrienti
        client_programmi.c         # programmi di allenamento (+ varianti)
        client_piani.c             # piani nutrizionali (+ varianti)
        client_storico.c           # storico pesi

        # --- accesso dati ---
        app_db.c                   # SQLite diretto
        lista_dinamica.c           # strutture dati dinamiche (lista/array)

        # --- libreria SQLite ---
        sqlite3.c
)

target_include_directories(newfitness PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/raylib-src/src
)

target_link_libraries(newfitness PRIVATE
        raylib opengl32 gdi32 winmm
)