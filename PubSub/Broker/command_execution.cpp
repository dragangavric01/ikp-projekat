#include <stdlib.h>
#include <string.h>
#include "command_execution.h"
#include "topic.h"
#include "message_queue.h"


// COMMAND FORMAT
// Publish: "2#<topic_name>#<message>"
// Subscribe : "3#<topic_name>"
// Topic exists : "4#<topic_name>"
// Subscriber number : "5#<topic_name>"


static Topic* get_topic_by_name(Topic topics[], int num_of_topics, const char* topic_name) {
	for (int i = 0; i < num_of_topics; i++) {
		if (!strcmp(topics[i].name, topic_name)) {
			return &(topics[i]);
		}
	}

	return NULL;
}

static void publish(Topic topics[], int num_of_topics, const char* topic_name, const char* message) {
	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	if (!topic_ptr) {
		return;
	}

	enqueue(&((*topic_ptr).message_queue), message);

	printf("Message '%s' enqueued (topic '%s')\n", message, (*topic_ptr).name);
	printf("Topic '%s' message_queue:\n", (*topic_ptr).name);
	print_message_queue((*topic_ptr).message_queue);

}

static void subscribe(Topic topics[], int num_of_topics, const char* topic_name, SOCKET connection_socket) {
	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	if (!topic_ptr) {
		return;
	}

	if ((*topic_ptr).subscriber_connection_sockets.size <= SUBSCRIBER_LIST_MAX_SIZE) {
		// There is no checking if the socket already exists in the list because that would be an O(n) operation
		add_to_start(&((*topic_ptr).subscriber_connection_sockets), connection_socket);

		printf("Socket %llu has been added to subscriber connection sockets\n", connection_socket);
		printf("\tSubscriber sockets: ");
		print_socket_list((*topic_ptr).subscriber_connection_sockets);
	} else {
		printf("Adding socket %llu to subscriber connection sockets failed because the list is full\n", connection_socket);
	}
}

static char* topic_exists(Topic topics[], int num_of_topics, const char* topic_name) {
	char* response = (char*)malloc(2);
	response[1] = '\0';
	
	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	if (topic_ptr) {
		response[0] = '1';
	} else {
		response[0] = '0';
	}

	return response;
}

static char* subscriber_number(Topic topics[], int num_of_topics, const char* topic_name) {
	int response_size = 5    // 5 characters for SUBSCRIBER_LIST_MAX_SIZE 
						+ 1; // for \0
	char* response = (char*)malloc(response_size);

	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	if (!topic_ptr) {
		return NULL;
	}

	_itoa_s((*topic_ptr).subscriber_connection_sockets.size, response, response_size, 10);

	return response;
}

static void extract_topic_name_and_message(char receive_buffer[], char** topic_name_ptr, char** message_ptr) {
	// there is no /0 at the end of topic_name
	int second_hashtag_index = 2;
	while (receive_buffer[second_hashtag_index] != '#') {
		++second_hashtag_index;
	}

	const int topic_name_size = second_hashtag_index - 2 + 1;

	*topic_name_ptr = (char*)malloc(topic_name_size);
	memcpy_s(*topic_name_ptr, topic_name_size - 1, &receive_buffer[2], topic_name_size - 1);
	*(*topic_name_ptr + topic_name_size - 1) = '\0';

	const int message_size = strlen(receive_buffer) - second_hashtag_index;
	*message_ptr = (char*)malloc(message_size);
	strcpy_s(*message_ptr, message_size, &receive_buffer[second_hashtag_index + 1]);
}

static char* extract_topic_name(char receive_buffer[]) {
	const int topic_name_size = strlen(receive_buffer) + 1 - 2;
	char* topic_name = (char*)malloc(topic_name_size);

	strcpy_s(topic_name, topic_name_size, &receive_buffer[2]);

	return topic_name;
}

char* execute_command(Topic topics[], int num_of_topics, char receive_buffer[], SOCKET connection_socket) {
	if (receive_buffer[0] == '2') {
		char* topic_name;
		char* message;
		extract_topic_name_and_message(receive_buffer, &topic_name, &message);

		publish(topics, num_of_topics, topic_name, message);
	} else if (receive_buffer[0] == '3') {
		char* topic_name = extract_topic_name(receive_buffer);

		subscribe(topics, num_of_topics, topic_name, connection_socket);
	} else if (receive_buffer[0] == '4') {
		char* topic_name = extract_topic_name(receive_buffer);

		return topic_exists(topics, num_of_topics, topic_name);
	} else if (receive_buffer[0] == '5') {
		char* topic_name = extract_topic_name(receive_buffer);

		return subscriber_number(topics, num_of_topics, topic_name);
	}

	return NULL;
}

