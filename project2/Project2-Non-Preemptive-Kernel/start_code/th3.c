#include "common.h"
#include "scheduler.h"
#include "util.h"

uint32_t thread4_start;
uint32_t thread4_end;
uint32_t thread5_start;
uint32_t thread5_end;
void thread4(void)
{
	int i;
	for ( i = 0; i < 100; ++i){
		thread4_start=get_timer();
		thread4_start-=thread5_end;
		thread4_start = ((uint32_t) thread4_start) / (2*MHZ);     /* divide on CPU clock frequency in
						 * megahertz */

		if (i)
		{
			print_location(0, 1);
			printstr("The Time in switch between thread and process (in us): ");
			printint(60, 1, thread4_start);
		}
		thread4_end=get_timer();
		do_yield();
	}
	do_exit();
}

void thread5(void)
{
	int i;
	for ( i = 0; i < 100; ++i){
		thread5_start=get_timer();
		thread5_start-=thread4_end;
		thread5_start = ((uint32_t) thread5_start) / (MHZ);     /* divide on CPU clock frequency in
						 * megahertz */

		print_location(0, 2);
		printstr("The Time in switch between thread and thread (in us): ");
		printint(60, 2, thread5_start);
		thread5_end=get_timer();
		do_yield();
	}
	do_exit();
}
