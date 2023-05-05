/* An implementation of a block-based abstraction over a disk image file. */

#define _GNU_SOURCE
#include <string.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "bitmap.h"
#include "blocks.h"
#include "inode.h"

const int BLOCK_COUNT = 256; // we split the "disk" into 256 blocks
const int BLOCK_SIZE = 4096; // each block has 4K bytes
const int NUFS_SIZE = BLOCK_SIZE * BLOCK_COUNT; // the total disk size is 1MB
const int BLOCK_BITMAP_SIZE = BLOCK_COUNT / 8; // 256 bits = 32 bytes (Note: we assume BLOCK_COUNT is divisible by 8)
const int INODE_BITMAP_SIZE = (BLOCK_SIZE - BLOCK_BITMAP_SIZE) / ((sizeof(inode_t) * 8) + 1);
const int INODE_COUNT = 8 * INODE_BITMAP_SIZE;
// We have 4096-32=4064 bytes for storing the inode bitmap and table. Each 8 inodes requires 1 byte of bitmap.
// We end up with space for 31 bytes of inode bitmap, and the corresponding 248 inodes.

static int blocks_fd = -1;
static void *blocks_base = 0;

// Get the number of blocks needed to store the given number of bytes.
int bytes_to_blocks(int bytes) {
	int quo = bytes / BLOCK_SIZE;
	int rem = bytes % BLOCK_SIZE;
	if (rem == 0) {
		return quo;
	} else {
		return quo + 1;
	}
}

// Load and initialize the given disk image.
void blocks_init(const char *image_path) {
	blocks_fd = open(image_path, O_CREAT | O_RDWR, 0644);
	assert(blocks_fd != -1);

	// make sure the disk image is exactly 1MB
	int rv = ftruncate(blocks_fd, NUFS_SIZE);
	assert(rv == 0);

	// map the image to memory
	blocks_base =
		mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, blocks_fd, 0);
	assert(blocks_base != MAP_FAILED);

	// block 0 stores the block bitmap, the inode bitmap, and the inode table
	void *bbm = get_blocks_bitmap();
	bitmap_put(bbm, 0, 1);
	// block 1 stores the linked-lists of which blocks are being used by each inode
	bitmap_put(bbm, 1, 1);
}

// Close the disk image.
void blocks_free() {
	int rv = munmap(blocks_base, NUFS_SIZE);
	assert(rv == 0);
	close(blocks_fd);
}

// Allocate a new block and return its index.
int alloc_block() {
	for (int i = 2; i < BLOCK_COUNT; i++) {
		if (!bitmap_get(blocks_base, i)) {
			bitmap_put(blocks_base, i, 1);
			printf("+ alloc_block() -> %d\n", i);
			return i;
		}
	}

	return -1;
}

// Deallocate the block at the given index.
void free_block(int index) {
	printf("+ free_block(%d)\n", index);
	void *bbm = get_blocks_bitmap();
	bitmap_put(bbm, index, 0);
}

// Get the block at the given index, returning a pointer to its start.
void *get_block_at(int index) {
	return blocks_base + BLOCK_SIZE * index;
}

// The following functions return pointers to various parts of block 0, which consists of:
// - 32 bytes (256 bits) of the block bitmap (representing which blocks are available)
// - 31 bytes (248 bits) of the inode bitmap (representing which locations in the inode table are available to store inodes)
// - 3968 bytes of the inode table (storing the actual inodes)

// Return a pointer to the beginning of the block bitmap.
void *get_blocks_bitmap() {
	return blocks_base;
}

// Return a pointer to the beginning of the inode bitmap.
void *get_inode_bitmap() {
	return blocks_base + BLOCK_BITMAP_SIZE;
}

// Return a pointer to the beginning of the inode table.
void *get_inode_table() {
	return blocks_base + BLOCK_BITMAP_SIZE + INODE_BITMAP_SIZE;
}
