#include "inode.h"
#include "bitmap.h"

// typedef struct inode {
//   // int refs;  // reference count
//   int mode;  // permission & type
//   int size;  // bytes
//   int block; // single block pointer (if max file size <= 4K)
// } inode_t;

void print_inode(inode_t *node) {}

// get the inode struct corresponding to inum
inode_t *get_inode(int inum) {
    return (inode_t*) (blocks_get_block(1) + (inum * sizeof(inode_t)));
}

// get the first available inode 
int alloc_inode() {
    // get a pointer inode bitmap
    void *inode_bitmap = get_inode_bitmap(); 
    for (int ii = 0; ii<INODE_COUNT; ++ii) {
        if (!bitmap_get(inode_bitmap, ii)) { // if free 
            bitmap_put(inode_bitmap, ii, 1); // mark as used
            return ii; // return index
        }
    } 
    return -1; 
} 

void free_inode() {} 

int grow_inode(inode_t *node, int size) {}

int shrink_inode(inode_t *node, int size) {}

int inode_get_bnum(inode_t *node, int file_bnum);