#include "directory.h"
#include "inode.h"
#include "blocks.h"
#include "bitmap.h"
#include <string.h>

// Initialize the root directory
void directory_init() {
    printf("Initializing directory...\n");
    
    //marking block 1 as root
    void* block_bitmap = get_blocks_bitmap();
    bitmap_put(block_bitmap, 1, 1);

    // int root_inum = alloc_inode(); // should be 0 -> could be hardcoded?
    int root_inum = ROOT_INUM;
    void *inode_bitmap = get_inode_bitmap();
    bitmap_put(inode_bitmap, 0, 1); // mark first as used

    inode_t* root_inode = get_inode(root_inum); // will be 0

    // Initialize everything for the root inode...
    root_inode->mode = 040755; // set mode to directory
    root_inode->size = 256; // 256 files
    // root_inode->ptrs[0] = alloc_block(); // instead of allocating wasted space
    // root_inode->ptrs[0] = alloc_block(); // instead of allocating wasted space
    root_inode->block = -1; // instead of allocating wasted space
    root_inode->iptr = 0; //not allocated
    strcpy(root_inode->name, "/");

    // directory_put(root_inode, ".", root_inum);
}

// // Put the file given by pathname and inum at DIRECTORY di 
// int directory_put(inode_t *di, const char *name, int inum) {
//     dirent_t* entries = (dirent_t*) blocks_get_block(di->ptrs[0]); // get the directory's entries
//     for (int i = 0; i < MAX_ENTR; i++) {
//         // if (entries[i].inum == 0)
//     }
//     return -1; 
// }

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

int directory_lookup(inode_t *di, const char *name) { }

// Given the path for a file, returns its inum int
int tree_lookup(const char *path) {
    // printf("Searching for \'%s\' \n", path);
    // loop through all inodes
    for (int ii=0; ii<INODE_COUNT; ii++) {
        inode_t* cur_inode = get_inode(ii); // get cur inode
        // printf("inode[%d] has name \'%s\' \n", ii, cur_inode->name);
        if (strcmp(cur_inode->name, path) == 0) { // compare name to path
            // printf("returning ii: %d\n", ii);
            return ii; // return this inum
        } 
    }
    return -1; // didn't find the inode according to given path
}