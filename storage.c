#include "storage.h"
#include "blocks.h"
#include "bitmap.h" 

#define MIN(a, b) ((a) < (b) ? (a) : (b))

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

    // offset / BLOCK_SIZE  -- tells you which block to read from 
    // offset % BLOCK_SIZE  -- tells you from where on ^that block to start reading from

    // (offset + size) / BLOCK_SIZE  --- gives you the block you are ending on
    // (offset + size) % BLOCK_SIZE  --- tells you where exactly on ^that block you're ending

    // Just fill the buffer 'buf' with the contents of the file 
    // decrement size until it is 0 -- add assert to make sure it's 0 at the end which means you have read all the bytes

    // We are returning the number of bytes that transfer from the file to the buffer

    // // UNCOMMENT

    // // get the inode
    // int inum = tree_lookup(path);  // get inum from path
    // if (inum < 0) {
    //     return inum; //inum was not found in path -> can't wirte
    // }
    // inode_t *node = get_inode(inum);
    // printf("Reading file...\n");
    // print_inode(node);

    // // return 0 if the offset is at or beyond the end of the file
    // // * keep this the same
    // if (size + offset > node->size) {
    //     printf("size: %lu\n", size);
    //     printf("offset: %lu\n", offset);
    //     printf("Size+offset: %lu is bigger than inode size:%d\n", size+offset, node->size);
    //     return 0;
    // }
    // else {
    //     int block_start = offset / BLOCK_SIZE;  // get the file's starting block
    //     int block_offset = offset % BLOCK_SIZE; // where to start in ^ that block
    //     int block_end = (offset + size - 1) / BLOCK_SIZE;
    //     void* data = blocks_get_block(node->block) + block_offset; // pointer to the start point to read from
    //     // copied from below...
    //     // int bytes_remaining = BLOCK_SIZE - block_offset; // bytes remaining in the starting block ("block_start")
        
    //     memcpy(buf, data, size); // read the data from data->buf of amt size
    //     return size; 

    //     // Loop through each block and perform the read
    //     while (size > 0) {
    //         int blocks_read = 1;


    //         size -= blocks_read;
    //     }
    // }
    // END UNCOMMENT


    int inum = tree_lookup(path);
    if (inum < 0) {
        return inum;
    }
    inode_t *node = get_inode(inum);
    printf("+ storage_read(%s); inode %d\n", path, inum);
    print_inode(node);

    // can't read 
    if (offset >= node->size) { 
        return 0;
    }

    // adjust size if necessary
    if (offset + size >= node->size) { 
        size = node->size - offset;
    }

    int numRead = 0;

    // Determine the range of blocks affected by the read
    int block_start = offset / BLOCK_SIZE;
    int block_end = (offset + size - 1) / BLOCK_SIZE;

    // Initialize variables for tracking the read process
    int bytes_read = 0;
    int remaining_bytes = size;
    int current_offset = offset;

    for (int block_num = block_start; block_num <= block_end; ++block_num) {
            // Calculate the offset within the current block
            int block_offset = current_offset % BLOCK_SIZE;
            printf("  block_offset: %d\n", block_offset);

            // Get a pointer to the block where data will be written
            void* data_block = blocks_get_block(inode_get_bnum(node, block_num));
            printf("  getting block_num: %d\n", block_num);

            // Number of bytes that can be written to this block
            // We are either filling this entire block or only writing the amt of bytes remaining if that happens to be less
            int bytes_to_read = MIN(remaining_bytes, BLOCK_SIZE - block_offset);
            printf("  need to read: %d\n bytes", bytes_to_read);

            // Copy data from the buffer to the block
            // memcpy(data_block + block_offset, buf + bytes_written, bytes_to_read);
            memcpy(buf + bytes_read, data_block + block_offset,bytes_to_read);
            printf("  copied\n");

            // Update tracking variables
            current_offset += bytes_to_read;
            bytes_read += bytes_to_read;
            remaining_bytes -= bytes_to_read;
        }


    // // iterates until enough bytes are read
    // while (numRead < size) {
    //     int bnum = inode_get_bnum(node, offset + numRead);
    //     char *pagestart = blocks_get_block(bnum);
    //     char *readPtr = pagestart + ((offset + numRead) % 4096);

    //     int bytesToRead = 0;
    //     int leftOnPage = size - numRead;
    //     int pageRem = pagestart + 4096 - readPtr;
    //     (leftOnPage <= pageRem) ? (bytesToRead = leftOnPage) : (bytesToRead = pageRem);
    //     printf(" + reading from page: %d\n", bnum);
    //     memcpy(buf + numRead, readPtr, bytesToRead);
    //     numRead += bytesToRead;
    // }
    printf("Read %ld bytes\n", size);
    return size;
}

int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
    // Get the inode...
    int inum = tree_lookup(path);  // get inum from path
    if (inum < 0) {
        return inum; // inum was not found in path -> can't wirte
    }
    inode_t *node = get_inode(inum);
    print_inode(node);

    // Update the size of the file if necessary
    if (size + offset > node->size) {
        grow_inode(node, size + offset); // make the inode big enough to write this thing
        printf("Grew inode to size: %d\n", node->size);
    }

    // Determine the range of blocks affected by the write
    int block_start = offset / BLOCK_SIZE;
    int block_end = (offset + size - 1) / BLOCK_SIZE;

    // Initialize variables for tracking the write progress
    int bytes_written = 0;
    int remaining_bytes = size;
    int current_offset = offset;

    printf("  Starting block: %d\n", block_start);
    int block_offset = offset % BLOCK_SIZE;        // tells you from where on ^that block to start writing
    printf("  Offset: %d\n", block_offset);
    // void* data = blocks_get_block(node->block) + offset; // start of our write
    void* data = blocks_get_block(inode_get_bnum(node, block_start)) + offset; // start of our write
    printf("  Data: %p\n", data);
    int bytes_remaining = BLOCK_SIZE - block_offset; // bytes remaining in the starting block ("block_start")


    if (size < bytes_remaining) { // if we have enough room in the starting block
        memcpy(data, buf, size); // copy buf into data (size is how much) -> we know size fits into this block
    } else {
            // Loop through each block and perform the write
        for (int block_num = block_start; block_num <= block_end; ++block_num) {
            // Calculate the offset within the current block
            int block_offset = current_offset % BLOCK_SIZE;

            // Get a pointer to the block where data will be written
            void* data_block = blocks_get_block(inode_get_bnum(node, block_num));

            // Number of bytes that can be written to this block
            // We are either filling this entire block or only writing the amt of bytes remaining if that happens to be less
            int bytes_to_write = MIN(remaining_bytes, BLOCK_SIZE - block_offset);

            // Copy data from the buffer to the block
            memcpy(data_block + block_offset, buf + bytes_written, bytes_to_write);

            // Update tracking variables
            current_offset += bytes_to_write;
            bytes_written += bytes_to_write;
            remaining_bytes -= bytes_to_write;
        }
        printf("Wrote %d bytes\n", bytes_written);
        return bytes_written;
    }

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