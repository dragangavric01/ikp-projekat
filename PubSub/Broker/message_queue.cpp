#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message_queue.h"


void enqueue(MessageQueue* ptr_to_message_queue, const char* message) {
    MessageQueueNode** ptr_to_head = &((*ptr_to_message_queue).head);
    MessageQueueNode** ptr_to_tail = &((*ptr_to_message_queue).tail);
    int message_length = strlen(message);

    if (!(*ptr_to_head)) {  // queue is empty
        *ptr_to_head = (MessageQueueNode*)malloc(sizeof(MessageQueueNode));
        *ptr_to_tail = *ptr_to_head;  // there is only one element, so both head and tail point to it

        (**ptr_to_head).message = (char*)malloc(message_length + 1);
        strcpy_s((**ptr_to_head).message, message_length + 1, message);
        
        (**ptr_to_head).next = NULL;
    } else {
        (**ptr_to_tail).next = (MessageQueueNode*)malloc(sizeof(MessageQueueNode));
        *ptr_to_tail = (**ptr_to_tail).next;

        (**ptr_to_tail).message = (char*)malloc(message_length + 1);
        strcpy_s((**ptr_to_tail).message, message_length + 1, message);

        (**ptr_to_tail).next = NULL;
    }
}

char* dequeue(MessageQueue* ptr_to_message_queue) {
    MessageQueueNode** ptr_to_head = &((*ptr_to_message_queue).head);
    MessageQueueNode** ptr_to_tail = &((*ptr_to_message_queue).tail);

    if (!(*ptr_to_head)) {  // queue is empty
        return NULL;
    }

    char* first_message = (char*)malloc(sizeof((**ptr_to_head).message));
    strcpy_s(first_message, strlen((**ptr_to_head).message) + 1, (**ptr_to_head).message);
    
    MessageQueueNode* head_temp = *ptr_to_head;
    *ptr_to_head = (**ptr_to_head).next;
    free(head_temp);

    return first_message;
}

void free_message_queue(MessageQueue message_queue) {
    if (!(message_queue.head)) {
        return;
    }

    MessageQueueNode* previous_ptr;
    while (message_queue.head) {
        previous_ptr = message_queue.head;
        message_queue.head = (*(message_queue.head)).next;
        free(previous_ptr);
    }
}


void print_message_queue(MessageQueue message_queue) {
    if (!(message_queue.head)) {
        printf("\tThe message queue is empty\n\n");
        return;
    }

    int i = 1;
    while (message_queue.head) {
        printf("\t%d. %s\n", i, (*(message_queue.head)).message);

        message_queue.head = (*(message_queue.head)).next;
        i++;
    }

    puts("");
}