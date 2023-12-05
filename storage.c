#include "storage.h"
#include "blocks.h"
#include "bitmap.h"

// initialize the blocks and the root directory
void storage_init(const char *path) {
    blocks_init(path); // sets block 0 to used for both bitmaps

    void* block_bitmap = get_blocks_bitmap();
    // if this system is new... then we need new root, else keep old
    if (bitmap_get(block_bitmap, 1) == 0) {
        directory_init(); 
    }
}

// Get attributes based on path and fill given storage struct
int storage_stat(const char *path, struct stat *st) {
    
    // get the inum corresponding to this path using tree lookup
    int inum = tree_lookup(path);  // looks at all our inodes and gets the corresp. inum to this path

    if (inum == -1) return -1;

    inode_t* node = get_inode(inum);   // get the inode from that inum
    print_inode(node);

    // assign everything to stat according to inum
    st->st_size = node->size;
    st->st_mode = node->mode;
    st->st_uid = getuid();
    printf("+ storage_stat(%s) -> 0; inode %d\n", path, inum);
    return 0; 
}

// Read all the files we have and fill the buffer
int storage_read(const char *path, char *buf, size_t size, off_t offset) {
    
}