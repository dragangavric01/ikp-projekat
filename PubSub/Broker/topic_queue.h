
typedef struct TopicQueueNodeStruct {
    char* message;
    struct TopicQueueNodeStruct* next;
} TopicQueueNode;

typedef struct TopicQueuePointersStruct {
    TopicQueueNode* head;  // pointer to the first element
    TopicQueueNode* tail;  // pointer to the last element
} TopicQueuePointers;


void init_queue(TopicQueuePointers* ptr_to_queue_pointers);

// Adds message to the end of the queue
void enqueue(TopicQueuePointers* ptr_to_queue_pointers, const char* message);

// Reads the first node message and deletes the node. Returns NULL if the queue is empty. Caller needs to free the memory pointed by the return value 
char* dequeue(TopicQueuePointers* ptr_to_queue_pointers);

void print_queue(TopicQueuePointers queue_pointers);