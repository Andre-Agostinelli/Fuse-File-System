#include "directory.h"
#include "inode.h"
#include "blocks.h"
#include "bitmap.h"
#include <string.h>

// Initialize the root directory
void directory_init() {    
    bitmap_put(get_blocks_bitmap(), 1, 1); // marking block 1 as occupied for root
    bitmap_put(get_inode_bitmap(), 0, 1); // mark inode 0 as occupied for root

    int root_inum = ROOT_INUM; // ROOT_INUM is 0
    inode_t* root_inode = get_inode(root_inum); // will be 0

    // Initialize everything for the root inode...
    root_inode->mode = 040755; // set mode to directory
    root_inode->size = 256; // 256 files
    root_inode->block = -1; // instead of allocating wasted space
    root_inode->iptr = 0; // not allocated
    strcpy(root_inode->name, "/");
}

/*
 * Returns the data of the given path's furthest node...
 */
char* directory_get_name(const char* path) {
    slist_t* list = s_explode(path, '/');
    while(list->next) {
        list = list->next; // will always return furthest right string
    }
    return list->data;
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