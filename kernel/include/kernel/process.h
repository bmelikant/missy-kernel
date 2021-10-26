#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

typedef enum PROCESS_STATE {
    QUEUED,      // queued processes can be run at the next available slot
    RUNNING,     // this process is currently executing
    IDLE,        // this process is not running, but is not blocked and can be run when system ready
    BLOCKED,     // this process is blocked waiting on I/O or resources
    EXITED       // this process has exited
} _process_state_t;

/**
 * create a process from an in-memory function
 */
int create_process(__ptr_t load_address, size_t binary_size, size_t requested_heap_size);

int start_process(int pid);
int  signal_process(int pid, int signal);
void destroy_process(int pid);


#endif