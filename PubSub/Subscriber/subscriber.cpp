#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT 20000
#define BUFFER_SIZE 256

int main() {
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET acceptedSocket = INVALID_SOCKET;
    int iResult;
    char dataBuffer[BUFFER_SIZE];
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    sockaddr_in serverAddress;
    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("Subscriber waiting for broker to connect...\n");

    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(struct sockaddr_in);

    acceptedSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (acceptedSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("\nBroker connection accepted. Address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    unsigned long mode = 1; //non-blocking mode
    iResult = ioctlsocket(acceptedSocket, FIONBIO, &mode);
    if (iResult != NO_ERROR)
        printf("ioctlsocket failed with error: %ld\n", iResult);

    do {
        iResult = recv(acceptedSocket, dataBuffer, BUFFER_SIZE, 0);
        if (iResult > 0) {
            dataBuffer[iResult] = '\0';
            printf("Message received from broker : %s\n", dataBuffer);
        }
        else if (iResult == 0) {
            printf("Connection with broker closed.\n");
            break;
        }
        else {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
            }
            else {
                printf("recv failed with error: %d\n", WSAGetLastError());
                break;
            }
        }

        Sleep(1000);

    } while (true);

    closesocket(listenSocket);
    closesocket(acceptedSocket);
    WSACleanup();

    return 0;
}
