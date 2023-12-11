#include <string.h>

#ifndef HELPER_H
#define HELPER_H

// Skip the first character in the given string
// Utilized for skipping root in ls calls
char* skip_slash(char* path) {
    if (strlen(path) != 0) return path+1; 
    else return path;
}

#endif