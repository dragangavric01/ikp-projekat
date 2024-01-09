#include <stdio.h>
#include <stdlib.h>
#include "subscriber_list.h"


void init_list(SubscriberList* ptr_to_list) {
	(*ptr_to_list).head = NULL;
	(*ptr_to_list).size = 0;
}

void add_to_start(SubscriberList* ptr_to_list, SOCKET* connection_socket_ptr) {
	SubscriberListNode** ptr_to_head = &((*ptr_to_list).head);

	if (!(*ptr_to_head)) {  // list is empty
		*ptr_to_head = (SubscriberListNode*)malloc(sizeof(SubscriberListNode));

		(**ptr_to_head).connection_socket_ptr = connection_socket_ptr;
		(**ptr_to_head).next = NULL;
	} else {
		SubscriberListNode* old_head = *ptr_to_head;
		
		*ptr_to_head = (SubscriberListNode*)malloc(sizeof(SubscriberListNode));

		(**ptr_to_head).connection_socket_ptr = connection_socket_ptr;
		(**ptr_to_head).next = old_head;
	}

	(*ptr_to_list).size++;
}

