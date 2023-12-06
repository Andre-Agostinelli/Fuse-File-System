// Directory manipulation functions.
//
// Feel free to use as inspiration. Provided as-is.

// Based on cs3650 starter code
#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "blocks.h"
#include "inode.h"
#include "slist.h"

// We don't use this
typedef struct dirent {
  char name[DIR_NAME_LENGTH];
  int inum;
  char _reserved[12];
} dirent_t;

#define MAX_ENTR (4096 / sizeof(dirent_t))

#define ROOT_INUM 0

void directory_init();
int directory_lookup(inode_t *di, const char *name);
int directory_put(inode_t *di, const char *name, int inum);
int directory_delete(inode_t *di, const char *name);
slist_t *directory_list(const char *path);
void print_directory(inode_t *dd);
int tree_lookup(const char *path);

#endif
