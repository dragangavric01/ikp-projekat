// windows.h must be included after winsock2.h !!!!!!!!
#include "networking_broker.h"
#include "outgoing_buffer.h"
#include "global.h"
#include "common.h"  

#define TOPICS_NUM 3

static bool create_and_start_threads(Topic topics[], OutgoingBuffer* outgoing_buffer_ptr, HANDLE* send_thread_ptr);

static void cleanup(SOCKET welcoming_socket, Topic topics[], int number_of_topics, SocketList* connection_sockets_ptr, HANDLE* send_thread_ptr, OutgoingBuffer* outgoing_buffer_ptr);

static void receive_and_execute_commands(Topic topics[], int num_of_topics, SOCKET welcoming_socket, char receive_buffer[], SocketList* connection_sockets_ptr, OutgoingBuffer* outgoing_buffer_ptr);


int main(int argc, char* argv[]) {
    if (argc > 1) {
        window_setup(argv);
    }

    char receive_buffer[MAX_COMMAND_SIZE];
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

    HANDLE send_thread;
    bool threads_started = create_and_start_threads(topics, &outgoing_buffer, &send_thread);
    if (!threads_started) {
        cleanup(welcoming_socket, topics, TOPICS_NUM, &connection_sockets, &send_thread, &outgoing_buffer);
    }


    while (true) {
        accept_connection(welcoming_socket, topics, TOPICS_NUM, &connection_sockets);

        receive_and_execute_commands(topics, TOPICS_NUM, welcoming_socket, receive_buffer, &connection_sockets, &outgoing_buffer);
    }

    return 0;
}


static bool create_and_start_threads(Topic topics[], OutgoingBuffer* outgoing_buffer_ptr, HANDLE* send_thread_ptr) {
    TopicAndBuffer* updates_topic_and_buffer_ptr = (TopicAndBuffer*)malloc(sizeof(TopicAndBuffer));
    updates_topic_and_buffer_ptr->outgoing_buffer_ptr = outgoing_buffer_ptr;
    updates_topic_and_buffer_ptr->topic_ptr = &(topics[0]);
    topics[0].thread = CreateThread(NULL, 0, &produce, updates_topic_and_buffer_ptr, 0, NULL);

    TopicAndBuffer* news_topic_and_buffer_ptr = (TopicAndBuffer*)malloc(sizeof(TopicAndBuffer));
    news_topic_and_buffer_ptr->outgoing_buffer_ptr = outgoing_buffer_ptr;
    news_topic_and_buffer_ptr->topic_ptr = &(topics[1]);
    topics[1].thread = CreateThread(NULL, 0, &produce, news_topic_and_buffer_ptr, 0, NULL);

    TopicAndBuffer* warnings_topic_and_buffer_ptr = (TopicAndBuffer*)malloc(sizeof(TopicAndBuffer));
    warnings_topic_and_buffer_ptr->outgoing_buffer_ptr = outgoing_buffer_ptr;
    warnings_topic_and_buffer_ptr->topic_ptr = &(topics[2]);
    topics[2].thread = CreateThread(NULL, 0, &produce, warnings_topic_and_buffer_ptr, 0, NULL);

    *send_thread_ptr = CreateThread(NULL, 0, &consume, outgoing_buffer_ptr, 0, NULL);

    if (topics[0].thread && topics[1].thread && topics[2].thread && *send_thread_ptr) {
        return true;
    } else {
        return false;
    }
}

static void shut_down_threads() {
    EnterCriticalSection(shutting_down.crit_section_ptr);

    shutting_down.flag = true;

    // It waits until all threads are shut down because if it goes on and frees the queues and lists before that, the program will crash
    while (shutting_down.num_of_shut_down_threads < (TOPICS_NUM + 1)) {
        SleepConditionVariableCS(shutting_down.cond_var_ptr, shutting_down.crit_section_ptr, INFINITE);
    }

    LeaveCriticalSection(shutting_down.crit_section_ptr);
}

static void cleanup(SOCKET welcoming_socket, Topic topics[], int number_of_topics, SocketList* connection_sockets_ptr, HANDLE* send_thread_ptr, OutgoingBuffer* outgoing_buffer_ptr) {
    shut_down_threads();
    DeleteCriticalSection(shutting_down.crit_section_ptr);
    
    CloseHandle(*send_thread_ptr);
    DeleteCriticalSection((*outgoing_buffer_ptr).crit_section_ptr);
    DeleteCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));

    close_sockets_and_free_socket_list(connection_sockets_ptr);

    for (int i = 0; i < number_of_topics; i++) {
        CloseHandle(topics[i].thread);

        DeleteCriticalSection((*(topics[i].message_queue_ptr)).crit_section_ptr);
        free_message_queue(topics[i].message_queue_ptr);

        DeleteCriticalSection((*(topics[i].subscriber_connection_sockets_ptr)).crit_section_ptr);
        close_sockets_and_free_socket_list(topics[i].subscriber_connection_sockets_ptr);
    }

    closesocket(welcoming_socket);

    WSACleanup();
}

static void receive_and_execute_commands(Topic topics[], int num_of_topics, SOCKET welcoming_socket, char receive_buffer[], SocketList* connection_sockets_ptr, OutgoingBuffer* outgoing_buffer_ptr) {
    // Mutual exclusion not needed because connection_sockets list is only accessed in main thread

    SocketListNode* walker = (*connection_sockets_ptr).head;
    while (walker) {
        int return_value = receive_command(welcoming_socket, (*walker).socket_ptr, topics, num_of_topics, receive_buffer, connection_sockets_ptr, &walker);
        if (return_value == RECEIVED) {
            char* response = execute_command(topics, num_of_topics, receive_buffer, (*walker).socket_ptr);
            if (response) {
                OneOrMoreSockets one_or_more_sockets;
                one_or_more_sockets.cs_union.connection_socket = *((*walker).socket_ptr);
                one_or_more_sockets.more = false;
                OutgoingBufferElement new_element = { response, one_or_more_sockets };

                produce_new_message(outgoing_buffer_ptr, new_element);
            }
        }

        // If the current walker node has been deleted, "walker = (*walker).next;" has been done inside receive_command(), so it shouldn't be done again
        if (return_value != DELETED) {
            walker = (*walker).next;
        }
    }
}

