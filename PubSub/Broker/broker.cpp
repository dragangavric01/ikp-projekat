#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int main() {
    Node* head = NULL;

    print_queue(head);
    add_to_end(&head, "jedan");
    print_queue(head);
    add_to_end(&head, "dva");
    print_queue(head);
    add_to_end(&head, "tri");
    print_queue(head);
    add_to_end(&head, "cetiri");
    print_queue(head);

    for (int i = 0; i < 4; i++) {
        char* first_message = read_and_delete_first(&head);
        print_queue(head);
        printf("%s\n", first_message);
        free(first_message);
    }

    return 0;
}
