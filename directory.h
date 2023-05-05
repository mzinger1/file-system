/* Directory manipulation functions. */

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "blocks.h"
#include "inode.h"
#include "slist.h"

typedef struct direntry {
	char name[DIR_NAME_LENGTH];
	int inum;
	char _reserved[12];
} direntry_t;

// Initialize the root directory.
void directory_init();

// Return the relative path name from the given absolute path.
const char *get_filename(const char *path);

// Find the inode index of a file, given the directory that contains the file and the name of the file.
int find_file_in_dir(inode_t *dir, const char *name);

// Find the inode index of the parent of the given path.
int parent_inode_index(const char *path);

// Find the inode index of the given path.
int find_inode_index(const char *path);

// Put a file with the given name and inode index underneath directory dir.
int directory_put(inode_t *dir, const char *name, int inum);

// Remove the given path from the given directory.
int directory_delete(inode_t *dir, const char *name);

// Return a list of the directory contents for given path.
slist_t *directory_list(const char *path);

#endif
