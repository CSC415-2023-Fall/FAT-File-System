/**************************************************************
* Class:  CSC-415-03 Fall 2021
* Names:Amandeep Singh 
* Student IDs:921287533
* GitHub Name:Amandeep-Singh-24
* Group Name:Tryhards
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/

#include "fsInit.h"

// Begin the file system initialization process
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize) {
    
    // Inform user of the initialization process
    printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);

    // Create a new volume control block
    struct volume_control_block* vcb = (struct volume_control_block*) malloc(blockSize);

    // Ensure memory was allocated successfully
    if (!vcb) {
        perror("malloc");
        return -1;
    }

    // Check if this volume has been initialized before
    LBAread(vcb, 1, 0);

    // If the volume has been initialized before, skip reinitialization
    if (vcb->magicNumber == Unique_ID) {
        free(vcb);
        return 0;
    }

    // Set the properties for a new volume
    vcb->magicNumber = Unique_ID;
    strcpy(vcb->volume_name, "MyVolume");
    strcpy(vcb->fileSystemType, "FAT32");
    vcb->block_size = blockSize;
    vcb->start_block = 1;

    // Calculate how many clusters exist and determine the size of the FAT table
    uint64_t cluster_count = numberOfBlocks / CLUSTER_SIZE_IN_BLOCKS;
    uint64_t fat_table_size_in_bytes = cluster_count * 4;
    vcb->table_size = fat_table_size_in_bytes / blockSize;
    if (fat_table_size_in_bytes % blockSize != 0) {
        vcb->table_size += 1;
    }

    // Define where the root directory and free blocks start
    vcb->root_directory_start_block = vcb->start_block + vcb->table_size;
    vcb->first_free_block = vcb->root_directory_start_block + 32;
    vcb->free_block_count = numberOfBlocks - (1 + vcb->table_size + 32);
    vcb->last_allocated_block = vcb->first_free_block - 1;

    // Set up the system's free space tracking
    FreeSpace freeSpace;
    initializeFreeSpace(&freeSpace, numberOfBlocks - vcb->root_directory_start_block);

    // Set up the system's file allocation table
    FileAllocationTable fatTable;
    initializeFAT(&fatTable, numberOfBlocks);

    // Set up the root directory with default values
    int defaultEntries = DEFAULT_ENTRIES; 
    char* name = "DirEntry";
    char* type = "DirType";
    struct DirectoryEntry dirEntry;
    struct DirectoryEntry* parent = NULL;
    initRootDirectory(defaultEntries, blockSize, &freeSpace, &fatTable, vcb, name, type, &dirEntry, &parent);

    // Save the initialized VCB to the file system
    LBAwrite(vcb, 1, 0);

    // Clean up allocated memory
    free(vcb);

    return 0;
}

// Handle the system's exit routine
void exitFileSystem() {
    
    // Inform user of system shutdown
    printf("System exiting\n");
}
