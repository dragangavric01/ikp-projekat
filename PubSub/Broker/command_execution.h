#pragma once

#define _CRT_NONSTDC_NO_DEPRECATE

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socket_list.h"
#include "topic.h"
#include "message_queue.h"
#include "global.h"
#include "networking_broker.h"

char* execute_command(Topic topics[], int num_of_topics, char command[], SOCKET* connection_socket_ptr, SOCKET sm_client_socket);