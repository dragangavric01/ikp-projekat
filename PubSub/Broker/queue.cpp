#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

#define _CRT_SECURE_NO_WARNINGS 1

void add_to_end(Node** ptr_to_head, const char* message) {
    if (!(*ptr_to_head)) {  // queue is empty
        *ptr_to_head = (Node*)malloc(sizeof(Node));

        (**ptr_to_head).message = (char*)malloc(sizeof(message));
        strcpy_s((**ptr_to_head).message, sizeof(message), message);
        
        (**ptr_to_head).next = NULL;
    } else {
        Node* walker = *ptr_to_head;
        while ((*walker).next) {  // go to the last node
            walker = (*walker).next;
        }

        (*walker).next = (Node*)malloc(sizeof(Node));

        (*((*walker).next)).message = (char*)malloc(sizeof(message));
        strcpy_s((*((*walker).next)).message, sizeof(message), message);

        (*((*walker).next)).next = NULL;
    }
}

char* read_and_delete_first(Node** ptr_to_head) {
    if (!(*ptr_to_head)) {  // queue is empty
        return NULL;
    }

    Node* first_node_ptr = *ptr_to_head;

    char* first_message = (char*)malloc(sizeof((*first_node_ptr).message));
    strcpy_s(first_message, sizeof(first_message), (*first_node_ptr).message);
    
    *ptr_to_head = (*first_node_ptr).next;
    free(first_node_ptr);

    return first_message;
}

void print_queue(Node* head) {
    if (!head) {
        printf("The queue is empty\n\n");
        return;
    }

    printf("################################\n");
    while (head) {
        printf("\t%s\n", (*head).message);
        head = (*head).next;
    }
    printf("################################\n\n");
}