/* A block-based abstraction over a disk image file.
 * The disk image is mmapped, so block data is accessed using pointers. */

#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdio.h>

extern const int BLOCK_COUNT; // we split the "disk" into 256 blocks
extern const int BLOCK_SIZE; // each block has 4K bytes
extern const int NUFS_SIZE; // the total disk size is 1MB
extern const int BLOCK_BITMAP_SIZE; // 256 bits = 32 bytes for tracking the availability of the blocks
extern const int INODE_BITMAP_SIZE;
extern const int INODE_COUNT; // We end up with space for 31 bytes of inode bitmap, and the corresponding 248 inodes.


// Compute the number of blocks needed to store the given number of bytes.
int bytes_to_blocks(int bytes);

// Load and initialize the given disk image.
void blocks_init(const char *image_path);

// Close the disk image.
void blocks_free();

// Allocate a new block and return its index.
int alloc_block();

// Deallocate the block at the given index.
void free_block(int index);

// Get the block at the given index, returning a pointer to its start.
void *get_block_at(int index);

// Return a pointer to the beginning of the block bitmap.
void *get_blocks_bitmap();

// Return a pointer to the beginning of the inode bitmap.
void *get_inode_bitmap();

// Return a pointer to the beginning of the inode table.
void *get_inode_table();

#endif
