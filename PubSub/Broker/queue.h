
typedef struct NodeStruct {
    char* message;
    NodeStruct* next;
} Node;


void add_to_end(Node** ptr_to_head, const char* message);

// Caller treba da posle free-uje memoriju na koju pokazuje return value
char* read_and_delete_first(Node** ptr_to_head);

void print_queue(Node* head);
