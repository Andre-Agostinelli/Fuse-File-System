// based on cs3650 starter code

#include <assert.h>
#include <bsd/string.h>
// #include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "storage.h"
#include "blocks.h"
#include "inode.h"
#include "helper.h"

#define FUSE_USE_VERSION 26
#include <fuse.h>

// implementation for: man 2 access
// Checks if a file exists.
// This is the same as the access(2) system call. It returns -ENOENT if the path doesn't exist, -EACCESS if the requested permission isn't available, or 0 for success. Note that it can be called on files, directories, or any other object that appears in the filesystem. This call is not required but is highly recommended.
int nufs_access(const char *path, int mask) {
  int rv = -ENOENT;

  // We can always access the root directory...
  if (strcmp(path, "/") == 0) {
    rv = 0; // want to return success
  }

  // Get inum corresponding to supplied path
  int inum = tree_lookup(path);

  // if supplied path did not correspond to file, return -ENOENT
  if (inum == -1) rv = -ENOENT;
  else rv = 0; // inum != -1 means we found a valid file, want to return success

  printf("access(%s, %04o) -> %d\n", path, mask, rv);
  return rv;
}

// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
int nufs_getattr(const char *path, struct stat *st) {
  int rv = storage_stat(path, st);

  if (rv == -1) return -ENOENT;

  printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode,
         st->st_size);
  return rv;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {
  struct stat st;
  int rv;

  // Get the root file's attributes in the st struct
  rv = nufs_getattr("/", &st);
  // assert(rv == 0);

  filler(buf, ".", &st, 0); // add "." 
  printf(".\n"); // print . first

  // add all the names of our inodes
  for (int ii=0; ii<INODE_COUNT; ii++) {
    inode_t* cur_inode = get_inode(ii); // get cur inode
    // printf("Printing inode:  ");
    // print_inode(cur_inode);
    if (strcmp(skip_slash(cur_inode->name), "") != 0) { // if this inode's name is not empty
      printf("%s\n", skip_slash(cur_inode->name));
      filler(buf, skip_slash(cur_inode->name), &st, 0); // add this name without the slash
    } 
  }  

  assert(rv == 0);
  printf("readdir(%s) -> %d\n", path, rv);
  return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
// Note, for this assignment, you can alternatively implement the create
// function.
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
  int rv = 0;

  int inum = alloc_inode();
  if (inum == -1) {
    return -ENOENT;
  }
	inode_t* newnode = get_inode(inum);
  newnode->block = alloc_block();
  newnode->mode = mode;
  newnode->iptr = alloc_block();
  newnode->size = 0;
  strcpy(newnode->name, path);

  printf("mknod '%s' (%s, %04o) -> %d\n", newnode->name, path, mode, rv);
  return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
// leave as is...
int nufs_mkdir(const char *path, mode_t mode) {
  int rv = nufs_mknod(path, mode | 040000, 0);
  printf("mkdir(%s) -> %d\n", path, rv);
  return rv;
}

// Copied from cs.hmc.edu...'Remove (delete) the given file, symbolic link, hard link, or special node. Note that if you support hard links, unlink only deletes the data when the last hard link is removed. See unlink(2) for details.'
int nufs_unlink(const char *path) {
  int inum = tree_lookup(path);
  free_inode(inum);
  int rv = 0;
  printf("unlink(%s) -> %d\n", path, rv);
  return rv;
}

int nufs_link(const char *from, const char *to) {
  int rv = -1;
  printf("link(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

int nufs_rmdir(const char *path) {
  int rv = -1;
  printf("rmdir(%s) -> %d\n", path, rv);
  return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
// Rename the file, directory, or other object "from" to the target "to". Note that the source and target don't have to be in the same directory, so it may be necessary to move the source to an entirely new directory. See rename(2) for full details.
int nufs_rename(const char *from, const char *to) {
  int rv = 0;
  int inum = tree_lookup(from); // get inum from 'from' path
  if (inum == -1) return -1;
  inode_t* inode = get_inode(inum); // get inode from inum
  strcpy(inode->name, to); // change name to to

  printf("rename(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

int nufs_chmod(const char *path, mode_t mode) {
  int rv = -1;
  printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

int nufs_truncate(const char *path, off_t size) {
  printf("nufs_truncate SIZE: %ld\n", size);
  int rv = storage_truncate(path, size);
  printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
  return rv;
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi) {
  int rv = 0;
  printf("open(%s) -> %d\n", path, rv);
  return rv;
}

// Actually read data
// Copied from cs.hmc.edu... 'Read size bytes from the given file into the buffer buf, beginning offset bytes into the file. See read(2) for full details. Returns the number of bytes transferred, or 0 if offset was at or beyond the end of the file. Required for any sensible filesystem.'
int nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  // size = get_inode(tree_lookup(path))->size;
  int rv = storage_read(path, buf, size, offset);
  printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  printf("%s", buf); // print the contets of the file
  return rv;
}

// Actually write data
// Copied from cs.hmc.edu... 'Same as for read above, except that it can't return 0.'
int nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  int rv = storage_write(path, buf, size, offset); 
  printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Update the timestamps on a file or directory.
int nufs_utimens(const char *path, const struct timespec ts[2]) {
  int rv = 0;
  printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n", path, ts[0].tv_sec,
         ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
  return rv;
}

// Extended operations
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data) {
  int rv = -1;
  printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
  return rv;
}

void nufs_init_ops(struct fuse_operations *ops) {
  memset(ops, 0, sizeof(struct fuse_operations));
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->readdir = nufs_readdir;
  ops->mknod = nufs_mknod;
  // ops->create   = nufs_create; // alternative to mknod
  ops->mkdir = nufs_mkdir;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rmdir = nufs_rmdir;
  ops->rename = nufs_rename;
  ops->chmod = nufs_chmod;
  ops->truncate = nufs_truncate;
  ops->open = nufs_open;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int main(int argc, char *argv[]) {
  assert(argc > 2 && argc < 6);
  // printf("TODO: mount %s as data file\n", argv[--argc]);
  storage_init(argv[--argc]);
  nufs_init_ops(&nufs_ops);
  return fuse_main(argc, argv, &nufs_ops, NULL);
}
