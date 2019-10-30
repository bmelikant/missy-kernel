#include <kernel/chrdev.h>

#include <stdlib.h>
#include <errno.h>

#define CHRDEV_INODE_BASE	0x01

typedef struct CHARACTER_DEVICE_INODE _chrdev_inode_t;
struct CHARACTER_DEVICE_INODE {
	int inode;
	_chrdev_t *device_descriptor;
	_chrdev_inode_t *next;
};

// the base inode
static _chrdev_inode_t base_inode;
static _chrdev_inode_t *current_inode = NULL;
static int current_inode_number = CHRDEV_INODE_BASE;
static int minor_revision_number = 0;

/** internal function declarations */
static _chrdev_inode_t *new_inode();
static _chrdev_inode_t *find_device_inode(int descriptor);

int register_device(_chrdev_t *device_descriptor) {
	_chrdev_inode_t *current = new_inode();
	printf("got inode address 0x%x\n", (uint32_t) current);
	if (!current) {
		errno = ENOMEM;
		return -1;
	}

	current->device_descriptor = device_descriptor;
	current->inode = current_inode_number++;
	device_descriptor->_descriptor = current->inode;
	device_descriptor->minor_revision = minor_revision_number++;

	return current->inode;
}

int read_device_char(int descriptor) {
	_chrdev_inode_t *inode = find_device_inode(descriptor);
	if (inode && inode->device_descriptor->readchar) {
		return inode->device_descriptor->readchar();
		return 0;
	}
}

int write_device(int descriptor, const char *buffer, size_t length) {
	// find the inode number to write
	_chrdev_inode_t *inode = find_device_inode(descriptor);
	if (inode && inode->device_descriptor->write != NULL) {
		return inode->device_descriptor->write(buffer, length);
	}

	return -1;
}

static _chrdev_inode_t *find_device_inode(int descriptor) {
	for (_chrdev_inode_t *current = &base_inode; current != NULL; current = current->next) {
		if (current->inode == descriptor) {
			return current;
		}
		
	}
	return NULL;
}

static _chrdev_inode_t *new_inode() {
	if (!current_inode) {
		current_inode = &base_inode;
		current_inode->next = NULL;
		return current_inode;
	} else {
		// try to allocate a new inode
		current_inode->next = (_chrdev_inode_t *) malloc(sizeof(_chrdev_inode_t));
		if (!current_inode->next) {
			return NULL;
		}
		
		current_inode = current_inode->next;
		current_inode->next = NULL;
		return current_inode;
	}
}