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
#include "freespace.h"
#include <stdlib.h> 
#include <string.h> 

// Function to initialize the Free Space structure
void initializeFreeSpace(FreeSpace* space, uint32_t size) {
    if (space != NULL) {
        space->SIZE = size;                // Set the total size of free space

        space->startingBlock = 0;          // Initialize the starting block to 0
        
        space->freeBlocksCount = size;     // Set the initial free blocks count to the total size
    }
}

// Function to allocate free space
// Returns the starting block number of the allocated space or -1 if allocation fails
 uint32_t allocateFreeSpace(FreeSpace* space, FileAllocationTable* fatTable, size_t size, struct volume_control_block *vcb) {
    if (space == NULL || fatTable == NULL || vcb == NULL || size == 0) {
        printf("Invalid parameters\n");
        return (uint32_t)-1; // Invalid parameters
    }

    if (space->freeBlocksCount < size) {
        printf("Not enough free space\n");
        return (uint32_t)-1; // Not enough free space
    }

    uint32_t blockCount = 0;
    uint32_t firstBlock = (uint32_t)-1;
    uint32_t lastBlock = (uint32_t)-1;

    for (uint32_t i = 0; i < fatTable->size && blockCount < size; ++i) {
        if (fatTable->entries[i].status == 0) { // Block is free
            if (firstBlock == (uint32_t)-1) { // checking for an error. SideNote: the (uint32_t) is nessisary because -1 can normally not be inserted into uint32_t variable 
            //by keeping the uint32_t changes the -1 variable to max value uint32_t can hold 
                firstBlock = i;
            }

            if (lastBlock != (uint32_t)-1) {
                fatTable->entries[lastBlock].next = i;
            }

            lastBlock = i;
            blockCount++;

            fatTable->entries[i].status = 1; // Mark as allocated
            fatTable->entries[i].next = (uint32_t)-1; // Currently, this is the last block
        }
    }

    if (blockCount == size) {
        space->freeBlocksCount -= size; // Update free space count

        // Update VCB
        vcb->free_block_count -= size; //updating the block count for the vcb
        vcb->last_allocated_block = lastBlock; // updating the last block for the vcb 
        if (firstBlock == vcb->first_free_block) { 
            vcb->first_free_block = (lastBlock + 1) % fatTable->size;// sets the first_free_block in VCB and point to the next block 

        }

        return firstBlock;
    } else {
        //in case a failure of allocation. 
        for (uint32_t i = firstBlock; i != (uint32_t)-1 && i < fatTable->size; i = fatTable->entries[i].next) {
            fatTable->entries[i].status = 0;
        }// the purpose of this is to go back and reset the blocks used back to 0 available for use. 
        return (uint32_t)-1; // Allocation failed
    }
}



// Function to get the count of free space blocks
// Returns the number of free blocks available
size_t getFreeSpaceCount(const FreeSpace* space) {
    if (space != NULL) {
        return space->freeBlocksCount;    // Return the count of free blocks
    } else {
        return 0;                         // Return 0 if space is NULL
    }
}

// Function to initialize the File Allocation Table (FAT)
void initializeFAT(FileAllocationTable* fatTable, uint32_t size) {
    if (fatTable != NULL && size > 0) {  // Check if fatTable is not NULL and size is greater than 0
        fatTable->entries = (FATentry*)malloc(size * sizeof(FATentry));
        if (fatTable->entries != NULL) {
            memset(fatTable->entries, 0, size * sizeof(FATentry)); // Initialize memory to zero
        }
        fatTable->size = size;  // Set the size of the FAT
    }
}

// Function to allocate a block in the FAT
// Returns the block number on successful allocation or -1 if allocation fails
//doesn't contain any error checks 
uint32_t allocateBlock(FileAllocationTable* fatTable) {
    if (fatTable == NULL) {
        return (uint32_t)-1; // Return -1 to indicate failure
    }

    for (uint32_t i = 0; i < fatTable->size; i++) {
        if (fatTable->entries[i].status == 0) { // Check if the block is free

            fatTable->entries[i].status = 1;    // Mark the block as allocated
        
            return i; // Return the block number
        }
    }

    return (uint32_t)-1; // Return -1 if no free blocks are available
}

// Function voidfreeBlock(FAT* fattable, uint32_t blocknumber )
