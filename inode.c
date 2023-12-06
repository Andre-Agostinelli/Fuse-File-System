#include "inode.h"
#include "bitmap.h"
#include <errno.h>
#include <string.h>

// Print information about the given inode
void print_inode(inode_t *node) {
    if (node) { 
        printf("node mode: %04o, size: %d, block:%d, iptr: %d}\n", node->mode, node->size,  node->block, node->iptr);
    }
    else {
        printf("node{null}\n");
    }
}

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
            printf("Allocated new inode at inode_bitmap[%d]\n", ii);
            return ii; // return index
        }
    } 
    return -1; 
} 

// Free the inode given by inum
void free_inode(int inum) {
    inode_t *node = get_inode(inum); // get this inode

    blocks_free(node->block); // free its block

    // reset all its fields...
    node->mode = 0;
    node->size = 0;
    node->block = -1;
    node->iptr = -1;
    strcpy(node->name, "");

    bitmap_put(get_inode_bitmap(), inum, 0); // set this inode to free
} 

// Grow the inode by the given size
int grow_inode(inode_t *node, int size) {
    node->size += size; 
    return node->size;
}

int shrink_inode(inode_t *node, int size) {
    // needed for storage truncating when reading/writing
}

// get the memory location associated with the block index bnum for this inode
int inode_get_bnum(inode_t *node, int file_bnum) {
    return node->block; // simply return the memory address of this inode's block
}