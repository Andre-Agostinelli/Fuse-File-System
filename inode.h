// Inode manipulation routines.
//
// Feel free to use as inspiration. Provided as-is.

// based on cs3650 starter code
#ifndef INODE_H
#define INODE_H
#include "blocks.h"

// Max inodes possible = (4096*2blocks)/sizeof(inode) = 256
#define INODE_COUNT 256

typedef struct inode {
  // int refs;     // not used
  int mode;        // permission & type
  int size;        // # bytes written
  int block;       // 1 direct pointer
  int iptr;        // single indirect pointer
  char name[16];   // big enough to be safe
} inode_t;

// Prints inode.
void print_inode(inode_t *node);

// Returns the actual inode struct at the given inode index.
inode_t *get_inode(int inum);

// Simply marks inode bitmap and returns index of inode.
int alloc_inode();

// Essentially unmarks bitmap so space can be written over and returns index of inode.
void free_inode(int inum);

// Grows inode to the size required and returns said size.
int grow_inode(inode_t *node, int size);

// Shrinks inode by freeing blocks utilized and adjusting size. Returns size.
int shrink_inode(inode_t *node, int size);

// Gets the actual block corresponding to the inode's block number index.
int inode_get_bnum(inode_t *node, int file_bnum);

// Free's all blocks currently in use by inodes.
void free_data_blocks(inode_t *node);

#endif
