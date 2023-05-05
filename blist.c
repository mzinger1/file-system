/* Storing the list of blocks assigned to an inode. */

#include "blist.h"
#include "blocks.h"

// Allocate space in block 1 for another element in our linked-list of block assignments.
int alloc_blist() {
	blist_t* blists = get_block_at(1);
	for (int i = 0; i < 4096 / sizeof(blist_t); i++) {
		if (blists[i].block == 0) {
			blists[i].block = alloc_block();
			return i;
		}
	}
}

// Get the blist at a certain index.
blist_t* get_blist_at(int index) {
	return get_block_at(1) + (sizeof(blist_t) * index);
}
