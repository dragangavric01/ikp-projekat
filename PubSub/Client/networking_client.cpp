#include "networking_client.h"

// Prepares the client socket
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

// Attempts to establish a TCP connection with the broker by doing a TCP handshake
bool connect_to_broker(SOCKET client_socket, sockaddr_in* broker_data_ptr, bool* connected_ptr) {
    if (connect(client_socket, (SOCKADDR*)broker_data_ptr, sizeof(*broker_data_ptr)) == SOCKET_ERROR) {
        printf("Unable to connect to server.\n");
        return false;
    }

    *connected_ptr = true;

    return true;
}

// Sends a command to the broker
bool send_command(SOCKET client_socket, char* command) {
    int result = send(client_socket, command, (int)strlen(command) + 1, 0);

    if (result == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        return false;
    }

    return true;
}

// Multiplexing
static int read_check(SOCKET socket) {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(socket, &read_set);

    // Wsaiting period is 0.00 seconds, so this would be polling, not multiplexing. 
    timeval waiting_period;
    waiting_period.tv_sec = 0;
    waiting_period.tv_usec = 0;

    int result = select(0, &read_set, NULL, NULL, &waiting_period);
    if (result == SOCKET_ERROR) {
        return SELECT_ERROR;
    }

    if (FD_ISSET(socket, &read_set)) {
        return WONT_BLOCK;
    } else {
        return WILL_BLOCK;
    }
}

// Receives one or more messages from broker
bool receive_from_broker(SOCKET client_socket, char receive_buffer[], bool wait_until_received) {
    if (!wait_until_received) {
        int result = read_check(client_socket);
        if (result == SELECT_ERROR) {
            printf("Select failed with error: %d\n", WSAGetLastError());
            return false;
        } else if (result == WILL_BLOCK) {
            return true;
        }
    }

    int result = recv(client_socket, receive_buffer, CLIENT_RECEIVE_BUFFER_SIZE, 0);
    if (result == 0) {
        printf("Connection with server closed.\n");
        closesocket(client_socket);
    } else if (result < 0) {
        printf("recv failed with error: %d\n", WSAGetLastError());
    }

    return false;
}
