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

#define SERVER_PORT 16050
#define BUFFER_SIZE 256
#define SUBSCRIBER_IP_ADDRESS "127.0.0.1"
#define SUBSCRIBER_PORT 20000

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

    // Creating a socket for the subscriber
    SOCKET secondServerSocket = INVALID_SOCKET;
    sockaddr_in secondServerAddress;
    secondServerAddress.sin_family = AF_INET;
    secondServerAddress.sin_addr.s_addr = inet_addr(SUBSCRIBER_IP_ADDRESS);
    secondServerAddress.sin_port = htons(SUBSCRIBER_PORT);

    secondServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (secondServerSocket == INVALID_SOCKET) {
        printf("socket for subscriber failed with error: %ld\n", WSAGetLastError());
        closesocket(acceptedSocket);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (connect(secondServerSocket, (SOCKADDR*)&secondServerAddress, sizeof(secondServerAddress)) == SOCKET_ERROR) {
        printf("Unable to connect to subscriber.\n");
        closesocket(secondServerSocket);
        closesocket(acceptedSocket);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to subscriber.\n");

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

    printf("Broker waiting for incoming publisher connection.\n");

    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(struct sockaddr_in);

    acceptedSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (acceptedSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("\nPublisher connection accepted. Address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    do {
        iResult = recv(acceptedSocket, dataBuffer, BUFFER_SIZE, 0);
        if (iResult > 0) {
            dataBuffer[iResult] = '\0';
            printf("Message received from publisher: %s\n", dataBuffer);

            // Sending the received message to the subscriber
            int sendResult = send(secondServerSocket, dataBuffer, strlen(dataBuffer), 0);
            if (sendResult == SOCKET_ERROR) {
                printf("Failed to send message to the subscriber: %d\n", WSAGetLastError());
                break;
            }
            printf("Message sent to the subscriber.\n");
        }
        else if (iResult == 0) {
            printf("Connection with publisher closed.\n");
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

    closesocket(secondServerSocket);
    closesocket(acceptedSocket);
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}