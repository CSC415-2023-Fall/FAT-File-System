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
#include "mfs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>  // Ensure time.h is included for the time(NULL) function

// Function to allocate a number of blocks in the FAT (from freespace.c)
extern uint32_t allocateBlocks(int numberofBlocks);
extern struct volume_control_block *vcb;
extern struct FATEntry *fatTable;
DirectoryEntry *rootDir = NULL;
DirectoryEntry *currentDir = NULL;
char * cwd = NULL;


int loadRootDirectory() {
    printf("[loadRootDirectory] Loading root directory.\n");
    int minBytesNeeded = 16 * sizeof(DirectoryEntry);
    int blocksNeeded = (minBytesNeeded + 512 - 1) / 512;
    int mallocBytes = blocksNeeded * 512;

    int startBlock = 402;
    int blockSize = 512;
    rootDir = (DirectoryEntry *)malloc(mallocBytes);

    if (rootDir == NULL) {
        fprintf(stderr, "[LOAD ROOT] Failed to allocate memory for the root directory\n");
        return -1;
    }

    printf("[LOAD ROOT] Start root block: %d\n", startBlock);
    printf("[LOAD ROOT] Number of blocks for root: %d\n", blocksNeeded);

    // Read the root directory from the disk
    if (readDirectoryFromDisk(startBlock, rootDir, 10) != 0) {
        fprintf(stderr, "[LOAD ROOT] Failed to load root directory from disk\n");
        free(rootDir);
        return -1;
    }

    currentDir = rootDir; // Now cwd points to the root directory

    // Set the current working directory to root
    // cwd = strdup("/");  // Ensure cwd is dynamically allocated
    // if (cwd == NULL) {
    //     fprintf(stderr, "[LOAD ROOT] Failed to allocate memory for cwd\n");
    //     free(rootDir);
    //     return -1;
    // }
    cwd = malloc(MAX_NAME_LENGTH);
    cwd[0] = '/';
        cwd[1] = '\0';
	currentDir = rootDir;
    printf("[loadRootDirectory] Root directory loaded. Start block: %d\n", startBlock);
    return 0;  // Successful loading of root directory
}

DirectoryEntry* initDirectory(int defaultEntries, DirectoryEntry *dirEntry, DirectoryEntry *parent, char* name) {
        printf("[initDirectory] Initializing directory: Name: '%s', Default Entries: %d\n", name, defaultEntries);
    // Calculate bytes needed based on the number of default entries and their size
    int bytesNeeded = defaultEntries * sizeof(DirectoryEntry);

    // Compute the number of blocks needed for the directory
    int blocksNeeded = (bytesNeeded + 512 - 1) / 512;

    // Allocate memory for the directory entries
    DirectoryEntry* dir = (DirectoryEntry*) malloc(bytesNeeded);
    if (!dir) {
        perror("Failed to allocate memory for directory entries");
        return NULL;  // Return NULL to indicate failure
    }

    // Allocate space for the directory entries on disk using FAT
    uint32_t startBlock = allocateBlocks(blocksNeeded);
    printf("Startblock: %d\n", startBlock);
    if (startBlock == END_OF_FILE) {
        printf("Failed to allocate blocks for directory entries.\n");
        return NULL;  // Return NULL to indicate failure
    }

    // Initialize the current directory (".") and parent directory ("..") entries
    strcpy(dir[0].file_name, ".");
    dir[0].isDirectory = 1;
    dir[0].file_size = bytesNeeded;
    dir[0].ctime = dir[0].mtime = dir[0].atime = time(NULL);
    dir[0].location = startBlock;

    if (!parent) {
        parent = &dir[0];
    }

    // Initialize the parent directory ("..") entry
    strcpy(dir[1].file_name, "..");
    dir[1].file_size = parent->file_size;
    dir[1].isDirectory = 1;
    dir[1].ctime = parent->ctime;
    dir[1].mtime = parent->mtime;
    dir[1].atime = parent->atime;
    dir[1].location = parent->location;

    // Initialize other directory entries
    for (int i = 2; i < defaultEntries; i++) {
    dir[i].file_name[0] = '\0'; // Set the file name to an empty string
    dir[i].file_size = 0;       // Size is 0 for empty files/directories
    dir[i].ctime = dir[i].mtime = dir[i].atime = time(NULL);
    dir[i].location = 0;        // Location will be set later when allocated
}

    // Update location for the directory entries
   // for (int i = 0; i < defaultEntries; i++) {
   //     dir[i].location = startBlock;
   // }

    // Write the directory entries to disk
    uint64_t blocksWritten = LBAwrite((unsigned char*) dir, blocksNeeded, /*vcb->root_directory_start_block*/ startBlock);
    if (blocksWritten != blocksNeeded) {
        printf("Failed to write all directory blocks to disk.\n");
        return NULL;  // Return NULL to indicate failure
    }

    // Return the directory entry array
        printf("[initDirectory] Directory initialized: '%s', Start Block: %u\n", name, startBlock);
    return dir;
}


