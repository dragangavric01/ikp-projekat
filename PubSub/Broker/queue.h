
typedef struct NodeStruct {
    char* message;
    struct NodeStruct* next;
} Node;

typedef struct QueuePointers {
    Node* head;  // pointer to the first element
    Node* tail;  // pointer to the last element
} QueuePointers;

// Adds message to the end of the queue
void enqueue(QueuePointers* ptr_to_queue_pointers, const char* message);

// Reads the first node message and deletes the node. Returns NULL if the queue is empty. Caller needs to free the memory pointed by the return value 
char* dequeue(QueuePointers* ptr_to_queue_pointers);

void print_queue(QueuePointers queue_pointers);