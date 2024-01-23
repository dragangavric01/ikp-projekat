#pragma once

#include "socket_list.h"
#include "message_queue.h"

typedef struct TopicStruct {
	const char* name;
	MessageQueue* message_queue_ptr;
	SocketList* subscriber_connection_sockets_ptr;
	HANDLE thread;
} Topic;


Topic initialize_topic(const char* topic_name);
