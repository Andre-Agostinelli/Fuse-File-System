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
// int storage_read(const char *path, char *buf, size_t size, off_t offset) {
//     int inum = tree_lookup(path);
//     if (inum < 0) {
//         return inum;
//     }
//     inode_t *node = get_inode(inum);
//     printf("+ storage_read(%s); inode %d\n", path, inum);
//     print_inode(node);

//     // can't read 
//     if (offset >= node->size) { 
//         return 0;
//     }

//     // adjust size if necessary
//     if (offset + size >= node->size) { 
//         size = node->size - offset;
//     }

//     int numRead = 0;

//     // Determine the range of blocks affected by the read
//     int block_start = offset / BLOCK_SIZE;
//     int block_end = (offset + size - 1) / BLOCK_SIZE;

//     // Initialize variables for tracking the read process
//     int bytes_read = 0;
//     int remaining_bytes = size;
//     int current_offset = offset;

//     for (int block_num = block_start; block_num <= block_end; ++block_num) {
//             // Calculate the offset within the current block
//             int block_offset = current_offset % BLOCK_SIZE;
//             printf("  block_offset: %d\n", block_offset);

//             // Get a pointer to the block where data will be written
//             void* data_block = blocks_get_block(inode_get_bnum(node, block_num));
//             printf("  getting block_num: %d\n", block_num);

//             // Number of bytes that can be written to this block
//             // We are either filling this entire block or only writing the amt of bytes remaining if that happens to be less
//             int bytes_to_read = MIN(remaining_bytes, BLOCK_SIZE - block_offset);
//             printf("  bytes_to_read: %d\n bytes", bytes_to_read);

//             // Copy data from the buffer to the block
//             // memcpy(data_block + block_offset, buf + bytes_written, bytes_to_read);
//             memcpy(buf + bytes_read, data_block + block_offset, bytes_to_read);
//             printf("  copied %d bytes\n", bytes_to_read);

//             // Update tracking variables
//             current_offset += bytes_to_read;
//             bytes_read += bytes_to_read;
//             remaining_bytes -= bytes_to_read;
//         }
//     printf("Read %ld bytes\n", size);
//     return size;
// }

// 'Read size bytes from the given file into the buffer buf, beginning offset bytes into the file.
int storage_read(const char *path, char *buf, size_t size, off_t offset) {
    int inum = tree_lookup(path);
    if (inum < 0) {
        return inum;
    }
    inode_t *node = get_inode(inum);
    printf("+ storage_read(%s); inode %d\n", path, inum);
    print_inode(node);

    printf("  Reading %ld bytes from the file %s, starting %ld bytes into the file's data\n", size, node->name, offset);

    // ensures offset is accurate
    if (offset >= node->size) {
        printf("  Offset inaccurate, returning 0.\n");
        return 0;
    }

    // Adjust size of read if attempting to read beyond cur size
    if (offset + size >= node->size) {
        int oldsize = size; 
        size = node->size - offset;
        printf("  File %s only has %d bytes, so adjusting size to %ld. No need to try and read %d bytes...\n", node->name, node->size, size, oldsize);
    }

    // size equals the number of bytes that we WANT to read

    // Determine the range of blocks affected by the read
    int block_start = offset / BLOCK_SIZE;
    int block_end = (offset + size - 1) / BLOCK_SIZE;
    printf("  Read will start from block %d\n", block_start);
    printf("  Read will end on block %d\n", block_end);

    // Initialize variables for tracking the write progress
    int bytes_read_so_far = 0;    // the number of bytes we have read SO FAR
    int remaining_bytes = size;   // # of bytes left to read 
    int current_offset = offset;  // the current offset within the current block

    // void* data = blocks_get_block(inode_get_bnum(node, block_start)) + offset; // start of our read
    int block_offset = offset % BLOCK_SIZE;          // how far into that block to start reading
    void* data = blocks_get_block(inode_get_bnum(node, block_start)) + block_offset; // start of our read

    int bytes_remaining_in_starting_block = BLOCK_SIZE - block_offset; // bytes remaining in the starting block ("block_start")

    if (size < bytes_remaining_in_starting_block) { // if we have enough room in the starting block to finish the read
        printf("Just reading from the block we started on\n");
        memcpy(buf, data, size); // copy #size bytes into buf from data 
    } else {
        // Loop through each block and perform the read
        for (int block_num = block_start; block_num <= block_end; ++block_num) {
            // Get a pointer to the block where data will be read from
            // alloc block if it doesnt exist? 
            void* data_block = blocks_get_block(inode_get_bnum(node, block_num));
            printf("  getting block_num: %d\n", block_num);

            // Calculate the offset within the current block
            int block_offset = current_offset % BLOCK_SIZE;
            printf("  block_offset (within the current block): %d\n", block_offset);

            // Number of bytes that can be read from this block
            // Either reading this entire block or only the amt of bytes remaining to read if that happens to be less
            int bytes_reading_from_block = MIN(remaining_bytes, BLOCK_SIZE - block_offset);
            printf("  read %d bytes from this block\n", bytes_reading_from_block);

            // Copy data from the block to the buffer
            memcpy(buf + bytes_read_so_far, data_block + block_offset, bytes_reading_from_block);
            printf("  copied %d bytes into the buffer, starting at %d\n", bytes_reading_from_block, bytes_read_so_far);
            

            // Update tracking variables
            current_offset += bytes_reading_from_block;
            bytes_read_so_far += bytes_reading_from_block;
            remaining_bytes -= bytes_reading_from_block;
            printf("  current offset is now %d\n", current_offset);
            printf("  we have read %d bytes so far\n", bytes_read_so_far);
            printf("  %d bytes left to read\n", remaining_bytes);
        }
        printf("  read %d bytes TOTAL. The write is now complete.\n", bytes_read_so_far);
        return bytes_read_so_far;
    }

    return size;
}

int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
    storage_truncate(path, size+offset); // adjust the size of the file if necessary
    // Get the inode...
    int inum = tree_lookup(path);  // get inum from path
    if (inum < 0) {
        return inum; // inum was not found in path -> can't wirte
    }
    inode_t *node = get_inode(inum);
    printf("+ storage_write(%s); inode %d\n", path, inum);
    print_inode(node);

    // Determine the range of blocks affected by the write
    int block_start = offset / BLOCK_SIZE;
    int block_end = (offset + size) / BLOCK_SIZE;

    // Initialize variables for tracking the write progress
    int bytes_written = 0;
    int remaining_bytes = size;
    int current_offset = offset;

    printf("  Starting block: %d\n", block_start);
    int block_offset = offset % BLOCK_SIZE;        // tells you from where on ^that block to start writing
    printf("  Offset: %d\n", block_offset);
    // void* data = blocks_get_block(node->block) + offset; // start of our write
    void* data = blocks_get_block(inode_get_bnum(node, block_start)) + offset; // start of our write
    printf("Writing to %d\n", inode_get_bnum(node, block_start));
    printf("  Data: %p\n", data);
    int bytes_remaining = BLOCK_SIZE - block_offset; // bytes remaining in the starting block ("block_start")


    if (size < bytes_remaining) { // if we have enough room in the starting block
        memcpy(data, buf, size); // copy buf into data (size is how much) -> we know size fits into this block
    } else {
            // Loop through each block and perform the write
        for (int block_num = block_start; block_num <= block_end; ++block_num) {
            // Calculate the offset within the current block
            int block_offset = current_offset % BLOCK_SIZE;
            printf("  block_offset: %d\n", block_offset);

            // Get a pointer to the block where data will be written
            // alloc block if it doesnt exit
            void* data_block = blocks_get_block(inode_get_bnum(node, block_num));
            printf("  getting block_num: %d\n", block_num);

            // Number of bytes that can be written to this block
            // We are either filling this entire block or only writing the amt of bytes remaining if that happens to be less
            int bytes_to_write = MIN(remaining_bytes, BLOCK_SIZE - block_offset);
            printf("  bytes_to_write: %d bytes\n", bytes_to_write);

            // Copy data from the buffer to the block
            memcpy(data_block + block_offset, buf + bytes_written, bytes_to_write);
            printf("  copied %d bytes\n", bytes_to_write);
            

            // Update tracking variables
            current_offset += bytes_to_write;
            bytes_written += bytes_to_write;
            remaining_bytes -= bytes_to_write;
        }
        printf("  wrote %d bytes\n", bytes_written);
        return bytes_written;
    }

    // using the size, figure out how many blocks you're going to be affecting

    return size; 
}

// Truncate or extend the given file so that it is precisely size bytes long.
int storage_truncate(const char *path, off_t size) {
    printf("storage_truncate SIZE: %ld\n", size);
    int inode_num = tree_lookup(path);
    if (inode_num < 0) {
        return inode_num;
    }

    inode_t* inode = get_inode(inode_num);
    if (size > inode->size) {
        int rv = grow_inode(inode, (size-(inode->size))); // grow the inode by exactly this many bytes (size-(inode->size)) is exactly how many bytes we need
        printf("storage_truncate returning %d\n", rv);
        return rv;
    } 
    else {
        int rv = shrink_inode(inode, size);
        return rv;
    }
}