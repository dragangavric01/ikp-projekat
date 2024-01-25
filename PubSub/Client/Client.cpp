#include <time.h>
#include  <iostream> 
#include "networking_client.h"
#include "command_creation.h"
#include "stress_test.h"
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
    char receive_buffer[CLIENT_RECEIVE_BUFFER_SIZE];
    memset(receive_buffer, 0, CLIENT_RECEIVE_BUFFER_SIZE);

    bool connected = false;
    sockaddr_in broker_data;
    SOCKET client_socket;

    Sleep(5000);  // wait until broker is ready

    setup(&broker_data, &client_socket);

    if (argc > 1 && argv[5][0] != '0') {
        test(argv[5][0], client_socket, &connected, &broker_data, receive_buffer);

        cleanup(client_socket);
        return 0;
    }

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

// Receives all subscription messages that haven't been received yet
static void receive_subscription_messages(SOCKET client_socket, char receive_buffer[]) {
    bool blocking = false;
    int message_number = 1;

    blocking = receive_from_broker(client_socket, receive_buffer, false);
    if (!blocking) {
        printf("Subscription messages:");

        int i = 0;
        while (true) {
            if (receive_buffer[i] == '#') {
                printf("\n    %d. ", message_number);
                message_number++;
                i++;

                while (receive_buffer[i] != '\0') {
                    printf("%c", receive_buffer[i]);
                    i++;
                }

                i++;
            } else {
                break;
            }
        }

        puts("");

        memset(receive_buffer, 0, CLIENT_RECEIVE_BUFFER_SIZE);
    }

    if (message_number == 1) {
        puts("No subscription messages have been received\n");
        return;
    }
}

static char get_option() {
    char *option = (char*)malloc(2);

    puts("\n***************************************");

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

        puts("");

        char* command = create_command(option, topic, message);
        bool return_value = send_command(client_socket, command);
        free(command);

        return return_value;
    } else if (option == '3') {
        printf("Topic: ");
        gets_s(topic, MAX_TOPIC_SIZE);

        if (!(*connected_ptr)) {
            puts("Not connected\n");
            return true;
        }

        puts("");

        char* command = create_command(option, topic, NULL);
        bool return_value = send_command(client_socket, command);
        free(command);

        return return_value;
    } else if (option == '4') {
        printf("Topic: ");
        gets_s(topic, MAX_TOPIC_SIZE);

        if (!(*connected_ptr)) {
            puts("Not connected\n");
            return true;
        }
        
        puts("");

        char* command = create_command(option, topic, NULL);
        bool return_value = send_command(client_socket, command);
        free(command);

        receive_from_broker(client_socket, receive_buffer, true);

        if (receive_buffer[0] == '1') {
            printf("Topic '%s' exists\n\n", topic);
        } else {
            printf("Topic '%s' does not exist\n\n", topic);
        }

        return return_value;
    } else if (option == '5') {
        printf("Topic: ");
        gets_s(topic, MAX_TOPIC_SIZE);

        if (!(*connected_ptr)) {
            puts("Not connected");
            return true;
        }

        puts("");

        char* command = create_command(option, topic, NULL);
        bool return_value = send_command(client_socket, command);
        free(command);

        receive_from_broker(client_socket, receive_buffer, true);
        if (receive_buffer[0] == '#') {
            printf("Topic '%s' does not exist\n\n", topic);
        } else {
            int subscriber_number = atoi(receive_buffer);
            printf("There are %d subscribers for topic '%s'\n\n", subscriber_number, topic);
        }

        return return_value;
    } else if (option == '6') {
        return true;
    } else if (option == '7') {
        return false;
    } else {
        puts("Invalid input.\n\n");
    }

    return true;
}
