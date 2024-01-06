#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

void enqueue(TopicQueuePointers* ptr_to_queue_pointers, const char* message) {
    TopicQueueNode** ptr_to_head = &((*ptr_to_queue_pointers).head);
    TopicQueueNode** ptr_to_tail = &((*ptr_to_queue_pointers).tail);

    if (!(*ptr_to_head)) {  // queue is empty
        *ptr_to_head = (TopicQueueNode*)malloc(sizeof(TopicQueueNode));
        *ptr_to_tail = *ptr_to_head;  // there is only one element, so both head and tail point to it

        (**ptr_to_head).message = (char*)malloc(sizeof(message));
        strcpy_s((**ptr_to_head).message, sizeof(message), message);
        
        (**ptr_to_head).next = NULL;
    } else {
        (**ptr_to_tail).next = (TopicQueueNode*)malloc(sizeof(TopicQueueNode));
        *ptr_to_tail = (**ptr_to_tail).next;

        (**ptr_to_tail).message = (char*)malloc(sizeof(message));
        strcpy_s((**ptr_to_tail).message, sizeof(message), message);

        (**ptr_to_tail).next = NULL;
    }
}

char* dequeue(TopicQueuePointers* ptr_to_queue_pointers) {
    TopicQueueNode** ptr_to_head = &((*ptr_to_queue_pointers).head);
    TopicQueueNode** ptr_to_tail = &((*ptr_to_queue_pointers).tail);

    if (!(*ptr_to_head)) {  // queue is empty
        return NULL;
    }

    char* first_message = (char*)malloc(sizeof((**ptr_to_head).message));
    strcpy_s(first_message, sizeof((**ptr_to_head).message), (**ptr_to_head).message);
    
    TopicQueueNode* head_temp = *ptr_to_head;
    *ptr_to_head = (**ptr_to_head).next;
    free(head_temp);

    return first_message;
}

void print_queue(TopicQueuePointers queue_pointers) {
    if (!(queue_pointers.head)) {
        printf("The queue is empty\n\n");
        return;
    }

    printf("################################\n");
    while (queue_pointers.head) {
        printf("\t%s\n", (*(queue_pointers.head)).message);
        queue_pointers.head = (*(queue_pointers.head)).next;
    }
    printf("################################\n\n");
}