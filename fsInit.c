/**************************************************************
* Class:  CSC-415-03 Fall 2021
* Names: Amandeep Singh 
* Student IDs: 921287533
* GitHub Name: Amandeep-Singh-24
* Group Name: Tryhards
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
struct volume_control_block *vcb = NULL; // Global definition


// Begin the file system initialization process
     int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize) {
  
    
    // Inform user of the initialization process
    printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);

    // Create a new volume control block
    vcb = (struct volume_control_block*) malloc(blockSize);

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
    vcb->table_size = 400;


    // Define where the root directory and free blocks start
    vcb->root_directory_start_block = vcb->start_block + vcb->table_size + 1;
    vcb->last_allocated_block = vcb->root_directory_start_block - 1;
    vcb->free_block_count = numberOfBlocks - (vcb->root_directory_start_block + 10); // 10 is the value of blocksNeeded
    vcb->first_free_block = vcb->root_directory_start_block + 11; // Add 1 to 10 to get the next free block


    // Initialize the FAT table
    initFAT(numberOfBlocks); // Adjusted call

    FATupdate(); // updating the fat table onto disk 

    // Save the initialized VCB to the file system
    LBAwrite(vcb, 1, 0);
    // Set up the root directory with default values
    int defaultEntries = DEFAULT_ENTRIES; 
    char* name = "DirEntry";
    struct DirectoryEntry dirEntry;
    struct DirectoryEntry* parent = NULL;
    initDirectory(defaultEntries, &dirEntry, parent, name);

    // Clean up allocated memory
    free(vcb);

    return 0;
}
    
// Handle the system's exit routine
void exitFileSystem() {
    
    // Inform user of system shutdown
    printf("System exiting\n");
}