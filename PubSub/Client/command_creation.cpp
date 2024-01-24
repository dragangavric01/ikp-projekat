#include <stdlib.h>
#include <string.h>
#include "command_creation.h"


// COMMAND FORMAT
// Publish: "2#<topic_name>#<message>"
// Subscribe : "3#<topic_name>"
// Topic exists : "4#<topic_name>"
// Subscriber number : "5#<topic_name>"

// Creates a publish command
static char* create_publish_command(int topic_name_length, int message_length, const char* topic_name, const char* message) {
	char* command = (char*)malloc(1  // # that's at the beginning of the command (so broker can read the receive buffer)
							+ 1  // command_num
							+ 1  // #
							+ topic_name_length  // topic_name without null character (because null character can only be at the end of a command)
							+ 1  // #
							+ message_length + 1);  // message with null character

	*command = '#';  // so broker can read the receive buffer
	*(command + 1) = '2'; // put '2' after first #
	*(command + 2) = '#'; // put '#' after '2'

	memcpy(command + 3, topic_name, topic_name_length);  // copy topic_name without null character into command
	*(command + THIRD_HASHTAG_OFFSET(topic_name)) = '#';  // put '#' after topic_name

	memcpy(command + THIRD_HASHTAG_OFFSET(topic_name) + 1, message, message_length + 1);   // copy message into command

	return command;
}

// Creates a subscribe, topic exists, or subscriber number command
static char* create_nonpublish_command(char command_num, int topic_name_length, const char* topic_name) {
	char* command = (char*)malloc(1  // # that's at the beginning of the command (so broker can read the receive buffer)
							+ 1  // command_num
							+ 1  // #
							+ topic_name_length + 1);  // topic_name with null character

	*command = '#';  // so broker can read the receive buffer
	*(command + 1) = command_num; // put command_num at the start of command
	*(command + 2) = '#'; // put '#' after command_num

	memcpy(command + 3, topic_name, topic_name_length + 1);  // copy topic_name with null character into command

	return command;
}

char* create_command(char command_num, const char* topic_name, const char* message) {
	char* command;

	int topic_name_length = strlen(topic_name);
	int message_length = -1;  // -1 so that in malloc after adding 1 to it you get zero, so no memory is allocated for message if message is NULL
	if (message) {
		message_length = strlen(message);
	}

	if (command_num == '2') {
		command = create_publish_command(topic_name_length, message_length, topic_name, message);
	} else {
		command = create_nonpublish_command(command_num, topic_name_length, topic_name);
	}

	return command;
}