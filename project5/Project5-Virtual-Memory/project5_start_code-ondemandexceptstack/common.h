/* common.h */
/* Do not change this file */

#ifndef COMMON_H
#define COMMON_H

#define NULL ((void *) 0)

/* Extra Credit */
#define EC_PRIORITIES
#define EC_DEADLOCK

/* System calls
 * This enumeration gives every system call
 * a unique syscall number.
 * They are exposed to processes in syslib.h, eg yield()
 * They are exposed to kernel/threads in scheduler.h, eg do_yield()
 */
typedef enum {
    SYSCALL_YIELD=0,
    SYSCALL_EXIT,
    SYSCALL_GETPID,
    SYSCALL_GETPRIORITY,
    SYSCALL_SETPRIORITY,
    SYSCALL_SLEEP,
    SYSCALL_SHUTDOWN,
    SYSCALL_WRITE_SERIAL,
    SYSCALL_PRINT_CHAR,
    NUM_SYSCALLS
} syscall_t;

/* Assertion macros */
#define ASSERT2(p, s) \
do { \
    if (!(p)) { \
         print_str(1, 1, "Assertion failure: ");  \
         print_str(1, 20, s);                     \
         print_str(2, 1, "file: ");               \
         print_str(2, 7, __FILE__);               \
         print_str(3, 1, "line: ");               \
         print_int(3, 7, __LINE__);               \
        while (1); \
    } \
} while (0)
#define ASSERT(p) ASSERT2(p, #p)
#define HALT(s) ASSERT2(FALSE, s)

/* Commonly used types */
typedef enum {
    FALSE, TRUE
} bool_t;

typedef signed char int8_t;
typedef short int int16_t;
typedef long int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

typedef int32_t pid_t;

#endif                          /* COMMON_H */
