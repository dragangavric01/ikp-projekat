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
#define SERVER_PORT 27016
#define BUFFER_SIZE 256

void publisher(SOCKET connectSocket);
void subscriber(SOCKET connectSocket);

int main() {
    SOCKET connectSocket = INVALID_SOCKET;
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    serverAddress.sin_port = htons(SERVER_PORT);

    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("Unable to connect to broker.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to broker.\n");

    while (true) {
        int choice;

        printf("Menu:\n");
        printf("1. Publisher\n");
        printf("2. Subscriber\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");

        scanf_s("%d", &choice);

        if (choice == 1) {
            publisher(connectSocket);
        }
        else if (choice == 2) {
            subscriber(connectSocket);
        }
        else if (choice == 3) {
            shutdown(connectSocket, SD_BOTH);
            closesocket(connectSocket);
            WSACleanup();
            return 0;
        }
        else {
            printf("Invalid choice. Please enter either 1, 2 or 3.\n");
        }
    }
    shutdown(connectSocket, SD_BOTH);
    closesocket(connectSocket);
    WSACleanup();
	return 0;
}

void publisher(SOCKET connectSocket) {
    char dataBuffer[BUFFER_SIZE];

        printf("\nEnter a message to send to the broker: ");
        if (scanf_s(" %[^\n]", dataBuffer, BUFFER_SIZE) != 1) {
            printf("Error reading input.\n");
            closesocket(connectSocket);
            WSACleanup();
        }

        if (strlen(dataBuffer) > 0) {
            int iResult = send(connectSocket, dataBuffer, strlen(dataBuffer), 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(connectSocket);
                WSACleanup();
            }
            printf("Message successfully sent.\n");
        }
        else {
            printf("No message entered.\n");
        }
}


void subscriber(SOCKET connectSocket) {
    int iResult;
    char dataBuffer[BUFFER_SIZE];
    do {
        iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);
        if (iResult > 0) {
            dataBuffer[iResult] = '\0';
            printf("Message received from broker: %s\n", dataBuffer);
            break;
        }
        else if (iResult == 0) {
            printf("Connection with broker closed.\n");
            break;
        }
        else {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                // No data available to receive
            }
            else {
                printf("recv failed with error: %d\n", WSAGetLastError());
                break;
            }
        }

        Sleep(1000);

    } while (true);
}