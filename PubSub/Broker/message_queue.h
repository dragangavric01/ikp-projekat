#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"


typedef struct MessageQueueNodeStruct {
    char* message;
    struct MessageQueueNodeStruct* next;
} MessageQueueNode;

typedef struct MessageQueueStruct {
    MessageQueueNode* head;  // pointer to the first element
    MessageQueueNode* tail;  // pointer to the last element
    CRITICAL_SECTION* crit_section_ptr;
    CONDITION_VARIABLE* cond_var_ptr;
} MessageQueue;


void initialize_message_queue(MessageQueue* ptr_to_message_queue);

void enqueue(MessageQueue* message_queue_ptr, char* message, const char* topic_name);

char* dequeue(MessageQueue* message_queue_ptr);

void free_message_queue(MessageQueue* message_queue_ptr);
