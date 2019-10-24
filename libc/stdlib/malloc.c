/**
 * stdlib/malloc.c: malloc implementation for libc
 * This code is taken almost entirely as-is from The C Programming Language Second Edition
 * At some point I may re-implement this as a tree-based allocator, but for now this is sufficient
 * to get me moving
 * 
 * This file also contains the functions free() and calloc() since these also rely on the state of
 * the _base and _next_free variables
 */

#include <sys/unistd.h>

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define MALLOC_MIN_SBRK_REQUEST	1024		// request at minimum 1024 bytes from sbrk

// allocated memory block type header
typedef union MEMORY_HEADER {
	struct {
		union MEMORY_HEADER *next;
		size_t size;
	} s;
	unsigned long align;
} _mheader_t;

typedef _mheader_t *	_mheader_p;

// internal function declarations
static _mheader_p morecore(size_t size);

/** internal allocator variables */
static _mheader_t _base;
static _mheader_p _next_free = NULL;

/**
 * malloc() : allocate n bytes of memory from the kernel heap and return the allocated address
 */
void *malloc(size_t nbytes) {
	_mheader_p header, prev_header;
	size_t allocation_size = nbytes + sizeof(_mheader_t);

	// start a new allocation list. basically just here to force morecore on the first loop through
	if ((prev_header = _next_free) == NULL) {
		_base.s.next = _next_free = prev_header = &_base;
		_base.s.size = 0;
	}

	// find a header big enough to fit the request
	for (header = prev_header->s.next; /** no condition */ ; prev_header = header, header = header->s.next) {
		// current header is big enough
		if (header->s.size >= allocation_size) {
			// best case scenario: exact match
			if (header->s.size == allocation_size) {
				prev_header->s.next = header->s.next;
			} else {
				header->s.size -= allocation_size;
				header = (_mheader_p) ((char *)(header)+header->s.size);
				header->s.size = allocation_size;
			}

			_next_free = prev_header;
			return (void *)(header+1);
		}

		// list wrapped to beginning; call morecore
		if (header == _next_free) {
			if ((header = morecore(allocation_size)) == NULL) {
				// couldn't find any more free memory
				return NULL;
			}
		}
	}

	// this is just a failsafe in case the function makes it this far. It should not get here
	return NULL;
}

/**
 * free() : release an allocated block of memory back into the heap for reuse
 */
void free(void *block) {
	_mheader_p _mblock, _header;
	_mblock = (_mheader_p)(block)-1;

	for (_header = _next_free; !(_mblock > _header && _mblock < _header->s.next); _header = _header->s.next) {
		if (_header >= _header->s.next && (_mblock > _header || _mblock < _header->s.next)) {
			break;
		}
	}

	// coalesce adjacent blocks up
	if (_mblock + _mblock->s.size == _header->s.next) {
		_mblock->s.size += _header->s.next->s.size;
		_mblock->s.next = _header->s.next->s.next;
	} else {
		_mblock->s.next = _header->s.next;
	}

	// coalesce adjacent blocks down
	if (_header + _header->s.size == _mblock) {
		_header->s.size += _mblock->s.size;
		_header->s.next = _mblock->s.next;
	} else {
		_header->s.next = _mblock;
	}

	_next_free = _header;
}

/**
 * calloc() : allocate count blocks of size size
 */
void *calloc(size_t count, size_t size) {
	void *_memblock = malloc(count*size);
	if ((_memblock = malloc(count*size)) != NULL) {
		memset(_memblock,0,count*size);
	}
	return _memblock;
}

/** 
 * morecore() : get more memory from the system allocator
 */
static _mheader_p morecore(size_t size) {
	size += sizeof(_mheader_t);
	if (size < MALLOC_MIN_SBRK_REQUEST) {
		size = MALLOC_MIN_SBRK_REQUEST;
	}

	_mheader_p _next = (_mheader_p)(sbrk(size));
	if (_next == (_mheader_p) -1) {
		errno = ENOMEM;
		return NULL;
	}

	// adjust down for the size of the header
	_next->s.size = size;
	free((void*)(_next)+sizeof(_mheader_t));
	return _next;
}