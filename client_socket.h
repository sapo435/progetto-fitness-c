#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

/*
 * client_socket.h
 * ---------------
 * Modulo di comunicazione col server via socket TCP.
 * Ogni chiamata apre una connessione, invia un comando,
 * riceve la risposta e chiude il socket.
 */

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 4096

/*
 * Invia un comando al server e riceve la risposta.
 *   cmd           - stringa del comando (es. "GET_UTENTI")
 *   risposta      - buffer dove scrivere la risposta del server
 *   risposta_size - dimensione del buffer
 * Ritorna 1 se ok, 0 se errore di connessione.
 */
int invia_comando(const char *cmd, char *risposta, int risposta_size);

#endif /* CLIENT_SOCKET_H */
