# Project 2: File System by Andre Agostinelli and Isaac Levine

# Design

This project implements a simple file system with the following design considerations:

## Block Structure

- Block 0 stores the block bitmap and inode bitmap.
  - The block bitmap is of size `BLOCK_BITMAP_SIZE` (32) and starts at the base of block 0.
  - The inode bitmap starts at block 0 + 32 (also size 32).

## Inodes

- The file system stores 256 inodes.
- Blocks 1 and 2 are used to store inodes.
- Each inode is of size 32, (32*256) = 8192 bytes = 2 blocks
- Each inode has 1 direct pointer block, and 1 indirect pointer block.
- Each inode has a name field. Note that our inode structs actually store the "/" in the inode's name. However, in readdir, we skip over that slash and just print "hello.txt" instead of "/hello.txt," even though we still actually store the '/' in the inode's name.
- Psuedo-Directory handling - 'cd' and 'ls' are not supported for these 'directories' as you cannot 'cd' into them, however they will display and are stored correctly in the root block.

## Data

- Data blocks encompass the entirety of the rest of the filesystem, meaning 256 - (3 blocks used: 1 init bitmaps, and 2 for inodes) = 253 open data blocks.
- 1 indirect block can store pointers to 1024 blocks: BLOCK_SIZE (4096) / INT_SIZE (4) = 1024
  - However, we only have 253 blocks available for data...
  - Meaning the max file size possible is: Assuming using 1 direct block fully, and using indirect block for address, 253 - 1 = 252 blocks x 4096 bytes = 1,032,192 Bytes -> 1.032192 MB (assuming no other data/blocks are in use on disk...)

## Supported Functionality:

  - Create files, supporting files with names at least 10 characters long
  - List the files in the filesystem root directory (where you mounted it)
  - Write to small files (under 4k)
  - Read from small files (under 4k)
  - Rename files
  - Delete files

    # Extension:

    - Create Directories
    - List Directories in the root
      - Does not support 'cd' into or only 'ls' things in given directories due to way they are stored...
    - Writing and Reading to at least 100 files with size 4K
      - Can store max 253 files of 4k and is only limited by disk image sizing
    - Writing and Reading to at least 5 files with size 100K
      - Can store max (253 open blocks / (25 blocks needed for 100k + 1 block of indirection pointers)) = 9 files with 100k size
    - Writing and Reading to at least 1 file with size 500K
      - Can store max (253 open blocks / (123 blocks needed for 500k + 1 block of indirection)) = 2 files with 500k size
    - * NOTE: ALL PREVIOUS ASSUMPTIONS WERE MADE BASED ON A CLEAN/NEW DISK *


## [Project 2 Assignment Page](https://khoury-cs3650.github.io/p2.html)

- [Makefile](Makefile)   - Targets are explained in the assignment text
- [README.md](README.md) - This README
- [helpers](helpers)     - Helper code implementing access to bitmaps and blocks
- [hints](hints)         - Incomplete bits and pieces that you might want to use as inspiration
- [nufs.c](nufs.c)       - The main file of the file system driver
- [test.pl](test.pl)     - Tests to exercise the file system

## Running the tests

You might need install an additional package to run the provided tests as well as one to use the fuse filesystem:

```
$ sudo apt-get install libtest-simple-perl

$ sudo apt-get install libfuse-dev libbsd-dev pkg-config
// Install's these packages:
libfuse-dev
libbsd-dev
pkg-config
```
## Makefile:

- Supported make commands:

- [test](test) - Running 'make test' will run 32 test coverage 
- [clean](clean) - Running 'make clean' will unmount your image and remove all executables
- [nufs](nufs) - Running 'make nufs' will compile all the nufs code and included files
- [mount](mount) - Running 'make mount' will run the fuse filesystem and is the main project driver
  - This calls pairs with 'cd' into the mount and using our filesystem...
- [unmount](mount) - Running 'make unmount' will remove your mounted disk image 

