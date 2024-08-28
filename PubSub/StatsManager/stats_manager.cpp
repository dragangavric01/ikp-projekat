#include  <iostream> 
#include <time.h>
#include <stdint.h>
#include "networking_sm.h"
#include "notification_handling.h"
#include "file_updating.h"


#ifdef DEBUG
    #define UPDATE_PERIOD 20
#else 
    #define UPDATE_PERIOD 60
#endif


int main(int argc, char* argv[]) {
    if (argc > 1) {
        window_setup(argv);
    }

    char receive_buffer[BROKER_RECEIVE_BUFFER_SIZE];
    memset(receive_buffer, 0, BROKER_RECEIVE_BUFFER_SIZE);

    SOCKET welcoming_socket = INVALID_SOCKET;
    setup(&welcoming_socket);

    SOCKET connection_socket;
    while (true) {
        int result = accept_connection(welcoming_socket, &connection_socket);
        if (result == ERROR) {
            exit(16);
        } else if (result == CONNECTED) {
            puts("Broker connected\n");
            break;
        }
    }

    TopicStats news_stats = { 0, 0, 0, 0, 0, };
    TopicStats updates_stats = { 0, 0, 0, 0, 0, };
    TopicStats warnings_stats = { 0, 0, 0, 0, 0, };
    TopicStats topic_stats_array[3] = { news_stats, updates_stats, warnings_stats };

    puts("Loading stats from storage...");
    read_and_show_stats();

    bool notifications_received = false;
    bool notifications_just_received = false;
    int time_difference = 0;
    time_t last_update_time = time(NULL);
    while (true) {
        notifications_just_received = handle_notifications(welcoming_socket, connection_socket, receive_buffer, topic_stats_array);
        if (notifications_just_received) {
            time_difference = UPDATE_PERIOD - (int)difftime(time(NULL), last_update_time);
            if (time_difference > 0) {
                printf("A notification has been received. Update in %d seconds.\n", time_difference);
            } else {
                printf("A notification has been received.\n");
            }

            notifications_received = true;
        }

        if (notifications_received && difftime(time(NULL), last_update_time) >= UPDATE_PERIOD) {
            update_and_show_stats(topic_stats_array);
            notifications_received = false;
            last_update_time = time(NULL);
        }
    }

    return 0;
}

