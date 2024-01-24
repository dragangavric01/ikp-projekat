#include "networking_broker.h"

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

// For making nonblocking socket behave as if it was blocking
static void wait_until_nonblocking(SOCKET socket) {
    fd_set write_set;
    FD_ZERO(&write_set);
    FD_SET(socket, &write_set);

    int result = select(0, NULL, &write_set, NULL, NULL);
    if (result == SOCKET_ERROR) {
        EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
        printf("Select failed with error: %d\n", WSAGetLastError());
        LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
    }
}

// Prepares the welcoming socket
void setup(SOCKET* welcoming_socket_ptr) {
    int result;

    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        // No need for mutual exlusive printf here because threads haven't been started yet
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

    result = getaddrinfo(NULL, PORT_NUMBER, &hints, &resulting_address);
    if (result != 0) {
        // No need for mutual exlusive printf here because threads haven't been started yet
        printf("getaddrinfo failed with error: %d\n", result);

        WSACleanup();
        exit(1);
    }

    *welcoming_socket_ptr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*welcoming_socket_ptr == INVALID_SOCKET) {
        // No need for mutual exlusive printf here because threads haven't been started yet
        printf("socket failed with error: %ld\n", WSAGetLastError());

        freeaddrinfo(resulting_address);
        WSACleanup();
        exit(1);
    }

    result = bind(*welcoming_socket_ptr, resulting_address->ai_addr, (int)resulting_address->ai_addrlen);
    if (result == SOCKET_ERROR) {
        // No need for mutual exlusive printf here because threads haven't been started yet
        printf("bind failed with error: %d\n", WSAGetLastError());

        freeaddrinfo(resulting_address);
        closesocket(*welcoming_socket_ptr);
        WSACleanup();
        exit(1);
    }

    freeaddrinfo(resulting_address);

    unsigned long  mode = 1;
    if (ioctlsocket(*welcoming_socket_ptr, FIONBIO, &mode) != 0) {
        // No need for mutual exlusive printf here because threads haven't been started yet
        printf("ioctlsocket failed with error.");

        closesocket(*welcoming_socket_ptr);
        WSACleanup();
        exit(1);
    }

    result = listen(*welcoming_socket_ptr, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        // No need for mutual exlusive printf here because threads haven't been started yet
        printf("listen failed with error: %d\n", WSAGetLastError());

        closesocket(*welcoming_socket_ptr);
        WSACleanup();
        exit(1);
    }
}

// Does the TCP handshake with a client that wants to connect (if there is such client) and adds the new connection socket to connection_sockets
void accept_connection(SOCKET welcoming_socket, Topic topics[], int number_of_topics, SocketList* connection_sockets_ptr) {
    if ((*connection_sockets_ptr).size > CONNECTION_SOCKET_LIST_MAX_SIZE) {
        return;
    }

    int result = read_check(welcoming_socket);
    if (result == SELECT_ERROR) {
        EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
        printf("Select failed with error: %d\n", WSAGetLastError());
        LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));

        return;
    } else if (result == WILL_BLOCK) {
        return;
    }

    SOCKET* connection_socket_ptr = (SOCKET*)malloc(sizeof(SOCKET));  // I have to dynamically allocate it because SocketListNode keeps a pointer to the socket and if it the SOCKET variable was a local variable, it would go out of scope and the pointer in SocketListNode would stop being valid
    *connection_socket_ptr = accept(welcoming_socket, NULL, NULL);
    if (*connection_socket_ptr == INVALID_SOCKET) {
        EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
        printf("accept failed with error: %d\n", WSAGetLastError());
        LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
        
        return;
    }

    unsigned long  mode = 1;
    if (ioctlsocket(*connection_socket_ptr, FIONBIO, &mode) != 0) {
        EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
        printf("ioctlsocket failed with error.");
        LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
        
        return;
    }

    add_to_start(connection_sockets_ptr, connection_socket_ptr);
    
    EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
    printf("Socket %llu has been added to connection sockets\n", *connection_socket_ptr);
    printf("Connection sockets: ");
    print_socket_list_unsafe(connection_sockets_ptr);
    LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
}

// Receives one or more commands if there are commands to receive
int receive_commands(SOCKET welcoming_socket, SOCKET* connection_socket_ptr, Topic topics[], int number_of_topics, char receive_buffer[], SocketList* connection_sockets_ptr, SocketListNode** ptr_to_walker) {
    int result = read_check(*connection_socket_ptr);
    if (result == SELECT_ERROR) {
        EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
        printf("Select failed with error: %d\n", WSAGetLastError());
        LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
        
        return NOT_RECEIVED;
    } else if (result == WILL_BLOCK) {
        return NOT_RECEIVED;
    }
    
    result = recv(*connection_socket_ptr, receive_buffer, BROKER_RECEIVE_BUFFER_SIZE, 0);
    if (result > 0) {
        return RECEIVED;
    } else if (result == 0) {
        EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
        printf("Connection with client closed.\n");
        LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));

        closesocket(*connection_socket_ptr);
        *connection_socket_ptr = INVALID_SOCKET;   // So I know it has been closed when I see it in lists

        *ptr_to_walker = (**ptr_to_walker).next; 
        free_node(connection_sockets_ptr, connection_socket_ptr);
        for (int i = 0; i < number_of_topics; i++) {
            free_node(topics[i].subscriber_connection_sockets_ptr, connection_socket_ptr);
        }

        return DELETED;
    } else {
        EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
        printf("recv failed with error: %d\n", WSAGetLastError());
        LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
        
        return NOT_RECEIVED;
    }
}

// Sends a message to a client
void send_to_client(SOCKET connection_socket, char* message) {
    // I want send() to be able to wait until there is free space in the socket output buffer. Since the socket is set to nonblocking with ioctlsocket(), I have to make it behave as if it is blocking by calling select() and passing NULL instead of timeval struct instance.
    wait_until_nonblocking(connection_socket);
    
    if (send(connection_socket, message, (int)strlen(message) + 1, 0) == SOCKET_ERROR) {
        EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
        printf("send failed with error: %d\n", WSAGetLastError());
        LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
    }
}
