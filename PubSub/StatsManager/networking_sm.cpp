#include "networking_sm.h"

// Multiplexing
static int read_check(SOCKET socket) {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(socket, &read_set);

    // If waiting period was 0.00 seconds, then this would be polling, not multiplexing. The program would then needlessly waste CPU time when there are not many commands to receive and connections to accept, because it would be constantly checking if there are any.
    timeval waiting_period;
    waiting_period.tv_sec = 0;
    waiting_period.tv_usec = 200000;  // 0.2 seconds

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


// Prepares the welcoming socket
void setup(SOCKET* welcoming_socket_ptr) {
    int result;

    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        exit(1);
    }

    addrinfo* resulting_address = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    char port_number[6];
    _itoa(STATS_MANAGER_PORT_NUMBER, port_number, 10);
    result = getaddrinfo(NULL, port_number, &hints, &resulting_address);
    if (result != 0) {
        printf("getaddrinfo failed with error: %d\n", result);

        WSACleanup();
        exit(1);
    }

    *welcoming_socket_ptr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*welcoming_socket_ptr == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());

        freeaddrinfo(resulting_address);
        WSACleanup();
        exit(1);
    }

    result = bind(*welcoming_socket_ptr, resulting_address->ai_addr, (int)resulting_address->ai_addrlen);
    if (result == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());

        freeaddrinfo(resulting_address);
        closesocket(*welcoming_socket_ptr);
        WSACleanup();
        exit(1);
    }

    freeaddrinfo(resulting_address);

    unsigned long  mode = 1;
    if (ioctlsocket(*welcoming_socket_ptr, FIONBIO, &mode) != 0) {
        printf("ioctlsocket failed with error.");

        closesocket(*welcoming_socket_ptr);
        WSACleanup();
        exit(1);
    }

    result = listen(*welcoming_socket_ptr, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());

        closesocket(*welcoming_socket_ptr);
        WSACleanup();
        exit(1);
    }
}

int accept_connection(SOCKET welcoming_socket, SOCKET* connection_socket_ptr) {
    int result = read_check(welcoming_socket);
    if (result == SELECT_ERROR) {
        printf("Select failed with error: %d\n", WSAGetLastError());
        closesocket(welcoming_socket);
        WSACleanup();
        return ERROR;
    } else if (result == WILL_BLOCK) {
        return WILL_BLOCK;
    }

    *connection_socket_ptr = accept(welcoming_socket, NULL, NULL);
    if (*connection_socket_ptr == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(welcoming_socket);
        WSACleanup();
        return ERROR;
    }

    unsigned long  mode = 1;
    if (ioctlsocket(*connection_socket_ptr, FIONBIO, &mode) != 0) {
        printf("ioctlsocket failed with error.");
        closesocket(welcoming_socket);
        WSACleanup();
        return ERROR;
    }

    return CONNECTED;
}


bool receive_notifications(SOCKET welcoming_socket, SOCKET connection_socket, char receive_buffer[]) {
    int result = read_check(connection_socket);
    if (result == SELECT_ERROR) {
        printf("Select failed with error: %d\n", WSAGetLastError());

        return false;
    } else if (result == WILL_BLOCK) {
        return false;
    }

    result = recv(connection_socket, receive_buffer, RECEIVE_BUFFER_SIZE, 0);
    if (result > 0) {
        return true;
    } else if (result == 0) {
        printf("Connection with client closed.\n");
        closesocket(welcoming_socket);
        closesocket(connection_socket);
        WSACleanup();
        exit(16);
    } else {
        printf("recv failed with error: %d\n", WSAGetLastError());
        return false;
    }
}
