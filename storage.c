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

int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
    // int truncate = storage_truncate(path, size + offset);

    // // truncate will check if inode is in tree & accordingly shrink/grow if needed
    // if (truncate < 0) {
    //     return truncate;
    // }
    
    int inum = tree_lookup(path); 
    if (inum < 0) {
        return inum; //inum was not found in path -> can't wirte
    }

    inode_t *node = get_inode(inum);
    print_inode(node);

    int written_so_far = 0;

    // char* data = blocks_get_block(inode_get_bnum(node, 0)); 

    int block_start = offset / BLOCK_SIZE;  //-- tells you which of ptrs[] block to read from 
    int block_offset = offset % BLOCK_SIZE; //-- tells you from where on ^that block to start writing
    void* data = blocks_get_block(node->ptrs[block_start]) + block_offset; //start of our write

    while (written_so_far < size) {
        // need to check how much is left on my current block...
        
    }


    // copy buffer into -> node data
    // return error if write to too little space -> 
    // for read u copy node stuff into buffer -> and can only read size left...


    //get block # corresponding with inode - inode_get_bnum?
    //block_get_block
    //need cursor to write - writeptr?
    // need to always know: 1. bytes left on page 2. left to write
    // need to be able to go to diff page if size is large -> supports more than 4k.

}

// 
int storage_truncate(const char *path, off_t size) {
    int inode_num = tree_lookup(path);
    if (inode_num < 0) {
        return inode_num;
    }

    inode_t* inode = get_inode(inode_num);
    if (size >= inode->size) {
        int rv = grow_inode(inode, size);
        return rv;
    } else {
        int rv = shrink_inode(inode, size);
        return rv;
    }
}

