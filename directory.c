#include "directory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int initRootDirectory(int defaultEntries, uint64_t blockSize, FreeSpace* freeSpace, FileAllocationTable* fatTable, struct volume_control_block* vcb) {
    int sizeOfBlock = blockSize;

    int bytesNeeded = defaultEntries * sizeof(struct DirectoryEntry);
    int blocksNeeded = (bytesNeeded + (sizeOfBlock - 1)) / sizeOfBlock;
    bytesNeeded = blocksNeeded * sizeOfBlock;

    // Allocate memory for the directory entries
    struct DirectoryEntry *dir = (struct DirectoryEntry *)malloc(bytesNeeded);

    if (!dir) {
        perror("Failed to allocate memory for directory entries");
        return -1;
    }

    // Initialize directory entries
    for (int i = 2; i < defaultEntries; i++) {
        strcpy(dir[i].file_name, "");
        dir[i].isDirectory = 0; // Set the "isDirectory" field to 0 to mark entries as unused
    }

    // Calculate the starting block of the directory
    int startBlock = allocateFreeSpace(freeSpace, fatTable, bytesNeeded, vcb);

    printf("Allocate freespace 1\n");

    // "." directory entry
    strcpy(dir[0].file_name, ".");
    dir[0].file_size = bytesNeeded; // Adjust as needed
    dir[0].isDirectory = 1; // 1 denotes directory
    dir[0].ctime = time(NULL); // Set the creation time
    dir[0].mtime = time(NULL); // Set the modification time

    // ".." directory entry 
    strcpy(dir[1].file_name, "..");
    dir[1].file_size = bytesNeeded; // Adjust as needed
    dir[1].isDirectory = 1; // 1 denotes directory
    dir[1].ctime = time(NULL); // Set the creation time
    dir[1].mtime = time(NULL); // Set the modification time

    // Root directory is committed to disk
uint64_t blocksWritten = LBAwrite((unsigned char *)dir, blocksNeeded, 21);

    if (blocksWritten != blocksNeeded) {
        printf("Failed to write all directory blocks to disk.\n");
        free(dir);
        return -1;
    }

    free(dir);

    return startBlock;
}
