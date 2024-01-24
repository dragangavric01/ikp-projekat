// windows.h must be included after winsock2.h !!!!!!!!
#include "networking_broker.h"
#include "outgoing_buffer.h"
#include "mad.h"
#include "global.h"
#include "common.h"  

#define TOPICS_NUM 3

static bool create_and_start_threads(Topic topics[], OutgoingBuffer* outgoing_buffer_ptr, HANDLE* consumer_thread_ptr);

static void receive_and_execute_commands(Topic topics[], int num_of_topics, SOCKET welcoming_socket, char receive_buffer[], SocketList* connection_sockets_ptr, OutgoingBuffer* outgoing_buffer_ptr);


int main(int argc, char* argv[]) {
    if (argc > 1) {
        window_setup(argv);
    }

    char receive_buffer[BROKER_RECEIVE_BUFFER_SIZE];
    memset(receive_buffer, 0, BROKER_RECEIVE_BUFFER_SIZE);

    SOCKET welcoming_socket = INVALID_SOCKET;
    setup(&welcoming_socket);  // It's important that this line is right after the declaration of welcoming_socket because if setup() fails, it won't free other resources, it will just exit immediatelly. When it's here, no other resources have been taken before it's call.

    initialize_shutting_down_flag();
    initialize_printf_crit_section();

    SocketList connection_sockets;
    OutgoingBuffer outgoing_buffer;
    initialize_socket_list(&connection_sockets, false);
    initialize_outgoing_buffer(&outgoing_buffer);

    Topic topics[TOPICS_NUM];
    topics[0] = initialize_topic("updates");
    topics[1] = initialize_topic("news");
    topics[2] = initialize_topic("warnings");

    HANDLE consumer_thread;
    bool threads_started = create_and_start_threads(topics, &outgoing_buffer, &consumer_thread);
    if (!threads_started) {
        mutual_assured_destruction(welcoming_socket, topics, TOPICS_NUM, &connection_sockets, &consumer_thread, &outgoing_buffer);
    }

    puts("Broker is ready\n");

    while (true) {
        accept_connection(welcoming_socket, topics, TOPICS_NUM, &connection_sockets);

        receive_and_execute_commands(topics, TOPICS_NUM, welcoming_socket, receive_buffer, &connection_sockets, &outgoing_buffer);
    }

    return 0;
}


static bool create_and_start_threads(Topic topics[], OutgoingBuffer* outgoing_buffer_ptr, HANDLE* consumer_thread_ptr) {
    TopicAndBuffer* updates_topic_and_buffer_ptr = (TopicAndBuffer*)malloc(sizeof(TopicAndBuffer));
    updates_topic_and_buffer_ptr->outgoing_buffer_ptr = outgoing_buffer_ptr;
    updates_topic_and_buffer_ptr->topic_ptr = &(topics[0]);
    topics[0].producer_thread = CreateThread(NULL, 0, &produce, updates_topic_and_buffer_ptr, 0, NULL);

    TopicAndBuffer* news_topic_and_buffer_ptr = (TopicAndBuffer*)malloc(sizeof(TopicAndBuffer));
    news_topic_and_buffer_ptr->outgoing_buffer_ptr = outgoing_buffer_ptr;
    news_topic_and_buffer_ptr->topic_ptr = &(topics[1]);
    topics[1].producer_thread = CreateThread(NULL, 0, &produce, news_topic_and_buffer_ptr, 0, NULL);

    TopicAndBuffer* warnings_topic_and_buffer_ptr = (TopicAndBuffer*)malloc(sizeof(TopicAndBuffer));
    warnings_topic_and_buffer_ptr->outgoing_buffer_ptr = outgoing_buffer_ptr;
    warnings_topic_and_buffer_ptr->topic_ptr = &(topics[2]);
    topics[2].producer_thread = CreateThread(NULL, 0, &produce, warnings_topic_and_buffer_ptr, 0, NULL);

    *consumer_thread_ptr = CreateThread(NULL, 0, &consume, outgoing_buffer_ptr, 0, NULL);

    if (topics[0].producer_thread && topics[1].producer_thread && topics[2].producer_thread && *consumer_thread_ptr) {
        return true;
    } else {
        return false;
    }
}

// Does the action that is specified by the command and if there is a response to the command it puts the response in the outgoing buffer
static void execute_command_and_produce(Topic topics[], int num_of_topics, char receive_buffer[], OutgoingBuffer* outgoing_buffer_ptr, SOCKET* connection_socket_ptr) {
    char* response = execute_command(topics, num_of_topics, receive_buffer, connection_socket_ptr);
    if (response) {
        OneOrMoreSockets one_or_more_sockets;
        one_or_more_sockets.cs_union.connection_socket = *connection_socket_ptr;
        one_or_more_sockets.more = false;
        OutgoingBufferElement new_element = { response, one_or_more_sockets };

        produce_new_message(outgoing_buffer_ptr, new_element);
    }
}

// Receives commands from all clients that have sent one or more commands since the last time it was executed and it calls execute_command_and_produce() for every received command
static void receive_and_execute_commands(Topic topics[], int num_of_topics, SOCKET welcoming_socket, char receive_buffer[], SocketList* connection_sockets_ptr, OutgoingBuffer* outgoing_buffer_ptr) {
    // Mutual exclusion not needed because connection_sockets list is only accessed in main thread

    SocketListNode* walker = (*connection_sockets_ptr).head;
    while (walker) {
        int return_value = receive_commands(welcoming_socket, (*walker).socket_ptr, topics, num_of_topics, receive_buffer, connection_sockets_ptr, &walker);
        if (return_value == RECEIVED) {
            int i = 0;
            while (true) {
                if (receive_buffer[i] == '#') {  // At the beginning of every command is '#'
                    execute_command_and_produce(topics, num_of_topics, &(receive_buffer[i+1]), outgoing_buffer_ptr, (*walker).socket_ptr);

                    // Go to the beggining of the next command
                    while (receive_buffer[i] != '\0') {
                        i++;
                    }
                    i++;
                } else {
                    break;
                }
            }

            memset(receive_buffer, 0, BROKER_RECEIVE_BUFFER_SIZE);
        }

        // If the current walker node has been deleted, "walker = (*walker).next;" has been done inside receive_command(), so it shouldn't be done again
        if (return_value != DELETED) {
            walker = (*walker).next;
        }
    }
}

