/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
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

// Initialize the file system with the given number of blocks and block size
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize) {
    // Display initialization status
    printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);

    // Allocate memory for the volume control block
    struct volume_control_block* vcb = (struct volume_control_block*) malloc(blockSize);

    // Check for memory allocation failure
    if (!vcb) {
        perror("malloc");
        return -1;
    }

    // Read the first block to check if the volume is already initialized
    LBAread(vcb, 1, 0);

    // Check if volume is already initialized by verifying the magic number
    if (vcb->magicNumber == Unique_ID) {
        // Release memory since file system is already initialized
        free(vcb);
        return 0;
    }

    // Initialize the Volume Control Block (VCB) properties
    vcb->magicNumber = Unique_ID;
    strcpy(vcb->volume_name, "MyVolume");
    strcpy(vcb->fileSystemType, "FAT32");
    vcb->block_size = blockSize;
    vcb->start_block = 1;

    // Calculate the cluster count and FAT table size
    uint64_t cluster_count = numberOfBlocks / CLUSTER_SIZE_IN_BLOCKS;
    uint64_t fat_table_size_in_bytes = cluster_count * 4;
    vcb->table_size = fat_table_size_in_bytes / blockSize;
    if (fat_table_size_in_bytes % blockSize != 0) {
        vcb->table_size += 1;
    }

    // Set starting points for root directory and free blocks
    vcb->root_directory_start_block = vcb->start_block + vcb->table_size;
    vcb->first_free_block = vcb->root_directory_start_block + 32;
    vcb->free_block_count = numberOfBlocks - (1 + vcb->table_size + 32);
    vcb->last_allocated_block = vcb->first_free_block - 1;

    // Initialize the free space for the file system
    FreeSpace freeSpace;
    initializeFreeSpace(&freeSpace, numberOfBlocks - vcb->root_directory_start_block);

    // Initialize the File Allocation Table (FAT)
    FileAllocationTable fatTable;
    initializeFAT(&fatTable, numberOfBlocks);

    // Initialize the root directory
    int defaultEntries = DEFAULT_ENTRIES; // Assuming you've defined DEFAULT_ENTRIES somewhere
    initRootDirectory(defaultEntries, blockSize, &freeSpace, &fatTable, vcb);

    // Write the initialized VCB to the start of the file system
    LBAwrite(vcb, 1, 0);

    // Release allocated memory for VCB
    free(vcb);

    return 0;
}

// Function to handle the file system exit
void exitFileSystem() {
    // Display exit status
    printf("System exiting\n");
}
