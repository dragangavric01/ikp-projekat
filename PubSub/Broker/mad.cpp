#include "mad.h"

// Sets shutting_down.flag to true and waits until all other threads have been shut down
static void shut_down_threads(Topic topics[], int number_of_topics, OutgoingBuffer* outgoing_buffer_ptr) {
    EnterCriticalSection(shutting_down.crit_section_ptr);
    shutting_down.flag = true;
    LeaveCriticalSection(shutting_down.crit_section_ptr);

    // Wake them up if they are sleeping
    for (int i = 0; i < number_of_topics; i++) {
        EnterCriticalSection((*(topics[i].message_queue_ptr)).crit_section_ptr);
        WakeConditionVariable((*(topics[i].message_queue_ptr)).cond_var_ptr);
        LeaveCriticalSection((*(topics[i].message_queue_ptr)).crit_section_ptr);
    }
    EnterCriticalSection((*outgoing_buffer_ptr).crit_section_ptr);
    WakeAllConditionVariable((*outgoing_buffer_ptr).empty_cv_ptr);
    WakeAllConditionVariable((*outgoing_buffer_ptr).fill_cv_ptr);
    LeaveCriticalSection((*outgoing_buffer_ptr).crit_section_ptr);

    EnterCriticalSection(shutting_down.crit_section_ptr);
    // It waits until all threads are shut down because if it goes on and frees the queues and lists before that, the program will crash
    while (shutting_down.num_of_shut_down_threads < (number_of_topics + 1)) {
        SleepConditionVariableCS(shutting_down.cond_var_ptr, shutting_down.crit_section_ptr, INFINITE);
    }
    LeaveCriticalSection(shutting_down.crit_section_ptr);
}

// Frees all resources in a safe way
// "Mutual assured destruction (MAD) is a doctrine of military strategy and national security policy which posits that a full-scale use of nuclear weapons by an attacker on a nuclear-armed defender with second-strike capabilities would cause the complete annihilation of both the attacker and the defender."
void mutual_assured_destruction(SOCKET welcoming_socket, Topic topics[], int number_of_topics, SocketList* connection_sockets_ptr, HANDLE* consumer_thread_ptr, OutgoingBuffer* outgoing_buffer_ptr) {
    shut_down_threads(topics, number_of_topics, outgoing_buffer_ptr);
    DeleteCriticalSection(shutting_down.crit_section_ptr);
    free(shutting_down.crit_section_ptr);
    free(shutting_down.cond_var_ptr);

    CloseHandle(*consumer_thread_ptr);

    DeleteCriticalSection((*outgoing_buffer_ptr).crit_section_ptr);
    free((*outgoing_buffer_ptr).crit_section_ptr);
    free((*outgoing_buffer_ptr).empty_cv_ptr);
    free((*outgoing_buffer_ptr).fill_cv_ptr);
    
    DeleteCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));

    close_sockets_and_free_socket_list(connection_sockets_ptr);

    for (int i = 0; i < number_of_topics; i++) {
        CloseHandle(topics[i].producer_thread);

        free_message_queue(topics[i].message_queue_ptr);
        DeleteCriticalSection((*(topics[i].message_queue_ptr)).crit_section_ptr);
        free((*(topics[i].message_queue_ptr)).crit_section_ptr);
        free((*(topics[i].message_queue_ptr)).cond_var_ptr);
        free(topics[i].message_queue_ptr);

        close_sockets_and_free_socket_list(topics[i].subscriber_connection_sockets_ptr);
        DeleteCriticalSection((*(topics[i].subscriber_connection_sockets_ptr)).crit_section_ptr);
        free(topics[i].subscriber_connection_sockets_ptr);
    }

    closesocket(welcoming_socket);

    WSACleanup();

    exit(1);
}