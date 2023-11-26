#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h>
#include "freespace.h"
#include "fsInit.h"

#define FREEBLOCK 0x00000000
#define OCCUPIEDBLOCK 0xFFFFFF7F
#define END_OF_FILE 0xFFFFFFFE



uint32_t *fatTable = NULL;
extern struct volume_control_block *vcb;



void initFAT(uint32_t numberOfBlocks) {
    // Allocate memory for the FAT table
    fatTable = (uint32_t*)malloc(numberOfBlocks * sizeof(uint32_t));
    if (fatTable == NULL) {
        fprintf(stderr, "initFAT: Failed to allocate memory for the FAT table.\n");
        return;
    }

    // The first block (VCB) is typically reserved and not part of the linked list
    fatTable[0] = OCCUPIEDBLOCK;

    // Initialize FAT entries to point to the next block in sequence
    for (uint32_t i = 1; i < numberOfBlocks; i++) {
        fatTable[i] =  i+1;
        
        if (i == numberOfBlocks - 1) {
            // Mark the last block with a special value indicating the end of the list
            fatTable[i] = END_OF_FILE;
        }
    
    }

    // Update FAT table on disk
    FATupdate();
}






void FATupdate() {
    // Calculate the number of blocks needed for the FAT
    uint64_t fatBlocks = toBlocks(vcb->table_size * sizeof(uint32_t));

    // Write the FAT table to the disk from memory
    if (LBAwrite(fatTable, fatBlocks, vcb->start_block) != fatBlocks) {
        fprintf(stderr, "Failed to update FAT on disk.\n");
    } else {
        printf("FAT successfully updated on disk.\n");
    }
}

// uint32_t findFreeBlock() {
//     // Start searching for a free block after the VCB
//     for (uint32_t i = 1; i < vcb->table_size; i++) {
//         if (fatTable[i] == FREEBLOCK) {
//             return i;  // Free block found, return its index.
//         }
//     }
//     printf("Failure to find a free block...\n");
//     return END_OF_FILE;  // No free block found
// }
// Allocate a new chain or extend an existing one in the FAT

uint32_t allocateBlocks(int numberOfBlocks, uint32_t startBlock) {
    if (numberOfBlocks <= 0) {
        return END_OF_FILE; // Invalid request
    }

    uint32_t currentBlock = startBlock;
    uint32_t previousBlock = END_OF_FILE;

    // If extending an existing chain, find the last block of the current chain
    if (currentBlock != END_OF_FILE) {
        while (currentBlock != END_OF_FILE) {
            previousBlock = currentBlock;
            currentBlock = fatTable[currentBlock];
        }
    }

    int blocksAllocated = (startBlock == END_OF_FILE) ? 0 : 1;

    while (blocksAllocated < numberOfBlocks) {
        uint32_t nextBlock = findNextFreeBlock();

        if (nextBlock == END_OF_FILE) {
            // No more free blocks
            if (startBlock == END_OF_FILE) {
                return END_OF_FILE; // Failed to allocate any blocks
            } else {
                // Partial allocation, release allocated blocks and return END_OF_FILE
                releaseMultipleBlocks(startBlock);
                return END_OF_FILE;
            }
        }

        // If extending an existing chain, update the previous block's reference
        if (currentBlock != END_OF_FILE) {
            fatTable[currentBlock] = nextBlock;
        } else {
            // If it's a new chain, update the startBlock
            startBlock = nextBlock;
        }

        currentBlock = nextBlock;
        blocksAllocated++;
    }

    // Mark the end of the chain
    fatTable[currentBlock] = END_OF_FILE;
    FATupdate();

    if (startBlock == END_OF_FILE) {
        // If a new chain was created, return the starting block
        return currentBlock;
    } else {
        // If extending an existing chain, return the original starting block
        return startBlock;
    }
}



uint32_t findNextFreeBlock() {
    for (uint32_t i = 0; i < vcb->table_size; i++) {
        if (fatTable[i] == FREEBLOCK) {
            return i; // Found a free block.
        }
    }
    return END_OF_FILE; // No free block found.
}

uint32_t allocateSingleBlock() {
    uint32_t freeBlock = findNextFreeBlock();
    if (freeBlock != END_OF_FILE) {
        fatTable[freeBlock] = OCCUPIEDBLOCK; // Mark the block as occupied.
        FATupdate(); // Update the FAT table on the disk.
    }
    return freeBlock;
}



void appendBlocksToChain(uint32_t startBlock, int numberOfBlocks) {
    uint32_t currentBlock = startBlock;

    while (fatTable[currentBlock] != END_OF_FILE) {

        currentBlock = fatTable[currentBlock];
    }

    for (int i = 0; i < numberOfBlocks; i++) {
        uint32_t newBlock = allocateSingleBlock();


        if (newBlock == END_OF_FILE) {
            break; // No more blocks available.
        }


        fatTable[currentBlock] = newBlock;

        currentBlock = newBlock;
    }



    fatTable[currentBlock] = END_OF_FILE; // Mark the end of the chain.
    
    
    FATupdate(); // Update the FAT table on the disk.
}

void releaseSingleBlock(uint32_t blockNum) { // this one releases a single block 

    if (blockNum < vcb->table_size) {
        fatTable[blockNum] = FREEBLOCK; // Mark the block as free.
        FATupdate(); // Update the FAT table on the disk.
    }
}

void releaseMultipleBlocks(uint32_t startBlock) { // this frees blocks function where it  takes the specified block number and releases 
    uint32_t currentBlock = startBlock;
    uint32_t nextBlock;

    while (currentBlock != END_OF_FILE && currentBlock < vcb->table_size) {
        nextBlock = fatTable[currentBlock];
        fatTable[currentBlock]=FREEBLOCK;

        
        currentBlock = nextBlock;
    }
    FATupdate();
}

// Helper function to calculate FAT size
// uint64_t calculateFATSize(uint64_t numberOfBlocks, uint64_t blockSize) {
//     uint64_t fatEntries = numberOfBlocks; // One entry per block
//     uint64_t fatSizeInBytes = fatEntries * sizeof(uint32_t); // 4 bytes per FAT entry
//     return (fatSizeInBytes + blockSize - 1) / blockSize; // Round up to the nearest block
// }
//thisi s just to calculate the number of blocks and it will round up. 
int toBlocks(int bytes){
    int blocks = (bytes + vcb->block_size - 1) / vcb->block_size;
    return blocks;
}