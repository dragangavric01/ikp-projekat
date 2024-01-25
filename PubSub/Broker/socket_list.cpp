#include "socket_list.h"


void initialize_socket_list(SocketList* ptr_to_list, bool mutually_exclusive) {
	(*ptr_to_list).head = NULL;
	(*ptr_to_list).size = 0;

	// In connection_sockets there is no need for mutual exclusion since it's only accessed in the main thread
	if (mutually_exclusive) {
		(*ptr_to_list).crit_section_ptr = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
		InitializeCriticalSection((*ptr_to_list).crit_section_ptr);
	} else {
		(*ptr_to_list).crit_section_ptr = NULL;
	}
}

// In connection_sockets there is no need for mutual exclusion since it's only accessed in the main thread
static void enter_cs(SocketList* ptr_to_list) {
	if ((*ptr_to_list).crit_section_ptr) {
		EnterCriticalSection((*ptr_to_list).crit_section_ptr);
	}
}

// In connection_sockets there is no need for mutual exclusion since it's only accessed in the main thread
static void leave_cs(SocketList* ptr_to_list) {
	if ((*ptr_to_list).crit_section_ptr) {
		LeaveCriticalSection((*ptr_to_list).crit_section_ptr);
	}
}

// Adds new socket to the start of the list
void add_to_start(SocketList* ptr_to_list, SOCKET* socket_ptr) {
	enter_cs(ptr_to_list);

	SocketListNode** ptr_to_head = &((*ptr_to_list).head);

	if (!(*ptr_to_head)) {  // list is empty
		*ptr_to_head = (SocketListNode*)malloc(sizeof(SocketListNode));

		(**ptr_to_head).socket_ptr = socket_ptr;
		(**ptr_to_head).next = NULL;
	} else {
		SocketListNode* old_head = *ptr_to_head;
		
		*ptr_to_head = (SocketListNode*)malloc(sizeof(SocketListNode));

		(**ptr_to_head).socket_ptr = socket_ptr;
		(**ptr_to_head).next = old_head;
	}

	(*ptr_to_list).size++;

	leave_cs(ptr_to_list);
}

// Deletes a node
void free_node(SocketList* ptr_to_list, SOCKET* socket_ptr) {
	enter_cs(ptr_to_list);

	SocketListNode** ptr_to_head = &((*ptr_to_list).head);

	if (!(*ptr_to_head)) {

		leave_cs(ptr_to_list);
		return;
	}

	if ((**ptr_to_head).socket_ptr == socket_ptr) {
		SocketListNode* old_head = *ptr_to_head;
		*ptr_to_head = (**ptr_to_head).next;
		free(old_head);

		(*ptr_to_list).size--;
	} else {
		SocketListNode* previous = *ptr_to_head;
		SocketListNode* walker = (*previous).next;
		while (walker) {
			if ((*walker).socket_ptr == socket_ptr) {
				(*previous).next = (*walker).next;
				free(walker);
				(*ptr_to_list).size--;

				leave_cs(ptr_to_list);
				return;
			}

			previous = walker;
			walker = (*walker).next;
		}
	}

	leave_cs(ptr_to_list);
}

// Closes all sockets and deletes all nodes
void close_sockets_and_free_socket_list(SocketList* ptr_to_list) {
	enter_cs(ptr_to_list);

	if (!((*ptr_to_list).head)) {
		leave_cs(ptr_to_list);
		return;
	}

	SocketListNode* previous_ptr;
	while ((*ptr_to_list).head) {
		previous_ptr = (*ptr_to_list).head;
		(*ptr_to_list).head = (*((*ptr_to_list).head)).next;

		if (*((*previous_ptr).socket_ptr) != INVALID_SOCKET) {  // If it hasn't been closed already in some other list
			closesocket(*((*previous_ptr).socket_ptr));
			free((*previous_ptr).socket_ptr);
			*((*previous_ptr).socket_ptr) = INVALID_SOCKET;  // So I know it has been closed when I see it in other lists
		}

		free(previous_ptr);
	}

	leave_cs(ptr_to_list);
}

// Prints the whole list. It should only be called after EnterCriticalSection() for the SocketList critical section is called.
void print_socket_list_unsafe(SocketList* ptr_to_list) {
	// There is no printf crit section being entered because it's done before calling this function

	enter_cs(ptr_to_list);

	if (!((*ptr_to_list).head)) {
		printf("The socket list is empty\n\n");
		leave_cs(ptr_to_list);

		return;
	}

	SocketListNode* walker = (*ptr_to_list).head;

	while (walker) {
		printf("%llu ", *((*walker).socket_ptr));
		walker = (*walker).next;
	}

	puts("\n");

	leave_cs(ptr_to_list);
}
