#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "common.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma pack(1)  

#define BROKER_IP_ADDRESS "127.0.0.1"
#define BROKER_PORT_NUMBER 16000
#define SELECT_ERROR 0
#define WILL_BLOCK 1
#define WONT_BLOCK 2


bool setup(sockaddr_in* broker_data_ptr, SOCKET* client_socket_ptr);

void cleanup(SOCKET client_socket);

bool connect_to_broker(SOCKET client_socket, sockaddr_in* broker_data_ptr, bool* connected_ptr);

bool send_command(SOCKET client_socket, char* command);

bool receive_from_broker(SOCKET client_socket, char receive_buffer[], bool wait_until_received);
