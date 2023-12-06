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
// Copied from cs.hmc.edu... 'Read size bytes from the given file into the buffer buf, beginning offset bytes into the file. See read(2) for full details. Returns the number of bytes transferred, or 0 if offset was at or beyond the end of the file. Required for any sensible filesystem.'
int storage_read(const char *path, char *buf, size_t size, off_t offset) {

    // get working with block 0 for now

    // get the inode
    int inum = tree_lookup(path);  // get inum from path
    if (inum < 0) {
        return inum; //inum was not found in path -> can't wirte
    }
    inode_t *node = get_inode(inum);
    printf("Reading file...\n");
    print_inode(node);

    // return 0 if the offset is at or beyond the end of the file
    if (size + offset > node->size) {
        printf("size: %lu\n", size);
        printf("offset: %lu\n", offset);
        printf("Size+offset: %lu is bigger than inode size:%d\n", size+offset, node->size);
        return 0;
    }
    else {
        int block_start = offset / BLOCK_SIZE;  // get the starting file's starting block
        int block_offset = offset % BLOCK_SIZE; // where to start in ^ that block
        void* data = blocks_get_block(node->block) + block_offset; // pointer to the start point to read from
        // copied from below...
        // int bytes_remaining = BLOCK_SIZE - block_offset; // bytes remaining in the starting block ("block_start")
        
        memcpy(buf, data, size); // read the data from data->buf of amt size
        return size; 
    }
}

int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
    // int truncate = storage_truncate(path, size + offset);

    // Get the inode...
    int inum = tree_lookup(path);  // get inum from path
    if (inum < 0) {
        return inum; //inum was not found in path -> can't wirte
    }
    inode_t *node = get_inode(inum);
    print_inode(node);

    // call inode grow if offset + size > inodes current size, grow the inode by the difference between those 2
    // ** ONLY GROW, NOT SHRINK ON WRITE **

    // Update the size of the file if necessary
    if (size + offset > node->size) {
        printf("Size+offset: %lu is bigger than inode size:%d\n", size+offset, node->size);
        grow_inode(node, size + offset); // make the inode big enough to write this thing
        printf("Grew inode to size: %d\n", node->size);
    }

    int written_so_far = 0;

    int block_start = offset / BLOCK_SIZE;  //-- tells you which of ptrs[] block to read from 
    printf("  Starting block: %d\n", block_start);
    int block_offset = offset % BLOCK_SIZE; //-- tells you from where on ^that block to start writing
    printf("  Offset: %d\n", block_offset);
    void* data = blocks_get_block(node->block) + offset; // start of our write
    printf("  Data: %p\n", data);
    int bytes_remaining = BLOCK_SIZE - block_offset; // bytes remaining in the starting block ("block_start")


    if (size < bytes_remaining) { // if we have enough room in the starting block
        memcpy(data, buf, size); // copy buf into data (size is how much) -> we know size fits into this block
        printf("  Writing data: %p\n", data);
    } 
    // else {
    //     // write amount bytes_remaining,
    //     // start at beginning of block[1]
    //     // keep writing until written_so_far == size OR 
    //     while (written_so_far < size) {
    //         void* data = blocks_get_block(node->ptrs[block_start]) + block_offset; // where to write from
    //     }
        
    // }

    // using the size, figure out how many blocks you're going to be affecting

    return size; 
}

// Don't need to grow inodes until we start using the indirection pointer
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

