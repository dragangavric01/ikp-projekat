#pragma once

#include "stdint.h"
#include "networking_sm.h"


typedef struct {
    uint64_t publish_commands_num;
    uint64_t subscribe_commands_num;
    uint64_t unsubscribe_commands_num;
    uint64_t topic_exists_commands_num;
    uint64_t subscriber_number_commands_num;
} TopicStats;


bool handle_notifications(SOCKET welcoming_socket, SOCKET connection_socket, char receive_buffer[], TopicStats* topic_stats_array);