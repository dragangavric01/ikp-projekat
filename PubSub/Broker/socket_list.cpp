#include <stdio.h>
#include <stdlib.h>
#include "socket_list.h"


void add_to_start(SocketList* ptr_to_list, SOCKET socket) {
	SocketListNode** ptr_to_head = &((*ptr_to_list).head);

	if (!(*ptr_to_head)) {  // list is empty
		*ptr_to_head = (SocketListNode*)malloc(sizeof(SocketListNode));

		(**ptr_to_head).socket = socket;
		(**ptr_to_head).next = NULL;
	} else {
		SocketListNode* old_head = *ptr_to_head;
		
		*ptr_to_head = (SocketListNode*)malloc(sizeof(SocketListNode));

		(**ptr_to_head).socket = socket;
		(**ptr_to_head).next = old_head;
	}

	(*ptr_to_list).size++;
}


void delete_socket(SocketList* ptr_to_list, SOCKET socket) {
	SocketListNode** ptr_to_head = &((*ptr_to_list).head);

	if (!(*ptr_to_head)) {
		return;
	}

	if ((**ptr_to_head).socket == socket) {
		SocketListNode* old_head = *ptr_to_head;
		*ptr_to_head = (**ptr_to_head).next;
		free(old_head);

		(*ptr_to_list).size--;
	} else {
		SocketListNode* previous = *ptr_to_head;
		SocketListNode* walker = (*previous).next;
		while (walker) {
			if ((*walker).socket == socket) {
				(*previous).next = (*walker).next;
				free(walker);
				(*ptr_to_list).size--;

				return;
			}

			previous = walker;
			walker = (*walker).next;
		}
	}
}

void close_sockets_and_free_socket_list(SocketList socket_list) {
	if (!(socket_list.head)) {
		return;
	}

	SocketListNode* previous_ptr;
	while (socket_list.head) {
		previous_ptr = socket_list.head;
		socket_list.head = (*(socket_list.head)).next;

		closesocket((*previous_ptr).socket);
		free(previous_ptr);
	}
}

void print_socket_list(SocketList socket_list) {
	if (!(socket_list.head)) {
		printf("The socket list is empty\n\n");
		return;
	}

	while (socket_list.head) {
		printf("%llu ", (*(socket_list.head)).socket);
		socket_list.head = (*(socket_list.head)).next;
	}

	puts("\n");
}
