/* process2.c Simple process which does some calculations and exits. */

#include "common.h"
#include "syslib.h"
#include "util.h"
#include "printf.h"

static int rec(int n);

#define LINE 6
int array[4096];

void __attribute__((section(".entry_function"))) _start(void)
{
    int i,j, res;
	
    for (i = 0; i <= 100; i++) {
        res = rec(i);
		
		array[i] = i;

		for (j=0; j<4096; j++)
			array[j] = 1;
		
        printf(LINE, 0, "PID %d : 1 + ... + %d = %d", getpid(), i, res);
        sleep(300);
        yield();
    }
    exit();
}

/* calculate 1+ ... + n */
static int rec(int n)
{
    if (n % 37 == 0)
        yield();

    if (n == 0)
        return 0;
    else
        return n + rec(n - 1);
}
