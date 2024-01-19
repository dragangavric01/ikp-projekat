#pragma once

typedef struct MessageQueueNodeStruct {
    char* message;
    struct MessageQueueNodeStruct* next;
} MessageQueueNode;

typedef struct MessageQueueStruct {
    MessageQueueNode* head;  // pointer to the first element
    MessageQueueNode* tail;  // pointer to the last element
} MessageQueue;


// Adds message to the end of the queue
void enqueue(MessageQueue* ptr_to_message_queue, const char* message);

// Reads the first node message and deletes the node. Returns NULL if the queue is empty. Caller needs to free the memory pointed by the return value 
char* dequeue(MessageQueue* ptr_to_message_queue);

void free_message_queue(MessageQueue message_queue);

void print_message_queue(MessageQueue message_queue);
