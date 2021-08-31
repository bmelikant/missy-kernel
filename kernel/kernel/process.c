#include <kernel/process.h>

#include <stdint.h>
#include <stddef.h>

struct PROCESS_STRUCT {
    unsigned int pid;           // id of running process
    _process_state_t pstate;    // what is this process doing right now?
    uint32_t flags;             // process flags
    __ptr_t load_address;       // where was the program loaded?
    __ptr_t _entry_pt;          // the entry point for the process
    __ptr_t _heap_start;        // the start of the process heap
    __ptr_t _brk;               // program break
};
