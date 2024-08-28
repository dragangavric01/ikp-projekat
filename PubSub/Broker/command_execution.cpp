#include "command_execution.h"


// COMMAND FORMAT
// Publish: "2#<topic_name>#<message>"
// Subscribe : "3#<topic_name>"
// Topic exists : "4#<topic_name>"
// Subscriber number : "5#<topic_name>"

// Returns a pointer to an instance of Topic structure that has the specified name or returns NULL if there is no topic with that name
static Topic* get_topic_by_name(Topic topics[], int num_of_topics, const char* topic_name) {
	for (int i = 0; i < num_of_topics; i++) {
		if (!strcmp(topics[i].name, topic_name)) {
			return &(topics[i]);
		}
	}

	return NULL;
}

// Puts the message in the message queue of topic whose name is topic_name
static bool publish(Topic topics[], int num_of_topics, const char* topic_name, char* message) {
	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	free((void*)topic_name);
	if (!topic_ptr) {
		return false;
	}

	enqueue((*topic_ptr).message_queue_ptr, message, (*topic_ptr).name);

	return true;
}

// Puts the connection socket in subscriber_connection_sockets of topic whose name is topic_name and prints the whole list
static bool subscribe(Topic topics[], int num_of_topics, const char* topic_name, SOCKET* connection_socket_ptr) {
	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	free((void*)topic_name);
	if (!topic_ptr) {
		return false;
	}

	if ((*((*topic_ptr).subscriber_connection_sockets_ptr)).size <= SUBSCRIBER_LIST_MAX_SIZE) {  // No need for mutual exclusion here because size gets changed only in main thread, so it's not being changed right now
		// There is no checking if the socket already exists in the list because that would be an O(n) operation
		add_to_start((*topic_ptr).subscriber_connection_sockets_ptr, connection_socket_ptr);

		EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
		printf("Socket %llu has been added to '%s' SC sockets\n", *connection_socket_ptr, (*topic_ptr).name);
		printf("Topic '%s' SC sockets: ", (*topic_ptr).name);
		print_socket_list_unsafe((*topic_ptr).subscriber_connection_sockets_ptr);
		LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));

		return true;
	} else {
		EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
		printf("Adding socket %llu to SC sockets failed because the list is full\n", *connection_socket_ptr);
		LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));

		return false;
	}
}

// Removes the connection socket from subscriber_connection_sockets of topic whose name is topic_name and prints the whole list
static bool unsubscribe(Topic topics[], int num_of_topics, const char* topic_name, SOCKET* connection_socket_ptr) {
	Topic* topic_ptr = get_topic_by_name(topics, num_of_topics, topic_name);
	free((void*)topic_name);
	if (!topic_ptr) {
		return false;
	}

	bool removed = free_node((*topic_ptr).subscriber_connection_sockets_ptr, connection_socket_ptr);
	if (removed) {
		EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
		printf("Socket %llu has been removed from '%s' SC sockets\n", *connection_socket_ptr, (*topic_ptr).name);
		printf("Topic '%s' SC sockets: ", (*topic_ptr).name);
		print_socket_list_unsafe((*topic_ptr).subscriber_connection_sockets_ptr);
		LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));

		return true;
	} else {
		EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
		printf("Removing socket %llu from '%s' SC sockets failed because the list doesn't contain this socket\n\n", *connection_socket_ptr, (*topic_ptr).name);
		LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));

		return false;
	}
}

// Returns "1\0" if the topic exists or "0\0" if it doesn't
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

// Returns the number of subscriber connection sockets for topic with name topic_name in a char* form
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

static void notify_stats_manager(SOCKET sm_client_socket, char* command_with_start_hashtag) {
	if (command_with_start_hashtag[1] == '2') {
		int second_hashtag_index = 3;
		while (command_with_start_hashtag[second_hashtag_index] != '#') {
			++second_hashtag_index;
		}

		const int command_without_message_size = second_hashtag_index + 1;
		char* command_without_message = (char*)malloc(command_without_message_size);
		memcpy(command_without_message, command_with_start_hashtag, command_without_message_size);
		command_without_message[command_without_message_size - 1] = '\0';

		send_to_stats_manager(sm_client_socket, command_without_message);
	} else {
		send_to_stats_manager(sm_client_socket, command_with_start_hashtag);
	}
}


char* execute_command(Topic topics[], int num_of_topics, char command[], SOCKET* connection_socket_ptr, SOCKET sm_client_socket) {
	if (command[0] == '2') {
		char* topic_name;
		char* message;
		extract_for_publish(command, &topic_name, &message);

		if (publish(topics, num_of_topics, topic_name, message)) {
			notify_stats_manager(sm_client_socket, command - 1);
		}
	} else if (command[0] == '3') {
		char* topic_name = extract_topic_name(command);

		if (subscribe(topics, num_of_topics, topic_name, connection_socket_ptr)) {
			notify_stats_manager(sm_client_socket, command - 1);
		}
	} else if (command[0] == '4') {
		char* topic_name = extract_topic_name(command);

		char* response = topic_exists(topics, num_of_topics, topic_name);
		if (response[0] == '1') {
			notify_stats_manager(sm_client_socket, command - 1);
		}

		return response;
	} else if (command[0] == '5') {
		char* topic_name = extract_topic_name(command);

		char* response = subscriber_number(topics, num_of_topics, topic_name);
		if (response[0] != '#') {
			notify_stats_manager(sm_client_socket, command - 1);
		}

		return response;
	} else if (command[0] == '6') {
		char* topic_name = extract_topic_name(command);
	
		if (unsubscribe(topics, num_of_topics, topic_name, connection_socket_ptr)) {
			notify_stats_manager(sm_client_socket, command - 1);
		}
	}

	return NULL;
}

