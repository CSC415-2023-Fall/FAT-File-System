/**************************************************************
* Class:  CSC-415-03 Fall 2021
* Names:Akshat Sohal
* Student IDs:917815046
* GitHub Name:sohal786
* Group Name:Tryhards
* Project: Basic File System
*
* File: directory.c
*
* Description: Initializing the root directory
*
* This file is where you will start and initialize your system
*
**************************************************************/

#include "directory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int initRootDirectory(int defaultEntries, uint64_t blockSize, FreeSpace* freeSpace, FileAllocationTable* fatTable, struct volume_control_block* vcb, char* name, char* type, struct DirectoryEntry* dirEntry, struct DirectoryEntry** parent) {
    int sizeOfBlock = blockSize;

    // Calculate bytes needed based on the number of default entries and their size
    int bytesNeeded = DEFAULT_ENTRIES * sizeof(struct DirectoryEntry);
    // Compute the number of blocks needed for the directory
    int blocksNeeded = (bytesNeeded + (sizeOfBlock - 1)) / sizeOfBlock;
    // Update bytesNeeded to the actual number of bytes after block allocation
    bytesNeeded = blocksNeeded * sizeOfBlock;

    // Allocate memory for the directory entries
    struct DirectoryEntry *dir = (struct DirectoryEntry *)malloc(bytesNeeded);
    if (!dir) {
        perror("Failed to allocate memory for directory entries");
        return -1;
    }

    // Initialize directory entries with the provided name and type
    for (int i = 2; i < defaultEntries; i++) {
        strcpy(dir[i].file_name, name);
        dir[i].isDirectory = *type;
        dir[i].location = 0;
    }

    // Allocate space for the directory entries on disk
    int startBlock = allocateFreeSpace(freeSpace, fatTable, bytesNeeded, vcb);
    printf("Allocate freespace 1\n");

    // Initialize the current directory (".") entry
    strcpy(dir[0].file_name, ".");
    dir[0].file_size = bytesNeeded;
    dir[0].isDirectory = 1;
    dir[0].ctime = time(NULL);
    dir[0].mtime = time(NULL);
    dir[0].location = startBlock;

    // Initialize the parent directory ("..") entry
    strcpy(dir[1].file_name, "..");
    dir[1].file_size = bytesNeeded;
    dir[1].isDirectory = 1;
    dir[1].ctime = time(NULL);
    dir[1].mtime = time(NULL);
    dir[1].location = startBlock;

    // Write the directory entries to disk
    uint64_t blocksWritten = LBAwrite((unsigned char *)dir, blocksNeeded, 21);
    if (blocksWritten != blocksNeeded) {
        printf("Failed to write all directory blocks to disk.\n");
        free(dir);
        return -1;
    }

    // Free the allocated memory for the directory entries
    free(dir);

    return startBlock;
}
