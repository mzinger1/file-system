/* Directory manipulation functions. */

#include "directory.h"
#include "bitmap.h"
#include "inode.h"

#include <string.h>
#include <assert.h>

// Initialize root directory.
void directory_init() {
	// allocate a root inode
	int root = alloc_inode();
	printf("root: %d\n", root);
	inode_t *node = get_inode(root);
	node->refs = 1;
	node->mode = 040775; // directory mode
	node->size = 0;
	node->block_list = alloc_blist();
	print_inode(node);
}

// Return the relative path name from the given absolute path.
const char* get_filename(const char *path) {
	slist_t* split_path = s_split(path, '/');
	while (split_path->next) {
		split_path = split_path->next;
	}
	return split_path->data;
}

// Finds and returns inode index of a file, 
// given the directory that contains the file and the name of the file.
int find_file_in_dir(inode_t *dir, const char *name) {
	// root dir corresponds to inode 0
	if (strcmp(name, "/") == 0) {
		return 0;
	}

	int entries_in_block = BLOCK_SIZE / sizeof(direntry_t);
	// retrieve contents of directory by obtaining starting block of block list
	direntry_t* contents = get_block_at(get_blist_at(dir->block_list)->block);
	for (int i = 0; i < entries_in_block; i++) {
		if (strcmp(contents[i].name, name) == 0) {
			return contents[i].inum;
		}
	}

	return -1;
}

// Finds and returns the inode index of the parent of the given path.
int parent_inode_index(const char *path) {
	slist_t* list = s_split(path, '/');
	assert(list != 0);
	int parent_index = 0; // root dir corresponds to inode 0
	while (list->next) {
		parent_index = find_file_in_dir(get_inode(parent_index), list->data);
		list = list->next;
	}
	return parent_index;
}

// Finds and returns the inode index of the given path.
int find_inode_index(const char *path) {
	slist_t* list = s_split(path, '/');
	assert(list != 0);
	int inode_index = 0; // root dir corresponds to inode 0
	while (list) {
		inode_index = find_file_in_dir(get_inode(inode_index), list->data);
		list = list->next;
	}
	return inode_index;
}

// Put a file with the given name and inode index underneath directory dir.
// and return index, or -1 if not succesfully put.
int directory_put(inode_t *dir, const char *name, int index) {
	direntry_t* contents = (direntry_t*) get_block_at(get_blist_at(dir->block_list)->block);
	int num_entries = BLOCK_SIZE / sizeof(direntry_t);
	inode_t* node = get_inode(index);
	// find the first free entry within the directory block, and place the file there
	for (int i = 0; i < num_entries; i++) {
		if(contents[i].inum == 0) {
			// if we found a spot for it, modify necessary params.
			node->refs += 1;
			contents[i].inum = index;
			strcpy(contents[i].name, name);
			dir->size+=sizeof(direntry_t);
			return index;
		}
	}
	printf("Unable to place file under directory.");
	return -1;
}

// Remove the given path from the given directory.
// returns free'd inode index corresponding to file, or -1 on fail.
int directory_delete(inode_t *dir, const char *name) {
	// retrieve the directory contents by getting the block at specified index
	direntry_t* contents = (direntry_t*) get_block_at(get_blist_at(dir->block_list)->block);
    	int num_entries = BLOCK_SIZE / sizeof(direntry_t);
	int inode_num = find_inode_index(name);
	const char* filename = get_filename(name);
	inode_t *node = get_inode(inode_num);
	// once retrieved, decrement ref. count
	node->refs--;
	for (int i = 0; i < num_entries; i++) {
		if (contents[i].inum == inode_num && strcmp(contents[i].name,filename) == 0) {
			// don't free it unless we know no one else references it
			if (node->refs < 1) {
				free_inode(inode_num);
			}
			contents[i].inum = 0;
			memset(contents[i].name, 0, sizeof(contents[i].name));
			return inode_num;
		}
	}
	return -1;
}

// Return a list of the directory contents for given path.
slist_t *directory_list(const char *path) {
	slist_t* directory_listing = NULL;
	int num = find_inode_index(path);
	inode_t* directory = get_inode(num);
	direntry_t* contents = get_block_at(get_blist_at(directory->block_list)->block);
	for (int i = 0; i < (directory->size)/sizeof(direntry_t); i++) {
		if (strcmp(contents[i].name, "") != 0) {
			// cons it onto the directory listing
			directory_listing = s_cons(contents[i].name, directory_listing);
		}
	}
	return directory_listing;
}
