#pragma once

#define THIRD_HASHTAG_OFFSET(topic_name) (1 + 1 + strlen(topic_name) + 1)

char* create_command(char command_num, const char* topic_name, const char* message);