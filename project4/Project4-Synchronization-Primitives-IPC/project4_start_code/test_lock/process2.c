/* process2.c Simple process which does some calculations and exits. */

#include "common.h"
#include "syslib.h"
#include "util.h"
#include "printf.h"



void __attribute__((section(".entry_function"))) _start(void)
{
    int t;
	sleep(5000);
	t = get_test_lock_p();
	printf(3,1,"process 2 will kill process 1");
	kill(2);
	printf(4,1,"process 2 has killed process 1");
	printf(5,1,"process 2 will try to get the lock");
	lock_acquire_p(t);
	printf(6,1,"process 2 has gotton the lock");
	lock_release_p(t);
	printf(7,1,"process 2 has released the lock");
	exit();
}
