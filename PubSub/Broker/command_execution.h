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

char* execute_command(Topic topics[], int num_of_topics, char receive_buffer[], SOCKET* connection_socket_ptr);
