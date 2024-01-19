#include "networking_client.h"
#include "command_creation.h"
#include "common.h"  // windows.h must be included after winsock2.h


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

    Sleep(1000);  // wait until broker is ready

    setup(&broker_data, &client_socket);

    while (true) {
        char option = get_option();

        if (!execute_requested_action(topic, message, &connected, &broker_data, option, receive_buffer, client_socket)) {
            break;
        }
    }

    cleanup(client_socket);
    return 0;
}

static void receive_messages() {
    // Posebna nit treba da izvrsava ovu funkciju i treba napraviti da pozivanje recv() bude medjusobno iskljucivo
    // Ova funkcija treba da prima poruke iz topic queue-ova na koje je klijent sub-ovan
    // Posto treba da ispisuje te poruke, treba napraviti message queue u koji se stavljaju primljene poruke jer ako je pozvan scanf iz main niti, a iz ove niti se pozove printf, nece se ispisati printf. Pa onda kada korisnik zavrsi sa unosom, ispisu se sve poruke iz queue-a. Samo onda pazi da pristupanje queue-u bude medjusobno iskljucivo (ili mozda cak conditional variable?)
    // Kad se pokrece sa skriptom, nece biti unosa sa scanf, ali treba da radi i kad se pokrece bez skripte
    // Pozivi printf i scanf treba da budu medjusobno iskljucivi
}

static void test() {
    // TODO: Treba da poziva random metode i da sleep-uje posle svake tipa jednu sekundu 
}

static char get_option() {
    char option[2];

    puts("\n***************************************\n");

    puts("Choose an option\n"
        "\t1) Connect to the Broker\n"
        "\t2) Publish a message\n"
        "\t3) Subscribe to a topic\n"
        "\t4) Check if a topic exists\n"
        "\t5) Get the number of subscribers for a topic\n"
        "\t6) Exit\n");

    printf("\tOption: ");
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
        printf("\tTopic: ");
        gets_s(topic, MAX_TOPIC_SIZE);

        printf("\tMessage: ");
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
        printf("\tTopic: ");
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
        printf("\tTopic: ");
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

        receive_from_broker(client_socket, receive_buffer);

        if (receive_buffer[0] == '1') {
            printf("Topic '%s' exists\n", topic);
        } else {
            printf("Topic '%s' does not exist\n", topic);
        }
    } else if (option == '5') {
        printf("\tTopic: ");
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

        receive_from_broker(client_socket, receive_buffer);
        int subscriber_number = atoi(receive_buffer);
        printf("There are %d subscribers for topic '%s'\n", subscriber_number, topic);
    } else if (option == '6') {
        return false;
    } else {
        puts("Invalid input.");
    }

    return true;
}