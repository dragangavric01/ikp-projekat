#pragma once

#include <winsock2.h>

#define SUBSCRIBER_LIST_MAX_SIZE 99999
#define CONNECTION_SOCKET_LIST_MAX_SIZE 99999

typedef struct SocketListNodeStruct {
    SOCKET socket;
    struct SocketListNodeStruct* next;
} SocketListNode;

typedef struct SocketList {
    SocketListNode* head;  // pointer to the first element
    int size;
} SocketList;


void add_to_start(SocketList* ptr_to_list, SOCKET socket);

void delete_socket(SocketList* ptr_to_list, SOCKET socket);

void close_sockets_and_free_socket_list(SocketList socket_list);

void print_socket_list(SocketList socket_list);