/**
Class:  CSC-415-03 Fall 2023
Name:Conrad Choi
Student ID:911679059
GitHub Name:ChoiConrad 
Project: File System Project
*
* File: freespace.c
*
* Description: This file contains the free space initialization and allocation functions for the freespace and the FAT. 
* Currently, the delete and the removal of blocks is in the process. 
*
**/
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h>
#include "freespace.h"
#include "fsInit.h"

uint32_t *fatTable = NULL;
extern struct volume_control_block *vcb;


// Initialize the File Allocation Table
void initFAT(uint32_t numberofBlocks, uint32_t size) {
    // Allocate memory for the FAT table
    fatTable = (uint32_t*)malloc(numberofBlocks * sizeof(uint32_t));
    if (fatTable == NULL) {
        fprintf(stderr, " init FAT: Failed to allocate memory for the FAT table.\n");
        return;
    }

    // Initialize the FAT table entries
    for (uint32_t i = 0; i < numberofBlocks; i++) {
        fatTable[i] = FREEBLOCK; // Initialize all blocks as free
    }

    // Mark the boot block as occupied
    fatTable[BOOTINGBLOCK] = OCCUPIEDBLOCK;

    // Update the FAT table on disk
    FATupdate();
}

uint32_t findFreeBlock() {
    if (fatTable == NULL) {
        // The FAT is not initialized.
        fprintf(stderr, "Error: FAT is not initialized.\n");
        return MY_EOF;  // incase if it fails 
    }

    for (uint32_t i = 0; i < vcb->table_size; i++) {
        if (fatTable[i] == FREEBLOCK) {
            return i;  // Free block found, return its index.
        }
    }
    printf("failure to find a free block...");
    return MY_EOF;  // No free block found, return end of the file 
}

void FATupdate() {
    // Calculate the number of blocks needed for the FAT
    uint64_t fatBlocks = toBlocks(vcb->table_size * sizeof(uint32_t));

    // Write the FAT table to the disk from memory
    if (LBAwrite(fatTable, fatBlocks, vcb->start_block) != fatBlocks) { // if fails to write 
        fprintf(stderr, "Failed to update FAT on disk.\n");
    } else {
        printf("FAT successfully updated on disk.\n");
    }
}

void freeBlock(uint32_t blockNum) {
    if (blockNum < toBlocks(vcb->table_size * sizeof(uint32_t))) {
        fatTable[blockNum] = FREEBLOCK; // Mark it as free
        FATupdate(); // Save updated FAT 
    }
}

int readFAT() {
    // Calculate the number of blocks needed for the FAT
    uint64_t fatBlocks = toBlocks(vcb->table_size * sizeof(uint32_t));

    // Read the FAT table from disk into memory
    if (LBAread(fatTable, fatBlocks, vcb->start_block) != fatBlocks) {
        fprintf(stderr, "Failed to read FAT from disk into memory.\n");
        return -1;
    }
    printf("FAT successfully read into memory.\n");
    return 0;
}

// Function to allocate a number of blocks in the FAT
uint32_t allocateBlocks(int numberofBlocks) {
    if (numberofBlocks <= 0) {
        return MY_EOF; // Invalid request
    }

    uint32_t startBlock = MY_EOF;
    int allocatedBlocks = 0;

    for (uint32_t i = 0; i < vcb->table_size && allocatedBlocks < numberofBlocks; i++) {
        if (fatTable[i] == FREEBLOCK) {
            if (startBlock == MY_EOF) {
                startBlock = i; // Found the start block for allocation
            }
            allocatedBlocks++;
        } else {
            // Reset if a non-free block is encountered to ensure contiguity
            allocatedBlocks = 0;
            startBlock = MY_EOF;
        }
    }

    if (allocatedBlocks == numberofBlocks) {
        // Mark all found blocks as occupied
        for (int i = 0; i < numberofBlocks; i++) {
            
            fatTable[startBlock + i] = OCCUPIEDBLOCK;

        }
        FATupdate(); // Update the FAT table on disk
        return startBlock; // Return the first block of the allocated sequence
    }

    return MY_EOF; // Not enough contiguous blocks available
}

uint32_t releaseBlocks(uint32_t beginBlock) {
    if (beginBlock >= vcb->table_size) {
        return MY_EOF; // Invalid block number
    }

    uint32_t currentBlock = beginBlock;// takes the begining block to current 

    while (currentBlock != MY_EOF && fatTable[currentBlock] != FREEBLOCK) {// if the current block isnt end of file and current block on fattable isn't free block
        uint32_t nextBlock = fatTable[currentBlock]; 
        fatTable[currentBlock] = FREEBLOCK; // Mark as free
        currentBlock = nextBlock; // Move to next block
        vcb->free_block_count++; // Increment free block count in VCB
    }

    // Update the FAT table on disk
    FATupdate();

    return 0; // Successfully released the blocks
}

// Function to check if a block is free
int isFree(uint32_t block) {
    if (block < 0 || block >= vcb->table_size) {
        return 0; // Invalid block number
    }

    return fatTable[block] == FREEBLOCK;


}

// Function to calculate the total number of free blocks
uint32_t totalFreeBlock() {

    uint32_t freeBlocks = 0; // creates a freeblock variable 

    for (uint32_t i = 0; i < vcb->table_size; i++) {//increment throguh the fattable 
        if (fatTable[i] == FREEBLOCK) {


            freeBlocks++; // increments a counter 
            //printf("test");
        }
    }
    return freeBlocks;
}

// Convert bytes to blocks with respect to the file system's block size
int toBlocks(int bytes) {
    // Utilize the block size from the VCB
    return (bytes + vcb->block_size - 1) / vcb->block_size;
}