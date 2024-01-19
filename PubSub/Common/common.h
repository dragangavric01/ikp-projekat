#pragma once

#include <windows.h>

#define MAX_TOPIC_SIZE 50 + 1
#define MAX_MESSAGE_SIZE 50 + 1
#define MAX_COMMAND_SIZE (MAX_TOPIC_SIZE + MAX_MESSAGE_SIZE + 4)

#define DEBUG

void window_setup(char* argv[]);
