#include <stdlib.h>
#include <string.h>
#include "command_execution.h"
#include "topic.h"
#include "topic_queue.h"


// COMMAND FORMAT
// Publish: "1#<topic_name>#<message>"
// Subscribe : "2#<topic_name>"
// Topic exists : "3#<topic_name>"
// Subscriber number : "4#<topic_name>"

void execute_command(Topic topics[], int num_of_topics, const char* command, SOCKET connection_socket) {
	if (*command == '1') {
		// there is no /0 at the end of topic_name
		int second_hashtag_offset = 2;
		while (*(command + second_hashtag_offset) != '#') {
			++second_hashtag_offset;
		}

		char* topic_name = (char*)malloc(second_hashtag_offset - 2);
		memcpy_s(topic_name, sizeof(topic_name) - 1, command + 2, sizeof(topic_name) - 1);
		*(topic_name + sizeof(topic_name) - 1) = '\0';

		char* message = (char*)malloc(sizeof(command) - second_hashtag_offset);
		strcpy_s(message, sizeof(message), command + second_hashtag_offset + 1);

		publish(topics, num_of_topics, topic_name, message);
	} else if (*command == '2') {
		char* topic_name = (char*)malloc(sizeof(command) - 2);
		strcpy_s(topic_name, sizeof(topic_name), command + 2);

		subscribe(topics, num_of_topics, topic_name, connection_socket);
	} else if (*command == '3') {
		char* topic_name = (char*)malloc(sizeof(command) - 2);
		strcpy_s(topic_name, sizeof(topic_name), command + 2);

		topic_exists(topics, num_of_topics, topic_name);
	} else if (*command == '4') {
		char* topic_name = (char*)malloc(sizeof(command) - 2);
		strcpy_s(topic_name, sizeof(topic_name), command + 2);

		subscriber_number(topics, num_of_topics, topic_name);
	}
}

static void publish(Topic topics[], int num_of_topics, const char* topic_name, const char* message) {
	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	if (!topic_ptr) {
		return;
	}
	
	enqueue(&((*topic_ptr).queue_pointers), message);
}

static void subscribe(Topic topics[], int num_of_topics, const char* topic_name, SOCKET connection_socket) {
	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	if (!topic_ptr) {
		return;
	}
	
	if ((*topic_ptr).subscriber_list.size <= SUBSCRIBER_LIST_MAX_SIZE) {
		add_to_start(&((*topic_ptr).subscriber_list), &connection_socket);
	}
}

static void topic_exists(Topic topics[], int num_of_topics, const char* topic_name) {
	const char* response;

	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	if (topic_ptr) {
		response = "1";
	} else {
		response = "0";
	}

	// TODO implement sending response
}

static void subscriber_number(Topic topics[], int num_of_topics, const char* topic_name) {
	char response[6];  // 5 characters for SUBSCRIBER_LIST_MAX_SIZE and 1 character for \0
	
	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	if (!topic_ptr) {
		return;
	}

	itoa((*topic_ptr).subscriber_list.size, response, 10);

	// TODO implement sending response
}

static Topic* get_topic_by_name(Topic topics[], int num_of_topics, const char* topic_name) {
	for (int i = 0; i < num_of_topics; i++) {
		if (!strcmp(topics[i].name, topic_name)) {
			return &(topics[i]);
		}
	}

	return NULL;
}