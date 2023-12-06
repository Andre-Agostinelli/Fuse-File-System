#include "inode.h"
#include "bitmap.h"

// Print information about the given inode
void print_inode(inode_t *node) {
    if (node) { 
        printf("node mode: %04o, size: %d, ptrs[0]: %d, ptrs[1]: %d, iptr: %d}\n", node->mode, node->size,  node->ptrs[0], node->ptrs[1], node->iptr);
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
            printf("Allocated new inode at inode_bitmap[%d]\n" + ii);
            return ii; // return index
        }
    } 
    return -1; 
} 

void free_inode() {
    // going to need to implement for deleting files / nufs_unlink
} 

// Grow the inode by the given size
int grow_inode(inode_t *node, int size) {
    // needed for storage truncating when reading/writing
    node->size += size;
    return node->size;
}

int shrink_inode(inode_t *node, int size) {
    // needed for storage truncating when reading/writing
}

// get the memory location associated with the block index bnum for this inode
int inode_get_bnum(inode_t *node, int file_bnum) {
    // int block_index = file_bnum / BLOCK_SIZE;

    // // if file_bnum is greater than 2, it is larger than 2 blocks -> need indirection pointer     
    // return block_index < 2 ? node->ptrs[file_bnum] : ((int*)blocks_get_block(node->iptr))[block_index - 2];

    return node->ptrs[file_bnum];
}