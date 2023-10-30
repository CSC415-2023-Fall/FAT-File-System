#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <sys/types.h>
#include <time.h>
#include <stdint.h>

#include "freespace.h"
#include "fsInit.h"

#define DEFAULT_ENTRIES 16
#define MAXDIRENTRIES 50
#define MAX_FILE_NAME_LENGTH 256

typedef struct FreeSpace FreeSpace;
typedef struct FileAllocationTable FileAllocationTable;


// Define the structure for Directory Entry (DE)
struct DirectoryEntry {
    char file_name[MAX_FILE_NAME_LENGTH];
    uint64_t file_size;
    time_t mtime;
    time_t atime;
    time_t ctime;
    char isDirectory;
};

// Function prototype
int initRootDirectory(int defaultEntries, uint64_t blockSize, FreeSpace* freeSpace, FileAllocationTable* fatTable, struct volume_control_block* vcb);

#endif // DIRECTORY_H
