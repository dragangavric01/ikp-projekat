
#include "message_queue.h"

void initialize_message_queue(MessageQueue* ptr_to_message_queue) {
    (*ptr_to_message_queue).head = NULL;
    (*ptr_to_message_queue).tail = NULL;

    (*ptr_to_message_queue).crit_section_ptr = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
    (*ptr_to_message_queue).cond_var_ptr = (CONDITION_VARIABLE*)malloc(sizeof(CONDITION_VARIABLE));

    InitializeCriticalSection((*ptr_to_message_queue).crit_section_ptr);
    InitializeConditionVariable((*ptr_to_message_queue).cond_var_ptr);
}

void print_message_queue_unsafe(MessageQueue* message_queue_ptr) {
    // There is no printf crit section being entered because it's done before calling this function
    
    EnterCriticalSection((*message_queue_ptr).crit_section_ptr);

    if (!((*message_queue_ptr).head)) {
        printf("    The message queue is empty\n\n");
        LeaveCriticalSection((*message_queue_ptr).crit_section_ptr);
        return;
    }

    int i = 1;
    MessageQueueNode* walker = (*message_queue_ptr).head;

    while (walker) {
        printf("    %d. %s\n", i, (*walker).message + 1);  // // +1 because I don't want to show the '#' at the beginning

        walker = (*walker).next;
        i++;
    }

    LeaveCriticalSection((*message_queue_ptr).crit_section_ptr);

    puts("");
}

// Adds message to the end of the queue
static void enqueue_unsafe(MessageQueue* ptr_to_message_queue, const char* message) {
    MessageQueueNode** ptr_to_head = &((*ptr_to_message_queue).head);
    MessageQueueNode** ptr_to_tail = &((*ptr_to_message_queue).tail);
    int message_length = strlen(message);

    if (!(*ptr_to_head)) {  // queue is empty
        *ptr_to_head = (MessageQueueNode*)malloc(sizeof(MessageQueueNode));
        *ptr_to_tail = *ptr_to_head;  // there is only one element, so both head and tail point to it

        (**ptr_to_head).message = (char*)message;
        (**ptr_to_head).next = NULL;
    } else {
        (**ptr_to_tail).next = (MessageQueueNode*)malloc(sizeof(MessageQueueNode));
        *ptr_to_tail = (**ptr_to_tail).next;

        (**ptr_to_head).message = (char*)message;
        (**ptr_to_tail).next = NULL;
    }
}

// Reads the first node message and deletes the node. Returns NULL if the queue is empty. Caller needs to free the memory pointed by the return value 
static char* dequeue_unsafe(MessageQueue* ptr_to_message_queue) {
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

void enqueue(MessageQueue* message_queue_ptr, char* message, const char* topic_name) {
    EnterCriticalSection((*message_queue_ptr).crit_section_ptr);

    enqueue_unsafe(message_queue_ptr, message);
    WakeConditionVariable((*message_queue_ptr).cond_var_ptr);

    EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
    printf("Message '%s' enqueued (topic '%s')\n", message + 1, topic_name);  // +1 because I don't want to show the '#' at the beginning
    printf("Topic '%s' message_queue:\n", topic_name);
    print_message_queue_unsafe(message_queue_ptr);
    LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));

    LeaveCriticalSection((*message_queue_ptr).crit_section_ptr);
}

char* dequeue(MessageQueue* message_queue_ptr) {
    EnterCriticalSection((*message_queue_ptr).crit_section_ptr);

    while (!((*message_queue_ptr).head)) {
        SleepConditionVariableCS((*message_queue_ptr).cond_var_ptr, (*message_queue_ptr).crit_section_ptr, INFINITE);
    }

    char* message = dequeue_unsafe(message_queue_ptr);

    LeaveCriticalSection((*message_queue_ptr).crit_section_ptr);

    return message;
}

void free_message_queue(MessageQueue* message_queue_ptr) {
    EnterCriticalSection((*message_queue_ptr).crit_section_ptr);
    
    if (!((*message_queue_ptr).head)) {

        LeaveCriticalSection((*message_queue_ptr).crit_section_ptr);
        return;
    }

    MessageQueueNode* walker = (*message_queue_ptr).head;
    MessageQueueNode* previous_ptr;
    while (walker) {
        previous_ptr = walker;
        walker = (*walker).next;
        free(previous_ptr);
    }

    LeaveCriticalSection((*message_queue_ptr).crit_section_ptr);
}
