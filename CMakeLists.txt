cmake_minimum_required(VERSION 3.10)
project(New_Fitness C)

set(CMAKE_C_STANDARD 11)

add_executable(New_Fitness
    main_logic.c
        client_logica.c
        client_ui.c
        client_api.c
    client_socket.c
)

target_link_libraries(New_Fitness m)
