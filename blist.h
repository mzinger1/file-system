/* Storing the list of blocks assigned to an inode. */

#ifndef BLIST_H
#define BLIST_H

// Structure of blist:
// Each blist has itself, next assignment.
typedef struct blist {
	int block;
	int next;
} blist_t;

// Allocate space in block 1 for another element in our linked-list of block assignments.
int alloc_blist();

// Get the blist at a certain index.
blist_t* get_blist_at(int index);

#endif
