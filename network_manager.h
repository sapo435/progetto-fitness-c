#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H
#include "sqlite3.h"
#ifdef _WIN32
    #include <winsock2.h>
#else
    typedef int SOCKET;
#endif

void avvia_server(sqlite3 *db,int porta);
void gestisci_client(SOCKET  client_socket, sqlite3 *db);

#endif