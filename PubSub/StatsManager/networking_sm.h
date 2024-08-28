#pragma once

#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma pack(1)  

#define RECEIVE_BUFFER_SIZE 2000

#define SELECT_ERROR 0
#define WILL_BLOCK 1
#define WONT_BLOCK 2

#define CONNECTED 0
#define ERROR -1


void setup(SOCKET* welcoming_socket_ptr);

int accept_connection(SOCKET welcoming_socket, SOCKET* connection_socket_ptr);

bool receive_notifications(SOCKET welcoming_socket, SOCKET connection_socket, char receive_buffer[]);