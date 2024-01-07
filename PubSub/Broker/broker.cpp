#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

#define SERVER_PORT 27016
#define BUFFER_SIZE 256

int main() {
    WSADATA wsaData;
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSockets[FD_SETSIZE];
    int clientCount = 0;
    char dataBuffer[BUFFER_SIZE];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    // Create a SOCKET for listening for incoming connection requests
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    if (bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("Broker is now running. Waiting for clients...\n");

    fd_set readfds;
    int maxSocketDescriptor = listenSocket;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(listenSocket, &readfds);

        for (int i = 0; i < clientCount; ++i) {
            FD_SET(clientSockets[i], &readfds);
        }

        int activity = select(0, &readfds, NULL, NULL, NULL);
        if (activity == SOCKET_ERROR) {
            printf("select error\n");
            break;
        }

        if (FD_ISSET(listenSocket, &readfds)) {
            SOCKET newSocket = accept(listenSocket, NULL, NULL);
            if (newSocket == INVALID_SOCKET) {
                printf("accept failed with error: %d\n", WSAGetLastError());
            }
            else {
                printf("New client connected\n");
                clientSockets[clientCount++] = newSocket;
                unsigned long mode = 1; // Set the new socket to non-blocking
                ioctlsocket(newSocket, FIONBIO, &mode);
            }
        }

        for (int i = 0; i < clientCount; ++i) {
            if (FD_ISSET(clientSockets[i], &readfds)) {
                int bytesRead = recv(clientSockets[i], dataBuffer, BUFFER_SIZE, 0);
                if (bytesRead == SOCKET_ERROR) {
                    int error = WSAGetLastError();
                    if (error != WSAEWOULDBLOCK) {
                        printf("Error in recv on socket %d: %d\n", clientSockets[i], error);
                    }
                }
                else if (bytesRead == 0) {
                    printf("Client disconnected\n");
                    closesocket(clientSockets[i]);
                    if (i < clientCount - 1) {
                        clientSockets[i] = clientSockets[clientCount - 1];
                    }
                    --clientCount;
                }
                else {
                    dataBuffer[bytesRead] = '\0';
                    printf("Message from client: %s\n", dataBuffer);
                    for (int j = 0; j < clientCount; ++j) {
                        if (i != j) {
                            send(clientSockets[j], dataBuffer, bytesRead, 0);
                        }
                    }
                }
            }
        }
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
