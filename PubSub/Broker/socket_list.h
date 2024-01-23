#pragma once

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "global.h"

#define SUBSCRIBER_LIST_MAX_SIZE 99999
#define CONNECTION_SOCKET_LIST_MAX_SIZE 99999


typedef struct SocketListNodeStruct {
    SOCKET* socket_ptr;  // It's a pointer because there are multiple SocketLists and when I want to close all sockets in all those lists, if one socket is in two lists I would call closesocket() twice. But if it is a pointer, I can set it's value to INVALID_SOCKET after calling closesocket() and in every list it will have that value, so I know it has been closed
    struct SocketListNodeStruct* next;
} SocketListNode;

typedef struct SocketList {
    SocketListNode* head;  // pointer to the first element
    int size;
    CRITICAL_SECTION* crit_section_ptr;
} SocketList;


void initialize_socket_list(SocketList* ptr_to_list, bool mutually_exclusive);

void add_to_start(SocketList* ptr_to_list, SOCKET* socket_ptr);

void free_node(SocketList* ptr_to_list, SOCKET* socket_ptr);

void close_sockets_and_free_socket_list(SocketList* ptr_to_list);

void print_socket_list(SocketList* ptr_to_list);
