#include <stdlib.h>
#include <string.h>
#include "command_creation.h"

#define SECOND_HASHTAG_OFFSET(topic_name) (2 + (sizeof(topic_name) - 1) + 1)


// COMMAND FORMAT
// Publish: "1#<topic_name>#<message>"
// Subscribe : "2#<topic_name>"
// Topic exists : "3#<topic_name>"
// Subscriber number : "4#<topic_name>"

char* create_command(char command_num, const char* topic_name, const char* message) {
	char* command;

	if (command_num == '1') {
		command = (char*)malloc(1  // command_num
			+ 1  // #
			+ (sizeof(topic_name) - 1)  // topic_name without null character (because null character can only be at the end of a command)
			+ 1  // #
			+ sizeof(message));  // message

		*command = '1';  // put '1' at the start of command
		*(command + 1) = '#'; // put '#' after '1'

		strcpy_s(command + 2, sizeof(topic_name) - 1, topic_name);  // copy topic_name without null character into command
		*(command + SECOND_HASHTAG_OFFSET(topic_name)) = '#';  // put '#' after topic_name

		strcpy_s(command + SECOND_HASHTAG_OFFSET(topic_name) + 1, sizeof(message), message);  // copy message into command
	} else {
		command = (char*)malloc(1  // command_num
			+ 1  // #
			+ sizeof(topic_name));  // topic_name 

		*command = command_num;  // put command_num at the start of command
		*(command + 1) = '#'; // put '#' after command_num

		strcpy_s(command + 2, sizeof(topic_name), topic_name);  // copy topic_name into command
	}

	return command;
}