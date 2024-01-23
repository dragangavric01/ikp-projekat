#include <time.h>
#include "networking_client.h"
#include "command_creation.h"
#include "common.h"  // windows.h must be included after winsock2.h

static void receive_subscription_messages(SOCKET client_socket, char receive_buffer[]);

static char get_option();

static bool execute_requested_action(char topic[], char message[], bool* connected_ptr, sockaddr_in* broker_data_ptr, char option, char receive_buffer[], SOCKET client_socket);


int main(int argc, char* argv[]) {
    if (argc > 1) {
        window_setup(argv);
    }
    
    char topic[MAX_TOPIC_SIZE];
    char message[MAX_MESSAGE_SIZE];
    char receive_buffer[MAX_MESSAGE_SIZE];
    bool connected = false;
    sockaddr_in broker_data;
    SOCKET client_socket;

    Sleep(2000);  // wait until broker is ready

    setup(&broker_data, &client_socket);

    while (true) {
        char option = get_option();

        if (!execute_requested_action(topic, message, &connected, &broker_data, option, receive_buffer, client_socket)) {
            break;
        }

        receive_subscription_messages(client_socket, receive_buffer);
    }

    cleanup(client_socket);
    return 0;
}

static void receive_subscription_messages(SOCKET client_socket, char receive_buffer[]) {
    bool none_received = true;
    bool blocking = false;
    
    do {
        blocking = receive_from_broker(client_socket, receive_buffer, false);
        if (!blocking) {
            printf("Subscription message: %s\n", receive_buffer);
            none_received = false;
        }
    } while (!blocking);

    if (none_received) {
        puts("No subscription messages have been received");
    }
}

static char get_option() {
    char option[2];

    puts("\n***************************************\n");

    puts("Choose an option\n"
        "1) Connect to the Broker\n"
        "2) Publish a message\n"
        "3) Subscribe to a topic\n"
        "4) Check if a topic exists\n"
        "5) Get the number of subscribers for a topic\n"
        "6) Read subscription messages that may have been received\n"
        "7) Exit\n");

    printf("Option: ");
    gets_s(option, 2);

    puts("");

    return option[0];
}

// topic and message size must be of size MAX_TOPIC_SIZE and MAX_MESSAGE_SIZE
static bool execute_requested_action(char topic[], char message[], bool* connected_ptr, sockaddr_in* broker_data_ptr, char option, char receive_buffer[], SOCKET client_socket) {
    if (option == '1') {
        if (*connected_ptr) {
            puts("Already connected\n");
            return true;
        }

        if (!connect_to_broker(client_socket, broker_data_ptr, connected_ptr)) {
            return false;
        }

        puts("Connected to broker\n");
    } else if (option == '2') {
        printf("Topic: ");
        gets_s(topic, MAX_TOPIC_SIZE);

        printf("Message: ");
        gets_s(message, MAX_MESSAGE_SIZE);

        if (!(*connected_ptr)) {
            puts("Not connected\n");
            return true;
        }

        char* command = create_command(option, topic, message);
        if (!send_command(client_socket, command)) {
            return false;
        }
    } else if (option == '3') {
        printf("Topic: ");
        gets_s(topic, MAX_TOPIC_SIZE);
        puts("");

        if (!(*connected_ptr)) {
            puts("Not connected\n");
            return true;
        }

        char* command = create_command(option, topic, NULL);
        if (!send_command(client_socket, command)) {
            return false;
        }
    } else if (option == '4') {
        printf("Topic: ");
        gets_s(topic, MAX_TOPIC_SIZE);
        puts("");

        if (!(*connected_ptr)) {
            puts("Not connected\n");
            return true;
        }

        char* command = create_command(option, topic, NULL);
        if (!send_command(client_socket, command)) {
            return false;
        }

        receive_from_broker(client_socket, receive_buffer, true);

        if (receive_buffer[0] == '1') {
            printf("Topic '%s' exists\n", topic);
        } else {
            printf("Topic '%s' does not exist\n", topic);
        }
    } else if (option == '5') {
        printf("Topic: ");
        gets_s(topic, MAX_TOPIC_SIZE);
        puts("");

        if (!(*connected_ptr)) {
            puts("Not connected\n");
            return true;
        }

        char* command = create_command(option, topic, NULL);
        if (!send_command(client_socket, command)) {
            return false;
        }

        receive_from_broker(client_socket, receive_buffer, true);
        if (receive_buffer[0] == '#') {
            printf("Topic '%s' does not exist\n", topic);
        } else {
            int subscriber_number = atoi(receive_buffer);
            printf("There are %d subscribers for topic '%s'\n", subscriber_number, topic);
        }
    } else if (option == '6') {
        return true;
    } else if (option == '7') {
        return false;
    } else {
        puts("Invalid input.");
    }

    return true;
}