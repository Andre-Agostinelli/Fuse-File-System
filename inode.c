#include "inode.h"
#include "bitmap.h"
#include <errno.h>
#include <string.h>

// Print information about the given inode
void print_inode(inode_t *node) {
    if (node) { 
        printf("node '%s' has mode: %04o, size: %d, block:%d, iptr: %d}\n", node->name, node->mode, node->size, node->block, node->iptr);
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

    // free all direct and indirect data blocks
    free_data_blocks(node);

    // reset all its fields...
    node->mode = 0;
    node->size = 0;
    node->block = -1;
    node->iptr = -1;
    strcpy(node->name, "");

    bitmap_put(get_inode_bitmap(), inum, 0); // set this inode to free
} 

// Grow the inode by the given size, returning the new size
int grow_inode(inode_t *node, int size) {

    int blocks_required = bytes_to_blocks(size); // tells total blocks needed for an inode this size...

    // Case: can't fit in direct block
    if (blocks_required > 1) {
        blocks_required = blocks_required - 1;
        if (node->iptr == -1) node->iptr = alloc_block();           
        //should always have indirection block atp
        int* indirection_block = (int*) blocks_get_block(node->iptr); // get a pointer to the block where the indirect block bnums are stored
        
        for (int i = 0; i < blocks_required; ++i) {
            if (indirection_block[i] == 0) { //means new block must be alloc'd
				indirection_block[i] = alloc_block();
			}
		}
    }

    printf("growing inode: from size %d, to size: %d---------", node->size, size);
    node->size = size; //if indirect is not needed then we will drop through and just assign new node size corresponding to direct block.
	return node->size;
}

// Shrink the inode down to the given size, return the resulting size
int shrink_inode(inode_t *node, int size) {
    if (size >= node->size) {
        // Nothing to do if the requested size is greater or equal to the current size.
        return node->size;
    }

    int num_blocks_used = bytes_to_blocks(node->size); // # blocks currently used
    int num_blocks_desired = bytes_to_blocks(size);    // # blocks to be used after the shrink

    // Free unnecessary direct and indirect blocks
    for (int i = num_blocks_desired; i < num_blocks_used; ++i) {
        free_block(inode_get_bnum(node, i));
    }

    // Update the size
    node->size = size;

    printf("Shrunk inode %s to size %d bytes\n", node->name, node->size);

    return node->size;
}

// get the memory location associated with the block index bnum for this inode
int inode_get_bnum(inode_t *node, int file_bnum) { 
    if (file_bnum == 0)  return node->block;
    else {
        return ((int*) blocks_get_block(node->iptr))[file_bnum-1]; 
    }
    // return node->block; // simply return the memory address of this inode's block
}

// Free all direct and indirect blocks associated with this inode
void free_data_blocks(inode_t *node) {

    // get num of indirect blocks
    int num_indirect = bytes_to_blocks(node->size) - 1;

    // get direct block and free it
    free_block(node->block);

    for (int i=1; i<=num_indirect; ++i) {
        // get the block and free it 
        free_block(inode_get_bnum(node, i));
    }
} 