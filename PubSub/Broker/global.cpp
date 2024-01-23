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


volatile CRITICAL_SECTION printf_crit_section;

void initialize_printf_crit_section() {
	InitializeCriticalSection((CRITICAL_SECTION*)(&printf_crit_section));
}