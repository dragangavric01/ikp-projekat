#include "global.h"

volatile ShuttingDownFlag shutting_down;

void initialize_shutting_down_flag() {
	shutting_down.flag = false;
	shutting_down.num_of_shut_down_threads = 0;

	shutting_down.crit_section_ptr = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
	shutting_down.cond_var_ptr = (CONDITION_VARIABLE*)malloc(sizeof(CONDITION_VARIABLE));

	InitializeCriticalSection(shutting_down.crit_section_ptr);
	InitializeConditionVariable(shutting_down.cond_var_ptr);
}

// Returns the value of shutting_down.flag
bool is_shutting_down() {
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
void signal_shut_down() {
	EnterCriticalSection(shutting_down.crit_section_ptr);

	(shutting_down.num_of_shut_down_threads)++;
	WakeConditionVariable(shutting_down.cond_var_ptr);

	LeaveCriticalSection(shutting_down.crit_section_ptr);
}


volatile CRITICAL_SECTION printf_crit_section;

void initialize_printf_crit_section() {
	InitializeCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
}