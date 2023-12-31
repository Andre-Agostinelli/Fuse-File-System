#include "directory.h"
#include "inode.h"
#include "blocks.h"
#include "bitmap.h"
#include <string.h>

// Initialize the root directory
void directory_init() {    
    bitmap_put(get_blocks_bitmap(), 1, 1); // marking block 1 as occupied for inodes
    bitmap_put(get_blocks_bitmap(), 2, 1); // marking block 2 as occupied for inodes
    bitmap_put(get_inode_bitmap(), 0, 1); // mark inode 0 as occupied for root directory

    int root_inum = ROOT_INUM; // ROOT_INUM is 0
    inode_t* root_inode = get_inode(root_inum); // will be 0

    // Initialize everything for the root inode...
    root_inode->mode = 040755; // set mode to directory
    root_inode->size = 256; // max 256 files
    root_inode->block = -1; // instead of allocating wasted space
    root_inode->iptr = -1; // not allocated
    strcpy(root_inode->name, "/");
}

// Given the path for a file, returns its inum int
int tree_lookup(const char *path) {
    for (int ii=0; ii<INODE_COUNT; ii++) {        // loop through all inodes
        inode_t* cur_inode = get_inode(ii);       // get cur inode
        if (strcmp(cur_inode->name, path) == 0) { // compare name to path
            return ii;                            // return this inum
        } 
    }
    return -1; // couldn't find inode from given path
}