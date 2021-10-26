#include <kernel/process.h>

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct PROCESS_STRUCT {
    _process_state_t pstate;    // what is this process doing right now?
    uint32_t flags;             // process flags
    __ptr_t load_address;       // where was the program loaded?
    __ptr_t _entry_pt;          // the entry point for the process
    __ptr_t _heap_start;        // the start of the process heap
    __ptr_t _brk;               // program break

    struct PROCESS_STRUCT *next;    // make this into an intrusive list
} _process_t;

_process_t *_base_struct = NULL;

int create_process(__ptr_t entry_pt, size_t binary_sz, size_t requested_heap) {
    // allocate the new process
    _process_t *newproc = (_process_t *) malloc(sizeof(_process_t));
    memset(newproc,0,sizeof(_process_t));

    if (!_base_struct) {
        _base_struct = newproc;
        return 1;
    }

    // add the process to the list and return it's index+1
    int i = 1;
    _process_t *_current_ptr = _base_struct;
    while (_current_ptr->next) {
        _current_ptr = _current_ptr->next;
        i++;       
    }

    _current_ptr->next = newproc;
    return i;
}