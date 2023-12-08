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
    printf("  Growing inode %s\n", node->name);
    printf("  inode %s is currently of size %d\n", node->name, node->size);
    int num_blocks_used = bytes_to_blocks(node->size); // # of blocks used by this inode
    printf("  currently using %d blocks\n" + num_blocks_used);
    int num_blocks_desired = bytes_to_blocks(node->size + size); // # of blocks needed for cur inode size + size 
    printf("  want to use %d blocks\n" + num_blocks_desired);

    // check difference between the two (how many new blocks u want to alloc)
    int num_new_blocks = num_blocks_desired - num_blocks_used;
    printf("  Need %d new blocks\n", num_new_blocks);

    // Case where no new blocks are needed
    if (num_new_blocks == 0) {
        node->size += size; 
        printf("  No new blocks needed to grow to size %d\n", node->size);
        return node->size;
    }

    // We MAY need new blocks 
    else {
        printf("  May need more blocks\n");
        int* last_block = ((int*) blocks_get_block(inode_get_bnum(node, num_blocks_used - 1))); // the last block used by this 
        int space_remaining_on_last_block = BLOCK_SIZE - (node->size % BLOCK_SIZE);
        printf("  %d remaining on the last block of this inode\n", space_remaining_on_last_block);

        if (space_remaining_on_last_block >= size) { // if we have enough rooom on the last block, don't alloc new block
            node->size += size; 
            printf("  Grew inode %s to size %d bytes\n", node->name, node->size);
            return node->size;
        }
        // do not have enough room in the last block of this inode, need to alloc more blocks
        else {
            printf("  Need to allocate %d new blocks\n", num_new_blocks);
            if (node->iptr == -1) node->iptr = alloc_block(); // alloc the indirect block if necessary
            for (int i = num_blocks_used; i <= num_blocks_desired; ++i) { // allocate as many new blocks as you need
                int* indirection_block = blocks_get_block(node->iptr); // get a pointer to the block where the indirect blocks are stored
                indirection_block[i] = alloc_block(); // add a new block address to the indir. block
                printf("Added new indirection block\n");
            }
            node->size += size; 
            printf("Grew inode %s to size %d bytes\n", node->name, node->size);
            return node->size;
        }        
    }
    // node->size += size; 
    // return node->size;
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