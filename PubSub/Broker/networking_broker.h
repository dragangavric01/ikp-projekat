#pragma once

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "socket_list.h"
#include "message_queue.h"
#include "command_execution.h"
#include "common.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma pack(1)  // jer saljem strukturu, pa da bude i na klijentu i na serveru pack(1)

#define PORT_NUMBER "16000"
#define SELECT_ERROR 0
#define WILL_BLOCK 1
#define WONT_BLOCK 2

bool setup(SOCKET* welcoming_socket_ptr);

void cleanup(SOCKET welcoming_socket, Topic topics[], int number_of_topics, SocketList connection_sockets);

bool accept_connection(SOCKET welcoming_socket, Topic topics[], int number_of_topics, SocketList* connection_sockets_ptr);

bool receive_command(SOCKET welcoming_socket, SOCKET connection_socket, Topic topics[], int number_of_topics, char receive_buffer[], SocketList connection_sockets);

void send_to_client(SOCKET connection_socket, char* message);
