#include "notification_handling.h"


static TopicStats* get_topic_stats(TopicStats* topic_stats_array, char* topic_name) {
    if (!strcmp(topic_name, "news")) {
        return &(topic_stats_array[0]);
    } else if (!strcmp(topic_name, "updates")) {
        return &(topic_stats_array[1]);
    } else if (!strcmp(topic_name, "warnings")) {
        return &(topic_stats_array[2]);
    } else {
        return NULL;
    }
}

static void handle_notification(char notification[], TopicStats* topic_stats_array) {
    char notification_type = notification[0];

    const int topic_name_size = strlen(notification) + 1 - 2;
    char* topic_name = (char*)malloc(topic_name_size);
    strcpy_s(topic_name, topic_name_size, &notification[2]);

    TopicStats* topic_stats_ptr = get_topic_stats(topic_stats_array, topic_name);
    free((void*)topic_name);
    if (!topic_stats_ptr) {
        return;
    }

    if (notification_type == '2') {
        (*topic_stats_ptr).publish_commands_num++;
    } else if (notification_type == '3') {
        (*topic_stats_ptr).subscribe_commands_num++;
    } else if (notification_type == '4') {
        (*topic_stats_ptr).topic_exists_commands_num++;
    } else if (notification_type == '5') {
        (*topic_stats_ptr).subscriber_number_commands_num++;
    } else if (notification_type == '6') {
        (*topic_stats_ptr).unsubscribe_commands_num++;
    }
}


bool handle_notifications(SOCKET welcoming_socket, SOCKET connection_socket, char receive_buffer[], TopicStats* topic_stats_array) {
    if (!receive_notifications(welcoming_socket, connection_socket, receive_buffer)) {
        return false;
    }

    int i = 0;
    while (true) {
        if (receive_buffer[i] == '#') {
            handle_notification(receive_buffer + 1, topic_stats_array);
        
            // Go to the beggining of the next notification
            while (receive_buffer[i] != '\0') {
                i++;
            }
            i++;
        } else {
            break;
        }
    }


    memset(receive_buffer, 0, RECEIVE_BUFFER_SIZE);


    return (i != 0);
}