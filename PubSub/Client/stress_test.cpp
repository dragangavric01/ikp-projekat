#include "stress_test.h"


static void subscribe_to_topic(const char* topic, SOCKET client_socket) {
    char* command = create_command('3', topic, NULL);
    bool return_value = send_command(client_socket, command);

    free(command);

    if (!return_value) {
        exit(1);
    }
}

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
    }

    if (message_number == 1) {
        puts("No subscription messages have been received");
        return;
    }

    puts("");
}

static int get_random_number(int min_number, int max_number) {
    if (min_number > RAND_MAX || max_number > RAND_MAX) {
        printf("error in get_random_number function (number bigger than RAND_MAX)");

        return min_number;
    }
    return rand() % (max_number + 1 - min_number) + min_number;
}

static void publish_9_messages_and_receive(char client_number, SOCKET client_socket, char receive_buffer[]) {
    const char* news = "news";
    const char* updates = "updates";
    const char* warnings = "warnings";

    const char* topic_name;
    char message_num_buffer[2];
    
    int message_num = 1;
    while (message_num < 10) {
        if (rand() < (RAND_MAX / 3)) {
            topic_name = news;
        } else if (rand() < (2 * RAND_MAX / 3)) {
            topic_name = updates;
        } else {
            topic_name = warnings;
        }

        _itoa_s(message_num, message_num_buffer, 2, 10);
        char* message = (char*)malloc(5);
        message[0] = 'c';
        message[1] = toupper(client_number);
        message[2] = 'm';
        message[3] = message_num_buffer[0];
        message[4] = '\0';

        char* command = create_command('2', topic_name, message);
        bool return_value = send_command(client_socket, command);
        printf("Command '%s' sent\n\n", command + 1);

        free(message);
        free(command);

        if (!return_value) {
            exit(1);
        }

        receive_subscription_messages(client_socket, receive_buffer);
        puts("");

        Sleep(get_random_number(1000, 2000));

        message_num++;
    }
}

void test(char client_number, SOCKET client_socket, bool* connected_ptr, sockaddr_in* broker_data_ptr, char receive_buffer[]) {
    time_t t;
    srand((unsigned)time(&t));
    connect_to_broker(client_socket, broker_data_ptr, connected_ptr);
    puts("Connected\n");

    if (client_number == '1') {
        system("title Subbed to: N");
        subscribe_to_topic("news", client_socket);
    } else if (client_number == '2') {
        system("title Subbed to: N/U");

        subscribe_to_topic("news", client_socket);
        subscribe_to_topic("updates", client_socket);
    } else if (client_number == '3') {
        system("title Subbed to: N/W");

        subscribe_to_topic("news", client_socket);
        subscribe_to_topic("warnings", client_socket);
    } else if (client_number == '4') {
        system("title Subbed to: N/U/W");

        subscribe_to_topic("news", client_socket);
        subscribe_to_topic("updates", client_socket);
        subscribe_to_topic("warnings", client_socket);
////////////////////////////////////////////////////
    } else if (client_number == '5') {
        system("title Subbed to: U");
        subscribe_to_topic("updates", client_socket);
    } else if (client_number == '6') {
        system("title Subbed to: U/W");

        subscribe_to_topic("updates", client_socket);
        subscribe_to_topic("warnings", client_socket);
    } else if (client_number == '7') {
        system("title Subbed to: N/U");

        subscribe_to_topic("updates", client_socket);
        subscribe_to_topic("news", client_socket);
    } else if (client_number == '8') {
        system("title Subbed to: N/U/W");
        
        subscribe_to_topic("updates", client_socket);
        subscribe_to_topic("updates", client_socket);
        subscribe_to_topic("warnings", client_socket);
////////////////////////////////////////////////////
    } else if (client_number == '9') {
        system("title Subbed to: W");
        subscribe_to_topic("warnings", client_socket);
    } else if (client_number == 'a') {
        system("title Subbed to: U/W");

        subscribe_to_topic("warnings", client_socket);
        subscribe_to_topic("updates", client_socket);
    } else if (client_number == 'b') {
        system("title Subbed to: N/W");

        subscribe_to_topic("warnings", client_socket);
        subscribe_to_topic("news", client_socket);
    } else if (client_number == 'c') {
        system("title Subbed to: N/U/W");
        
        subscribe_to_topic("warnings", client_socket);
        subscribe_to_topic("updates", client_socket);
        subscribe_to_topic("warnings", client_socket);
    }

    puts("Subscribed to topics\n\n");
    Sleep(get_random_number(20000, 24000));

    publish_9_messages_and_receive(client_number, client_socket, receive_buffer);
    Sleep(5000);
    receive_subscription_messages(client_socket, receive_buffer);  // receive the rest

    puts("\nPress any key to exit");
    getchar();
}