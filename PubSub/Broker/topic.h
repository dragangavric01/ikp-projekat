#pragma once

#include "message_queue.h"
#include "socket_list.h"

typedef struct TopicStruct {
	const char* name;
	MessageQueue message_queue;
	SocketList subscriber_connection_sockets;
} Topic;


Topic initialize_topic(const char* topic_name);