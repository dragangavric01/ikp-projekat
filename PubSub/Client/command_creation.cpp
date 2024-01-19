#include <stdlib.h>
#include <string.h>
#include "command_creation.h"


// COMMAND FORMAT
// Publish: "2#<topic_name>#<message>"
// Subscribe : "3#<topic_name>"
// Topic exists : "4#<topic_name>"
// Subscriber number : "5#<topic_name>"

char* create_command(char command_num, const char* topic_name, const char* message) {
	char* command;

	int topic_name_length = strlen(topic_name);
	int message_length = -1;  // -1 so that in malloc after adding 1 to it you get zero, so no memory is allocated for message if message is NULL
	if (message) {
		message_length = strlen(message);
	}

	if (command_num == '2') {
		command = (char*)malloc(1  // command_num
			+ 1  // #
			+ topic_name_length  // topic_name without null character (because null character can only be at the end of a command)
			+ 1  // #
			+ message_length + 1);  // message with null character

		*command = '2';  // put '2' at the start of command
		*(command + 1) = '#'; // put '#' after '2'

		memcpy(command + 2, topic_name, topic_name_length);  // copy topic_name without null character into command
		*(command + SECOND_HASHTAG_OFFSET(topic_name)) = '#';  // put '#' after topic_name

		memcpy(command + SECOND_HASHTAG_OFFSET(topic_name) + 1, message, message_length + 1);   // copy message into command
	} else {
		command = (char*)malloc(1  // command_num
			+ 1  // #
			+ topic_name_length + 1);  // topic_name with null character

		*command = command_num;  // put command_num at the start of command
		*(command + 1) = '#'; // put '#' after command_num

		memcpy(command + 2, topic_name, topic_name_length + 1);  // copy topic_name with null character into command
	}

	return command;
}