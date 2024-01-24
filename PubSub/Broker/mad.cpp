#include "mad.h"


static void shut_down_threads(int number_of_topics) {
    EnterCriticalSection(shutting_down.crit_section_ptr);

    shutting_down.flag = true;

    // It waits until all threads are shut down because if it goes on and frees the queues and lists before that, the program will crash
    while (shutting_down.num_of_shut_down_threads < (number_of_topics + 1)) {
        SleepConditionVariableCS(shutting_down.cond_var_ptr, shutting_down.crit_section_ptr, INFINITE);
    }

    LeaveCriticalSection(shutting_down.crit_section_ptr);
}

// "Mutual assured destruction (MAD) is a doctrine of military strategy and national security policy which posits that a full-scale use of nuclear weapons by an attacker on a nuclear-armed defender with second-strike capabilities would cause the complete annihilation of both the attacker and the defender."
void mutual_assured_destruction(SOCKET welcoming_socket, Topic topics[], int number_of_topics, SocketList* connection_sockets_ptr, HANDLE* consumer_thread_ptr, OutgoingBuffer* outgoing_buffer_ptr) {
    shut_down_threads(number_of_topics);
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