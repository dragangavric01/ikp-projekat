#pragma once

#include "topic.h"
#include "outgoing_buffer.h"
#include <windows.h>

void mutual_assured_destruction(SOCKET welcoming_socket, Topic topics[], int number_of_topics, SocketList* connection_sockets_ptr, HANDLE* consumer_thread_ptr, OutgoingBuffer* outgoing_buffer_ptr);
