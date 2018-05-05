/* Just "flies a plane" over the screen. */
#include "common.h"
#include "syslib.h"
#include "util.h"
#include "printf.h"



void __attribute__((section(".entry_function"))) _start(void)
{
	int t;
	t = get_test_lock_p();
	lock_init_p(t);

	printf(1,1,"process 1 is going to get the lock!!!");
    lock_acquire_p(t);
	printf(2,1,"process 1 has gotton the lock and will yield");
	wait(3);
	printf(7,1,"process 1 will not get here");
	exit();
}

