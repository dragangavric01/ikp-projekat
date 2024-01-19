#include "networking_broker.h"


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

static void wait_until_nonblocking(SOCKET socket) {
    fd_set write_set;
    FD_ZERO(&write_set);
    FD_SET(socket, &write_set);

    int result = select(0, NULL, &write_set, NULL, NULL);
    if (result == SOCKET_ERROR) {
        printf("Select failed with error: %d\n", WSAGetLastError());
    }
}

bool setup(SOCKET* welcoming_socket_ptr) {
    int result;

    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }

    addrinfo* resulting_address = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_protocol = IPPROTO_TCP; 
    hints.ai_flags = AI_PASSIVE;      

    result = getaddrinfo(NULL, PORT_NUMBER, &hints, &resulting_address);
    if (result != 0) {
        printf("getaddrinfo failed with error: %d\n", result);
        WSACleanup();

        return false;
    }

    *welcoming_socket_ptr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*welcoming_socket_ptr == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resulting_address);
        WSACleanup();

        return false;
    }

    result = bind(*welcoming_socket_ptr, resulting_address->ai_addr, (int)resulting_address->ai_addrlen);
    if (result == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resulting_address);
        closesocket(*welcoming_socket_ptr);
        WSACleanup();

        return false;
    }

    freeaddrinfo(resulting_address);

    unsigned long  mode = 1;
    if (ioctlsocket(*welcoming_socket_ptr, FIONBIO, &mode) != 0) {
        printf("ioctlsocket failed with error.");
        closesocket(*welcoming_socket_ptr);
        WSACleanup();

        return false;
    }

    result = listen(*welcoming_socket_ptr, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(*welcoming_socket_ptr);
        WSACleanup();

        return false;
    }

    return true;
}

static void queue_and_lists_cleanup(Topic topics[], int number_of_topics, SocketList connection_sockets) {
    close_sockets_and_free_socket_list(connection_sockets);

    for (int i = 0; i < number_of_topics; i++) {
        free_message_queue(topics[i].message_queue);
        close_sockets_and_free_socket_list(topics[i].subscriber_connection_sockets);
    }
}

void cleanup(SOCKET welcoming_socket, Topic topics[], int number_of_topics, SocketList connection_sockets) {
    queue_and_lists_cleanup(topics, number_of_topics, connection_sockets);

    closesocket(welcoming_socket);
    WSACleanup();
}

bool accept_connection(SOCKET welcoming_socket, Topic topics[], int number_of_topics, SocketList* connection_sockets_ptr) {
    if ((*connection_sockets_ptr).size > CONNECTION_SOCKET_LIST_MAX_SIZE) {
        return true;
    }

    int result = read_check(welcoming_socket);
    if (result == SELECT_ERROR) {
        printf("Select failed with error: %d\n", WSAGetLastError());
        cleanup(welcoming_socket, topics, number_of_topics, *connection_sockets_ptr);

        return false;
    } else if (result == WILL_BLOCK) {
        return true;
    }

    SOCKET connection_socket = accept(welcoming_socket, NULL, NULL);
    if (connection_socket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        cleanup(welcoming_socket, topics, number_of_topics, *connection_sockets_ptr);

        return false;
    }

    unsigned long  mode = 1;
    if (ioctlsocket(connection_socket, FIONBIO, &mode) != 0) {
        printf("ioctlsocket failed with error.");
        cleanup(welcoming_socket, topics, number_of_topics, *connection_sockets_ptr);

        return false;
    }

    add_to_start(connection_sockets_ptr, connection_socket);
    printf("Socket %llu has been added to connection sockets\n", connection_socket);
    printf("Connection sockets: ");
    print_socket_list(*connection_sockets_ptr);

    return true;
}

bool receive_command(SOCKET welcoming_socket, SOCKET connection_socket, Topic topics[], int number_of_topics, char receive_buffer[], SocketList connection_sockets) {
    int result = read_check(connection_socket);
    if (result == SELECT_ERROR) {
        printf("Select failed with error: %d\n", WSAGetLastError());
        return false;
    } else if (result == WILL_BLOCK) {
        return false;
    }
    
    result = recv(connection_socket, receive_buffer, MAX_COMMAND_SIZE, 0);
    if (result > 0) {
        printf("Message received from client: %s.\n", receive_buffer);
        return true;
    } else if (result == 0) {
        printf("Connection with client closed.\n");
        closesocket(connection_socket);

        delete_socket(&connection_sockets, connection_socket);
        
        for (int i = 0; i < number_of_topics; i++) {
            delete_socket(&(topics[i].subscriber_connection_sockets), connection_socket);
        }

        return false;
    } else {
        printf("recv failed with error: %d\n", WSAGetLastError());
        return false;
    }
}

void send_to_client(SOCKET connection_socket, char* message) {
    // I want send() to be able to wait until there is free space in the socket output buffer. Since the socket is set to nonblocking with ioctlsocket(), I have to make it behave as if it is blocking by calling select() and passing NULL instead of timeval struct instance.
    wait_until_nonblocking(connection_socket);
    
    if (send(connection_socket, message, (int)strlen(message) + 1, 0) == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
    }
}
