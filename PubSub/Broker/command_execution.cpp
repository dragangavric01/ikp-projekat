#include "command_execution.h"


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

static void publish(Topic topics[], int num_of_topics, const char* topic_name, char* message) {
	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	free((void*)topic_name);
	if (!topic_ptr) {
		return;
	}

	enqueue((*topic_ptr).message_queue_ptr, message, (*topic_ptr).name);
}

static void subscribe(Topic topics[], int num_of_topics, const char* topic_name, SOCKET* connection_socket_ptr) {
	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	free((void*)topic_name);
	if (!topic_ptr) {
		return;
	}

	if ((*((*topic_ptr).subscriber_connection_sockets_ptr)).size <= SUBSCRIBER_LIST_MAX_SIZE) {  // No need for mutual exclusion here because size gets changed only in main thread, so it's not being changed right now
		// There is no checking if the socket already exists in the list because that would be an O(n) operation
		add_to_start((*topic_ptr).subscriber_connection_sockets_ptr, connection_socket_ptr);

		EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
		printf("Socket %llu has been added to '%s' subscriber connection sockets\n", *connection_socket_ptr, (*topic_ptr).name);
		printf("Subscriber sockets: ");
		print_socket_list_unsafe((*topic_ptr).subscriber_connection_sockets_ptr);
		LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
	} else {
		EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
		printf("Adding socket %llu to subscriber connection sockets failed because the list is full\n", *connection_socket_ptr);
		LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
	}
}

static char* topic_exists(Topic topics[], int num_of_topics, const char* topic_name) {
	char* response = (char*)malloc(2);
	response[1] = '\0';
	
	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	free((void*)topic_name);
	if (topic_ptr) {
		response[0] = '1';
	} else {
		response[0] = '0';
	}

	return response;
}

static char* subscriber_number(Topic topics[], int num_of_topics, const char* topic_name) {
	int response_size = 5  // 5 characters for SUBSCRIBER_LIST_MAX_SIZE 
						+ 1; // for \0
	char* response = (char*)malloc(response_size);

	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	free((void*)topic_name);
	if (!topic_ptr) {
		response[0] = '#';
		response[1] = '\0';
		return response;
	}

	// No need for mutual exclusion here because size gets changed only in main thread, so it's not being changed right now
	_itoa_s((*((*topic_ptr).subscriber_connection_sockets_ptr)).size, response, response_size, 10);

	return response;
}

static void extract_for_publish(char command[], char** topic_name_ptr, char** message_ptr) {
	// there is no /0 at the end of topic_name
	int second_hashtag_index = 2;
	while (command[second_hashtag_index] != '#') {
		++second_hashtag_index;
	}

	const int topic_name_size = second_hashtag_index - 2 + 1;

	*topic_name_ptr = (char*)malloc(topic_name_size);
	memcpy_s(*topic_name_ptr, topic_name_size - 1, &command[2], topic_name_size - 1);
	*(*topic_name_ptr + topic_name_size - 1) = '\0';

	const int message_size = strlen(command) - second_hashtag_index;
	*message_ptr = (char*)malloc(1 + message_size);
	**message_ptr = '#';  // # is put at the beginning so that clients will be able to read the receive buffer
	strcpy_s(*message_ptr + 1, message_size, &command[second_hashtag_index + 1]);
}

static char* extract_topic_name(char command[]) {
	const int topic_name_size = strlen(command) + 1 - 2;
	char* topic_name = (char*)malloc(topic_name_size);

	strcpy_s(topic_name, topic_name_size, &command[2]);

	return topic_name;
}

char* execute_command(Topic topics[], int num_of_topics, char command[], SOCKET* connection_socket_ptr) {
	if (command[0] == '2') {
		char* topic_name;
		char* message;
		extract_for_publish(command, &topic_name, &message);

		publish(topics, num_of_topics, topic_name, message);
	} else if (command[0] == '3') {
		char* topic_name = extract_topic_name(command);

		subscribe(topics, num_of_topics, topic_name, connection_socket_ptr);
	} else if (command[0] == '4') {
		char* topic_name = extract_topic_name(command);

		return topic_exists(topics, num_of_topics, topic_name);
	} else if (command[0] == '5') {
		char* topic_name = extract_topic_name(command);

		return subscriber_number(topics, num_of_topics, topic_name);
	}

	return NULL;
}

