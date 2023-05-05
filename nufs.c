/* The main file. */

#include <assert.h>
#include <bsd/string.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>
#include "directory.h"
#include "inode.h"
#include "blocks.h"
#include "slist.h"
#include "bitmap.h" 


// Checks if a file exists.
// Returns -ENOENT on fail, 0 otherwise.
int nufs_access(const char *path, int mask) {
	int rv = 0;
	int exists = find_inode_index(path);
	if (exists < 0) {
		return -ENOENT;
	}
	printf("access(%s, %04o) -> %d\n", path, mask, rv);
	return rv;
}

// Gets an object's attributes (type, permissions, size, etc).
// Returns -ENOENT if object doesn't exist, 0 otherwise.
int nufs_getattr(const char *path, struct stat *st) {
	int rv = 0;
	int inode_index = find_inode_index(path);
	if (inode_index < 0) {
		return -ENOENT;
	}
	inode_t* node = get_inode(inode_index);
	// clean stats struct
	memset(st, 0, sizeof(struct stat));
	st->st_mode = node->mode;
	st->st_size = node->size;
	st->st_uid = getuid();
	printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode,
			st->st_size);
	return rv;
}

// Lists the contents of a directory, returning w/ 0 on successful listing.
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
					off_t offset, struct fuse_file_info *fi) {
	struct stat st;
	int rv = 0;
	// call directory to get list of contents
	slist_t* list = directory_list(path);
	rv = nufs_getattr(path, &st);
	// continue placing entries into the buffer 
	while (list) {
		if (strcmp(list->data, "") != 0) {
			filler(buf, list->data, &st, 0);
		}
		list = list->next;
	}
	printf("readdir(%s) -> %d\n", path, rv);
	return rv;
}

// Makes a filesystem object such as a file or directory.
// returns -1 on fail, 0 otherwise.
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
	int rv = -1; 

	// allocate the new inode
	int inum_new = alloc_inode();
	// return -1 if we couldn't properly allocate
	if (inum_new < 0) {
		return rv;
	}

	inode_t* inode_new = get_inode(inum_new);
	// populate the metadata
	inode_new->refs = 1;
	inode_new->mode = mode;
	inode_new->size = 0;
	inode_new->block_list = alloc_blist();
	assert(inode_new->block_list > 0); 

	const char *name = get_filename(path);
	// place the new file under parent
	int dir_num = parent_inode_index(path);
	inode_t *directory = get_inode(dir_num);
	directory_put(directory, name, inum_new);
	rv = 0;
	printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
	return rv;
}

// Makes a directory.
// returns -1 on fail, 0 otherwise.
int nufs_mkdir(const char *path, mode_t mode) {
	int rv = nufs_mknod(path, mode | 040000, 0);
	printf("mkdir(%s) -> %d\n", path, rv);
	return rv;
}

// Deletes files.
// returns -1 on fail, 0 otherwise.
int nufs_unlink(const char *path) {
	int rv = -1;
	int predecessor = parent_inode_index(path);
	assert(predecessor > -1);
	inode_t* directory = get_inode(predecessor);
	// once we found the directory, remove file from there
	directory_delete(directory, path);
	rv = 0;
	printf("unlink(%s) -> %d\n", path, rv);
	return rv;
}

// Puts files in directories.
// returns -1 on fail, 0 otherwise.
int nufs_link(const char *from, const char *to) {
	int rv = -1;
	int inode_num = find_inode_index(from);
	int directory_num = parent_inode_index(to);
	// ensure these exist
	assert(inode_num > -1);
	assert(directory_num > -1);

	inode_t *directory = get_inode(directory_num);
	directory_put(directory, get_filename(to), inode_num);
	rv = 0;
	printf("link(%s => %s) -> %d\n", from, to, rv);
	return rv;
}

// Removes empty directories.
// returns -1 on fail, 0 otherwise. 
int nufs_rmdir(const char *path) {
	int rv = -1;
	int inode_num = find_inode_index(path);
	inode_t *node = get_inode(inode_num);
	if (node->mode != 040775) {
		return rv;
	}
	// If the directory is empty, then we can unlink it
	if (directory_list(path) == NULL) {
		rv = nufs_unlink(path);
	}
	printf("rmdir(%s) -> %d\n", path, rv);
	return rv;
}

// Moves a file within the same filesystem.
// returns -1 on fail, 0 otherwise.
int nufs_rename(const char *from, const char *to) {
	int rv = -1; 
	// first, make sure the file eists
	int file_idx = find_inode_index(from);
	assert(file_idx > -1);
	rv = nufs_link(from, to);
	assert(rv > -1);

	rv = nufs_unlink(from);
	printf("rename(%s => %s) -> %d\n", from, to, rv);
	return rv;
}

// Changes permissions on a file.
int nufs_chmod(const char *path, mode_t mode) {
	int rv = 0;
	printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
	return rv;
}

// Limits files to a certain size.
// returns -1 on fail, 0 otherwise.
int nufs_truncate(const char *path, off_t size) {
	int rv = -1;
	int inode_num = find_inode_index(path);
	if (inode_num < 0) {
		return rv;
	} 
	// determine whether we need to grow or shrink 
	inode_t *node = get_inode(inode_num);
	if (size >= node->size) {
		rv = grow_inode(node, size);
	} else {
		rv = shrink_inode(node, size);
	}
	printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
	return rv;
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi) {
	int rv = 0;
	printf("open(%s) -> %d\n", path, rv);
	return rv;
}

// Reads data from a file.
// returns -1 on fail, or size on success.
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
				struct fuse_file_info *fi) {
	int rv = -1;
	int num = find_inode_index(path);
	if (num < 0) {
		return rv;
	}
	printf("reading from inode:%d\n", num);
	inode_t* node = get_inode(num);
	// returns 0 if offset at or beyond file
	if (offset >= node->size) {
		return 0;
	}
	
	blist_t* blocks = get_blist_at(node->block_list);
	int i = 0;
	while (blocks != 0) {
		void* block = get_block_at(blocks->block);
		// copy from block into the buffer
		memcpy(((void*) buf) + (i * BLOCK_SIZE), block, BLOCK_SIZE);
		if (blocks->next == 0) {
			break;
		}
		blocks = get_blist_at(blocks->next);
		i++;
	}

	rv = size;
	printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
	return rv;
}

// Writes data to a file.
// returns -1 on fail, or size on success.
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
				struct fuse_file_info *fi) {
	int rv = -1;
	int num = find_inode_index(path);
	if (num < 0) {
		return rv;
	}
	inode_t* node = get_inode(num);
	printf("write to inode: %d\n", num);
	node->size = size;
	// handle multiple block cases with offset
	grow_inode(node, offset + size);

	blist_t* blocks = get_blist_at(node->block_list);
	int offset_blocks = offset / BLOCK_SIZE;
	printf("offset: %ld, offset_blocks: %d\n", offset, offset_blocks);
	int i = 0;
	while (blocks != 0) {
		void* block = get_block_at(blocks->block);
		// copy from buffer into the block 
		if (i == offset_blocks) {
			memcpy(block, buf, BLOCK_SIZE);
		}
		if (blocks->next == 0) {
			break;
		}
		blocks = get_blist_at(blocks->next);
		i++;
	}
	rv = size;
	printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
	return rv;
}

// Updates the timestamps on a file or directory.
int nufs_utimens(const char *path, const struct timespec ts[2]) {
	int rv = 0;
	printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n", path, ts[0].tv_sec,
			ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
	return rv;
}

// Extended operations.
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
				unsigned int flags, void *data) {
	int rv = 0;
	printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
	return rv;
}

// Initializing operations.
void nufs_init_ops(struct fuse_operations *ops) {
	memset(ops, 0, sizeof(struct fuse_operations));
	ops->access = nufs_access;
	ops->getattr = nufs_getattr;
	ops->readdir = nufs_readdir;
	ops->mknod = nufs_mknod;
	ops->mkdir = nufs_mkdir;
	ops->link = nufs_link;
	ops->unlink = nufs_unlink;
	ops->rmdir = nufs_rmdir;
	ops->rename = nufs_rename;
	ops->chmod = nufs_chmod;
	ops->truncate = nufs_truncate;
	ops->open = nufs_open;
	ops->read = nufs_read;
	ops->write = nufs_write;
	ops->utimens = nufs_utimens;
	ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int main(int argc, char *argv[]) {
	assert(argc > 2 && argc < 6);
	// load and initialize the disk image passed
	blocks_init(argv[--argc]); 
	directory_init();
	nufs_init_ops(&nufs_ops);
	return fuse_main(argc, argv, &nufs_ops, NULL);
}
