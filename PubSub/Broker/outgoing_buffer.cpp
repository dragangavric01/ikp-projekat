#include "outgoing_buffer.h"

void initialize_outgoing_buffer(OutgoingBuffer* outgoing_buffer_ptr) {
	(*outgoing_buffer_ptr).fill_idx = 0;
	(*outgoing_buffer_ptr).use_idx = 0;
	(*outgoing_buffer_ptr).count = 0;

	(*outgoing_buffer_ptr).crit_section_ptr = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
	(*outgoing_buffer_ptr).empty_cv_ptr = (CONDITION_VARIABLE*)malloc(sizeof(CONDITION_VARIABLE));
	(*outgoing_buffer_ptr).fill_cv_ptr = (CONDITION_VARIABLE*)malloc(sizeof(CONDITION_VARIABLE));

	InitializeCriticalSection((*outgoing_buffer_ptr).crit_section_ptr);
	InitializeConditionVariable((*outgoing_buffer_ptr).empty_cv_ptr);
	InitializeConditionVariable((*outgoing_buffer_ptr).fill_cv_ptr);
}

// Puts a new message in the outgoing buffer
static void put(OutgoingBuffer* outgoing_buffer_ptr, OutgoingBufferElement new_element) {
	(*outgoing_buffer_ptr).array[(*outgoing_buffer_ptr).fill_idx] = new_element;

	(*outgoing_buffer_ptr).fill_idx = ((*outgoing_buffer_ptr).fill_idx + 1) % MAX_OUTGOING_BUFFER_SIZE;
	((*outgoing_buffer_ptr).count)++;
}

// Reads a message from the outgoing buffer
static OutgoingBufferElement get(OutgoingBuffer* outgoing_buffer_ptr) {
	OutgoingBufferElement element = (*outgoing_buffer_ptr).array[(*outgoing_buffer_ptr).use_idx];
	(*outgoing_buffer_ptr).use_idx = ((*outgoing_buffer_ptr).use_idx + 1) % MAX_OUTGOING_BUFFER_SIZE;
	((*outgoing_buffer_ptr).count)--;

	return element;
}

// Returns the value of shutting_down.flag
static bool is_shutting_down() {
	EnterCriticalSection(shutting_down.crit_section_ptr);
	if (shutting_down.flag) {
		LeaveCriticalSection(shutting_down.crit_section_ptr);
		return true;
	} else {
		LeaveCriticalSection(shutting_down.crit_section_ptr);
		return false;
	}
}

// Signals the main thread that one thread has been shut down
static void signal_shut_down() {
	EnterCriticalSection(shutting_down.crit_section_ptr);

	(shutting_down.num_of_shut_down_threads)++;
	WakeConditionVariable(shutting_down.cond_var_ptr);

	LeaveCriticalSection(shutting_down.crit_section_ptr);
}

// Waits until it is signaled by consumer thread that the outgoing buffer is not full and then calls put(). Then it signals the consumer thread that the buffer is not empty
void produce_new_message(OutgoingBuffer* outgoing_buffer_ptr, OutgoingBufferElement new_element) {
	EnterCriticalSection((*outgoing_buffer_ptr).crit_section_ptr);

	while ((*outgoing_buffer_ptr).count == MAX_OUTGOING_BUFFER_SIZE) {
		SleepConditionVariableCS((*outgoing_buffer_ptr).empty_cv_ptr, (*outgoing_buffer_ptr).crit_section_ptr, INFINITE);
	}
	
	put(outgoing_buffer_ptr, new_element);

	WakeConditionVariable((*outgoing_buffer_ptr).fill_cv_ptr);
	LeaveCriticalSection((*outgoing_buffer_ptr).crit_section_ptr);
}

// Function that is executed by topic producer threads. Calls dequeue() and produce_new_message() in a loop
DWORD WINAPI produce(LPVOID ptr_to_topic_and_buffer) {
	TopicAndBuffer* topic_and_buffer_ptr = (TopicAndBuffer*)ptr_to_topic_and_buffer;
	Topic* topic_ptr = (*topic_and_buffer_ptr).topic_ptr;
	OutgoingBuffer* outgoing_buffer_ptr = (*topic_and_buffer_ptr).outgoing_buffer_ptr;

	while (!is_shutting_down()) {
		char* message = dequeue((*topic_ptr).message_queue_ptr);

		OneOrMoreSockets one_or_more_sockets;
		one_or_more_sockets.cs_union.subscriber_connection_sockets_ptr = (*topic_ptr).subscriber_connection_sockets_ptr;
		one_or_more_sockets.more = true;
		OutgoingBufferElement new_element = { message, one_or_more_sockets };

		produce_new_message(outgoing_buffer_ptr, new_element);
	}

	free(topic_and_buffer_ptr);
	signal_shut_down();
	return 0; // "ExitThread(0) is the preferred method of exiting a thread in C code. However, in C++ code, the thread is exited before any destructors can be called or any other automatic cleanup can be performed. Therefore, in C++ code, you should return from your thread function."
}

// If element specifies that the message should be sent to more clients, then it sends the message through all subscription sockets of a certain topic and if it specifies that it is a response, it sends to one client
static void send_subscription_message_or_response(OutgoingBufferElement element) {
	if (element.one_or_more_sockets.more) {
		EnterCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
		printf("Sending message '%s' to subscribers\n\n", element.message + 1);  // +1 because I don't want to show the '#' at the beginning
		LeaveCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));

		SocketList* subscriber_connection_sockets_ptr = element.one_or_more_sockets.cs_union.subscriber_connection_sockets_ptr;

		EnterCriticalSection((*subscriber_connection_sockets_ptr).crit_section_ptr);

		SocketListNode* walker = (*subscriber_connection_sockets_ptr).head;
		while (walker) {
			SOCKET connection_socket = *((*walker).socket_ptr);  // Accessing a SocketList node must be done before leaving the critical section

			// I'm calling LeaveCriticalSection() before sending because sending may block this thread and that would mean other threads will be blocked too because this one is holding the critical section object
			LeaveCriticalSection((*subscriber_connection_sockets_ptr).crit_section_ptr);
			send_to_client(connection_socket, element.message);
			EnterCriticalSection((*subscriber_connection_sockets_ptr).crit_section_ptr);

			walker = (*walker).next;
		}

		LeaveCriticalSection((*subscriber_connection_sockets_ptr).crit_section_ptr);
	} else {
		send_to_client(element.one_or_more_sockets.cs_union.connection_socket, element.message);
	}
}

// Function that's executed by the consumer thread. In does this in a loop: It waits until main thread or a topic producer thread signals it that the outgoing buffer is not empty and then it calls get(). Then it signals one of the producer threads (including main thread) that the buffer is not full and it calls send_subscription_message_or_response()
DWORD WINAPI consume(LPVOID ptr_to_outgoing_buffer) {
	OutgoingBuffer* outgoing_buffer_ptr = (OutgoingBuffer*)ptr_to_outgoing_buffer;

	while (!is_shutting_down()) {
		EnterCriticalSection((*outgoing_buffer_ptr).crit_section_ptr);
		
		while (!((*outgoing_buffer_ptr).count)) {
			SleepConditionVariableCS((*outgoing_buffer_ptr).fill_cv_ptr, (*outgoing_buffer_ptr).crit_section_ptr, INFINITE);
		}

		OutgoingBufferElement element = get(outgoing_buffer_ptr);

		WakeConditionVariable((*outgoing_buffer_ptr).empty_cv_ptr);
		LeaveCriticalSection((*outgoing_buffer_ptr).crit_section_ptr);

		send_subscription_message_or_response(element);
		free(element.message);
	}

	signal_shut_down();
	return 0; // "ExitThread(0) is the preferred method of exiting a thread in C code. However, in C++ code, the thread is exited before any destructors can be called or any other automatic cleanup can be performed. Therefore, in C++ code, you should return from your thread function."
}

