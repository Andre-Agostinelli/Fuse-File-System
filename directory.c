#include "directory.h"
#include "inode.h"
#include "blocks.h"



// Initialize the root directory
void directory_init() {

    void* inode_bitmap = get_inode_bitmap();
    int root_inum = alloc_inode(); 

    inode_t* root_inode = get_inode(root_inum);

    // Initialize everything for the root inode...
    root_inode->mode = 040755; // set mode to directory
    root_inode->size = 0; 
    root_inode->ptrs[0] = alloc_inode(); 
    root_inode->ptrs[1] = alloc_inode(); 
    root_inode->iptr = 0; 

    directory_put(root_inode, ".", root_inum);
}

// Put the file given by pathname and inum at DIRECTORY di 
int directory_put(inode_t *di, const char *name, int inum) {
    dirent_t* entries = (dirent_t*) blocks_get_block(di->ptrs[0]); // get the directory's entries
    for (int i = 0; i < MAX_ENTR; i++) {
        // if (entries[i].inum == 0)
    }
    return -1; 
}

/*
 * Returns the data of the given path's furthest node...
 */
char* directory_get_name(const char* path) {
    slist_t* list = s_explode(path, '/');
    while(list->next) {
        list = list->next;
    }
    return list->data;
}