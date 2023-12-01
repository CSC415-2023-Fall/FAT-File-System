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
#include "mfs.h"

struct volume_control_block *vcb = NULL; // Global definition
extern DirectoryEntry *rootDir; 
extern DirectoryEntry *currentDir;
extern char *cwd;

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize) {
    printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);

    // Allocate space for VCB
    vcb = (struct volume_control_block*) malloc(blockSize);
    if (!vcb) {
        perror("malloc");
        return -1;
    }

    // Attempt to read existing VCB from disk
    LBAread(vcb, 1, 0);

    // Check if VCB has been initialized before
    if (vcb->magicNumber == Unique_ID) {
        // VCB already initialized, proceed to load root directory
        printf("initFileSystem: VCB already initialized, loading root directory\n");
        loadRootDirectory();
        return 0; // Return here to skip reinitialization
    }

    // VCB not initialized, proceed with initialization
    vcb->magicNumber = Unique_ID;
    strcpy(vcb->volume_name, "MyVolume");
    strcpy(vcb->fileSystemType, "FAT32");
    vcb->block_size = blockSize;
    vcb->start_block = 1;
    vcb->table_size = 400;

    vcb->root_directory_start_block = vcb->start_block + vcb->table_size + 1;
    vcb->last_allocated_block = vcb->root_directory_start_block - 1;
    vcb->free_block_count = numberOfBlocks - (vcb->root_directory_start_block + 10); 
    vcb->first_free_block = vcb->root_directory_start_block + 11; 

    initFAT(numberOfBlocks); // Initialize the FAT table

    LBAwrite(vcb, 1, 0); // Save the initialized VCB to the file system

    int defaultEntries = DEFAULT_ENTRIES; 
    char* name = "DirEntry";
    DirectoryEntry dirEntry;
    DirectoryEntry* parent = NULL;
    
    initDirectory(defaultEntries, &dirEntry, parent, name);
    loadRootDirectory();
    // Do not free vcb here as it is needed throughout the file system operation
    return 0;
}

void exitFileSystem() {
    printf("System exiting\n");
    free(vcb); // Free VCB during system shutdown
    vcb = NULL;
}
