#include "topic.h"

Topic initialize_topic(const char* topic_name) {
	MessageQueue* message_queue_ptr = (MessageQueue*)malloc(sizeof(MessageQueue));
	initialize_message_queue(message_queue_ptr);

	SocketList* subscriber_connection_sockets_ptr = (SocketList*)malloc(sizeof(SocketList));
	initialize_socket_list(subscriber_connection_sockets_ptr, true);
	
	Topic topic = { topic_name, message_queue_ptr, subscriber_connection_sockets_ptr, NULL };

	return topic;
}