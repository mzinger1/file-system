/* Inode manipulation routines. */

#include "bitmap.h"
#include "inode.h" 
#include "blocks.h"

// Print out metadata about the file represented by the given inode.
void print_inode(inode_t *node) {
	printf("ref count %d\n", node->refs);
	printf("mode: %d\n", node->mode);
	printf("size (bytes): %d\n", node->size);
	printf("block pointer initial position: %d\n", node->block_list);
}

// Return the inode at the given index.
inode_t *get_inode(int index) {
	return get_inode_table() + (index * sizeof(inode_t));
}

// Allocate an inode and return its index, or -1 if unable to allocate the inode.
int alloc_inode() {
	void* i_map = get_inode_bitmap();
	for (int i = 0; i < INODE_COUNT; i++) {
		if (bitmap_get(i_map, i) == 0) {
			bitmap_put(i_map, i, 1);
			printf("+ alloc_inode() -> %d\n", i);
			return i;
		}
	}
	printf("Unable to allocate inode");
	return -1;
}

// Free the inode at the given index.
void free_inode(int index) {
	inode_t *inode = get_inode(index);
	void* i_map = get_inode_bitmap();
	// find which blocks we need to free
	blist_t* blocks_to_free = get_blist_at(inode->block_list);
	while (blocks_to_free != 0) {
		free_block(blocks_to_free->block);
		blocks_to_free->block = 0;
		if (blocks_to_free->next == 0) {
			break;
		}
		blocks_to_free = get_blist_at(blocks_to_free->next);
	}
	bitmap_put(i_map, index, 0); // set inode to free
	inode->refs--; // decrement reference counter
}

// Grow the given inode to the given size in bytes.
// returns 0 on success.
int grow_inode(inode_t *node, int size) {
	blist_t *blocks = get_blist_at(node->block_list);
	int i = 1;
	while (i * BLOCK_SIZE < size) {
		if (blocks->next == 0) {
			blocks->next = alloc_blist();
		}
		blocks = get_blist_at(blocks->next);
		i++;
	}
	node->size = size;
	return 0;
}

// Shrink the given inode to the given size.
// returns 0 on success.
int shrink_inode(inode_t *node, int size) {
	blist_t *blocks = get_blist_at(node->block_list);
        int i = 1;
        while (i * (node->size) > size) {
                if (blocks->next != 0) {
			free_block(blocks->next);
                }
                blocks = get_blist_at(blocks->next);
		node->size -= BLOCK_SIZE;
                i++;
        }
        node->size = size;
        return 0;
}
