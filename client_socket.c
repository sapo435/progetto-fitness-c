/*
 * client_socket.c
 * ---------------
 * Implementazione della comunicazione col server via socket TCP.
 * Compatibile Windows (Winsock2) e macOS/Linux (POSIX).
 */

#include <stdio.h>
#include <string.h>
#include "client_socket.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR   -1
    #define closesocket    close
#endif

int invia_comando(const char *cmd, char *risposta, int risposta_size)
{
    SOCKET sock;
    struct sockaddr_in server_addr;
    int bytes_ricevuti;

#ifdef _WIN32
    /* Inizializza Winsock (solo Windows) */
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "[SOCKET] Errore WSAStartup\n");
        return 0;
    }
#endif

    /* Crea il socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "[SOCKET] Errore creazione socket\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }

    /* Configura l'indirizzo del server */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    /* Connessione al server */
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "[SOCKET] Impossibile connettersi al server %s:%d\n",
                SERVER_IP, SERVER_PORT);
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }

    /* Invia il comando */
    send(sock, cmd, (int)strlen(cmd), 0);

    /* Ricevi la risposta */
    memset(risposta, 0, risposta_size);
    bytes_ricevuti = recv(sock, risposta, risposta_size - 1, 0);
    if (bytes_ricevuti <= 0) {
        fprintf(stderr, "[SOCKET] Nessuna risposta dal server\n");
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }
    risposta[bytes_ricevuti] = '\0';

    /* Chiudi il socket */
    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif

    return 1;
}
