#include "storage.h"
#include "blocks.h"

// initialize 
void storage_init(const char *path) {
    blocks_init(path);
    directory_init();
}