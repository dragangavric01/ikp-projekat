#include "topic.h"

Topic initialize_topic(const char* topic_name) {
	MessageQueue message_queue = { NULL, NULL };
	SocketList subscriber_connection_sockets = { NULL, 0 };
	
	Topic topic = { topic_name, message_queue, subscriber_connection_sockets };

	return topic;
}