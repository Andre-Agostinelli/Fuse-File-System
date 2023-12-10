# Project 2: File System

[Project 2](https://khoury-cs3650.github.io/p2.html) by Andre Agostinelli and Isaac Levine.

- [Makefile](Makefile)   - Targets are explained in the assignment text
- [README.md](README.md) - This README
- [helpers](helpers)     - Helper code implementing access to bitmaps and blocks
- [hints](hints)         - Incomplete bits and pieces that you might want to use as inspiration
- [nufs.c](nufs.c)       - The main file of the file system driver
- [test.pl](test.pl)     - Tests to exercise the file system

## Running the tests

You might need install an additional package to run the provided tests:

```
$ sudo apt-get install libtest-simple-perl
```

Then using `make test` will run the provided tests.

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