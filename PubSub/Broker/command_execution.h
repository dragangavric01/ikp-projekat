#pragma once

#define _CRT_NONSTDC_NO_DEPRECATE

#include <stdio.h>
#include "WinSock2.h"
#include "socket_list.h"
#include "message_queue.h"
#include "topic.h"

char* execute_command(Topic topics[], int num_of_topics, char receive_buffer[], SOCKET connection_socket);