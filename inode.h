// Inode manipulation routines.
//
// Feel free to use as inspiration. Provided as-is.

// based on cs3650 starter code
#ifndef INODE_H
#define INODE_H
#include "blocks.h"

#define INODE_COUNT 256

typedef struct inode {
  // int refs;     // nah
  int mode;        // permission & type
  int size;        // bytes written
  int block;       // 2 direct pointers?
  int iptr;        // single indirect pointer
  char name[16];   // big enough to be safe
} inode_t;

void print_inode(inode_t *node);
inode_t *get_inode(int inum);
int alloc_inode();
void free_inode(int inum);
int grow_inode(inode_t *node, int size);
int shrink_inode(inode_t *node, int size);
int inode_get_bnum(inode_t *node, int file_bnum);

#endif
