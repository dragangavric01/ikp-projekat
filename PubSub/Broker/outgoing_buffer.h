#pragma once

#include <winsock2.h>
#include <windows.h>
#include "socket_list.h"
#include "topic.h"
#include "networking_broker.h"
#include "message_queue.h"
#include "global.h"

#define MAX_OUTGOING_BUFFER_SIZE 100


typedef union OneOrMoreSocketsUnion {
    SocketList* subscriber_connection_sockets_ptr;
    SOCKET connection_socket;
} OneOrMoreSocketsU;

typedef struct OneOrMoreSocketsStruct {
    OneOrMoreSocketsU cs_union;
    bool more;  // true if subscriber_connection_sockets_ptr is set and false if connection_socket is set
} OneOrMoreSockets;


typedef struct OutgoingBufferElementStruct {
    char* message;
    OneOrMoreSockets one_or_more_sockets;  // Threads that put messages from topic queues will supply the connection sockets list. Main thread will suply one connection socket that the response should be sent to.
} OutgoingBufferElement;

typedef struct OutgoingBufferStruct {
    OutgoingBufferElement array[MAX_OUTGOING_BUFFER_SIZE];
    int fill_idx;
    int use_idx;
    int count;
    CRITICAL_SECTION* crit_section_ptr;
    CONDITION_VARIABLE* empty_cv_ptr;
    CONDITION_VARIABLE* fill_cv_ptr;
} OutgoingBuffer;

typedef struct TopicAndBufferStruct {
    Topic* topic_ptr;
    OutgoingBuffer* outgoing_buffer_ptr;
} TopicAndBuffer;


void initialize_outgoing_buffer(OutgoingBuffer* outgoing_buffer_ptr);

void produce_new_message(OutgoingBuffer* outgoing_buffer_ptr, OutgoingBufferElement new_element);

DWORD WINAPI produce(LPVOID ptr_to_topic_and_buffer);

DWORD WINAPI consume(LPVOID ptr_to_outgoing_buffer);
