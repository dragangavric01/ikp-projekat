#pragma once

#include <windows.h>

#define MAX_TOPIC_SIZE 20 + 1
#define MAX_MESSAGE_SIZE 20 + 1
#define MAX_COMMAND_SIZE (MAX_TOPIC_SIZE + MAX_MESSAGE_SIZE + 4)

#define CLIENT_RECEIVE_BUFFER_SIZE (100 * MAX_MESSAGE_SIZE)
#define BROKER_RECEIVE_BUFFER_SIZE (100 * MAX_COMMAND_SIZE)

#define DEBUG

void window_setup(char* argv[]);
