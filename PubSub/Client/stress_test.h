#pragma once

#include <winsock2.h>
#include "networking_client.h"
#include "command_creation.h"
#include <time.h>
#include <iostream>

void test(char client_number, SOCKET client_socket, bool* connected_ptr, sockaddr_in* broker_data_ptr, char receive_buffer[]);
