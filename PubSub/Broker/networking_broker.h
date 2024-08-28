#pragma once

#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "socket_list.h"
#include "command_execution.h"
#include "common.h"
#include "message_queue.h"
#include "global.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma pack(1)  


#define PORT_NUMBER "16000"

#define SELECT_ERROR 0
#define WILL_BLOCK 1
#define WONT_BLOCK 2

#define RECEIVED 0
#define NOT_RECEIVED 1
#define DELETED 2


void setup(SOCKET* welcoming_socket_ptr, sockaddr_in* sm_data_ptr, SOCKET* sm_client_socket_ptr);

void accept_connection(SOCKET welcoming_socket, Topic topics[], int number_of_topics, SocketList* connection_sockets_ptr);

bool connect_to_sm(SOCKET sm_client_socket, sockaddr_in* sm_data_ptr);

int receive_commands(SOCKET welcoming_socket, SOCKET* connection_socket_ptr, Topic topics[], int number_of_topics, char receive_buffer[], SocketList* connection_sockets_ptr, SocketListNode** ptr_to_walker);

void send_to_client(SOCKET connection_socket, char* message);

void send_to_stats_manager(SOCKET sm_client_socket, char* message);