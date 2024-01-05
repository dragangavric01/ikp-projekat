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

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 16050
#define BUFFER_SIZE 256

int main()
{
    SOCKET connectSocket = INVALID_SOCKET;
    WSADATA wsaData;
    char dataBuffer[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    serverAddress.sin_port = htons(SERVER_PORT);

    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to broker.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to broker.\n");

    while (true)
    {
        printf("\nEnter a message to send to the broker: ");
        gets_s(dataBuffer, BUFFER_SIZE);

        int iResult = send(connectSocket, dataBuffer, strlen(dataBuffer), 0);
        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            return 1;
        }

        printf("Message successfully sent.");

        printf("\nType quit to close program, or any other message to continue: ");
        gets_s(dataBuffer, BUFFER_SIZE);
        if (!strcmp(dataBuffer, "quit"))
            break;
    }

    shutdown(connectSocket, SD_BOTH);
    closesocket(connectSocket);
    WSACleanup();

    return 0;
}