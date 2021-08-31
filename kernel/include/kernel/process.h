#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#include <stdint.h>
#include <stddef.h>

typedef enum PROCESS_STATE {
    QUEUED,      // queued processes can be run at the next available slot
    RUNNING,     // this process is currently executing
    IDLE,        // this process is not running, but is not blocked and can be run when system ready
    BLOCKED,     // this process is blocked waiting on I/O or resources
    EXITED       // this process has exited
} _process_state_t;

typedef struct PROCESS_STRUCT _process_t;

_process_t *create_process(__ptr_t load_addr, __ptr_t entry, __ptr_t heap, __ptr_t brk);
int  signal_process(_process_t *process);
void destroy_process(_process_t *process);

_process_t *get_process_by_id(unsigned int pid);

#endif