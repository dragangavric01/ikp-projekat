#pragma once

#include <windows.h>


typedef struct ShuttingDownFlagStruct {
	bool flag;
	int num_of_shut_down_threads;
	CRITICAL_SECTION* crit_section_ptr;
	CONDITION_VARIABLE* cond_var_ptr;
} ShuttingDownFlag;


extern volatile ShuttingDownFlag shutting_down;

extern volatile CRITICAL_SECTION printf_crit_section;


void initialize_shutting_down_flag();
bool is_shutting_down();
void signal_shut_down();

void initialize_printf_crit_section();