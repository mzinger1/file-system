/* Inode manipulation routines. */

#ifndef INODE_H
#define INODE_H

#include "blocks.h"
#include "blist.h"

typedef struct inode {
	int refs;  // reference count
	int mode;  // permission & type
	int size;  // bytes
	int block_list; // blocks connected
} inode_t;

// Prints out metadata about the file represented by a given inode.
void print_inode(inode_t *node);

// Return the inode at the given index.
inode_t *get_inode(int index);

// Allocate an inode and return its index, or -1 if unable to allocate the inode.
int alloc_inode();

// Free the inode at the given index.
void free_inode(int index);

// Grow the given inode to the given size in bytes.
int grow_inode(inode_t *node, int size);

// Shrink the given inode to the given size.
int shrink_inode(inode_t *node, int size);

#endif
