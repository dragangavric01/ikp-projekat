#include "networking_client.h"

bool setup(sockaddr_in* broker_data_ptr, SOCKET* client_socket_ptr) {
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }

    *client_socket_ptr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*client_socket_ptr == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();

        return false;
    }

    (*broker_data_ptr).sin_family = AF_INET;
    (*broker_data_ptr).sin_addr.s_addr = inet_addr(BROKER_IP_ADDRESS);
    (*broker_data_ptr).sin_port = htons(BROKER_PORT_NUMBER);

    return true;
}

void cleanup(SOCKET client_socket) {
    closesocket(client_socket);
    WSACleanup();
}

bool connect_to_broker(SOCKET client_socket, sockaddr_in* broker_data_ptr, bool* connected_ptr) {
    if (connect(client_socket, (SOCKADDR*)broker_data_ptr, sizeof(*broker_data_ptr)) == SOCKET_ERROR) {
        printf("Unable to connect to server.\n");
        return false;
    }

    *connected_ptr = true;

    return true;
}

bool send_command(SOCKET client_socket, char* command) {
    int result = send(client_socket, command, (int)strlen(command) + 1, 0);

    if (result == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        return false;
    }

    return true;
}

void receive_from_broker(SOCKET client_socket, char receive_buffer[]) {
    int result = recv(client_socket, receive_buffer, MAX_MESSAGE_SIZE, 0);
    if (result == 0) {
        printf("Connection with server closed.\n");
        closesocket(client_socket);
    } else if (result < 0) {
        printf("recv failed with error: %d\n", WSAGetLastError());
    }
}
