#include "networking_broker.h"
#include "common.h"  // windows.h must be included after winsock2.h

#define TOPICS_NUM 3


void receive_and_execute_commands(Topic topics[], int num_of_topics, SOCKET welcoming_socket, char receive_buffer[], SocketList connection_sockets);

void send_messages_to_subscribers(Topic* topic_ptr);


int main(int argc, char* argv[]) {
    if (argc > 1) {
        window_setup(argv);
    }

    char receive_buffer[MAX_COMMAND_SIZE];
    SOCKET welcoming_socket = INVALID_SOCKET;

    Topic topics[TOPICS_NUM];
    topics[0] = initialize_topic("updates");
    topics[1] = initialize_topic("news");
    topics[2] = initialize_topic("warnings");

    SocketList connection_sockets = { NULL, 0 };

    setup(&welcoming_socket);

    while (true) {
        if (!accept_connection(welcoming_socket, topics, TOPICS_NUM, &connection_sockets)) {
            break;
        }

        receive_and_execute_commands(topics, TOPICS_NUM, welcoming_socket, receive_buffer, connection_sockets);
    }

    cleanup(welcoming_socket, topics, TOPICS_NUM, connection_sockets);
    return 0;
}

void receive_and_execute_commands(Topic topics[], int num_of_topics, SOCKET welcoming_socket, char receive_buffer[], SocketList connection_sockets) {
    while (connection_sockets.head) {
        if (receive_command(welcoming_socket, (*(connection_sockets.head)).socket, topics, num_of_topics, receive_buffer, connection_sockets)) {
            char* response = execute_command(topics, num_of_topics, receive_buffer, (*(connection_sockets.head)).socket);
            if (response) {
                send_to_client((*(connection_sockets.head)).socket, response);
            }
        }

        connection_sockets.head = (*(connection_sockets.head)).next;
    }
}

void send_messages_to_subscribers(Topic* topic_ptr) {  // Pointer to topic because the heads of data structures will change in the main thread, so I don't want to have old copies
    // treba condition variable da koristim 
    // treba condition variable da koristim 
    // treba condition variable da koristim 

    SocketListNode** ptr_to_head = &((*topic_ptr).subscriber_connection_sockets.head);
    
    SocketListNode* walker;
    while (true) {
        char* message = dequeue(&((*topic_ptr).message_queue));

        walker = *ptr_to_head;
        while (walker) {
            send_to_client((*walker).socket, message);
            walker = (*walker).next;
        }
    }
}