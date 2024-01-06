#include <winsock2.h>

typedef struct SubscriberListNodeStruct {
    SOCKET* connection_socket_ptr;
    struct SubscriberListNodeStruct* next;
} SubscriberListNode;

typedef struct SubscriberList {
    SubscriberListNode* head;  // pointer to the first element
    int size;
} SubscriberList;


void add_to_start(SubscriberList* ptr_to_list, SOCKET* connection_socket_ptr);

