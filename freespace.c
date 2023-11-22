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

// Initialize the File Allocation Table
void initFAT(uint32_t numberofBlocks, uint32_t size) {
    // Allocate memory for the FAT table
    fatTable = (uint32_t*)malloc(numberofBlocks * sizeof(uint32_t));
    if (fatTable == NULL) {
        fprintf(stderr, "initFAT: Failed to allocate memory for the FAT table.\n");
        return;
    }

    // Initialize all blocks as free
    for (uint32_t i = 0; i < numberofBlocks; i++) {
        fatTable[i] = FREEBLOCK;
    }

    // The first block (VCB) is always occupied
    fatTable[0] = OCCUPIEDBLOCK;
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

uint32_t findFreeBlock() {
    // Start searching for a free block after the VCB
    for (uint32_t i = 1; i < vcb->table_size; i++) {
        if (fatTable[i] == FREEBLOCK) {
            return i;  // Free block found, return its index.
        }
    }
    printf("Failure to find a free block...\n");
    return END_OF_FILE;  // No free block found
}

// Allocate a new chain or extend an existing one in the FAT
uint32_t allocateBlocks(int numberOfBlocks, uint32_t startBlock) {
    if (numberOfBlocks <= 0) {
        return END_OF_FILE; // Invalid request
    }

    uint32_t currentBlock = startBlock;
    // If extending an existing chain, find the last block of the current chain
    if (currentBlock != END_OF_FILE) {
        while (fatTable[currentBlock] != END_OF_FILE) {
            currentBlock = fatTable[currentBlock];
        }
        if (fatTable[currentBlock] == END_OF_FILE) {
            return END_OF_FILE; // No more blocks to extend
        }
    } else {
        currentBlock = findNextFreeBlock(); // Allocate new chain
        if (currentBlock == END_OF_FILE) return END_OF_FILE;
    }

    int blocksAllocated = (startBlock == END_OF_FILE) ? 0 : 1;
    while (blocksAllocated < numberOfBlocks && currentBlock != END_OF_FILE) {
        uint32_t nextBlock = findNextFreeBlock();
        if (nextBlock == END_OF_FILE) break; // No more free blocks
        fatTable[currentBlock] = nextBlock;
        currentBlock = nextBlock;
        blocksAllocated++;
    }

    if (blocksAllocated < numberOfBlocks) {
        // Not enough blocks were allocated
        releaseBlocks(startBlock); // Release partial allocation
        return END_OF_FILE;
    }

    // Mark the end of the chain
    fatTable[currentBlock] = END_OF_FILE;
    FATupdate();
    return startBlock == END_OF_FILE ? currentBlock : startBlock;
}


void releaseBlocks(uint32_t beginBlock) {
    if (beginBlock >= vcb->table_size || beginBlock == 0) {
        return; // Invalid block number or trying to release VCB
    }

    uint32_t currentBlock = beginBlock;
    while (currentBlock != END_OF_FILE) {
        uint32_t nextBlock = fatTable[currentBlock];
        fatTable[currentBlock] = FREEBLOCK;
        currentBlock = nextBlock;
    }
    FATupdate();
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

void releaseSingleBlock(uint32_t blockNum) {
    if (blockNum < vcb->table_size) {
        fatTable[blockNum] = FREEBLOCK; // Mark the block as free.
        FATupdate(); // Update the FAT table on the disk.
    }
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

void releaseBlockChain(uint32_t startBlock) {
    uint32_t currentBlock = startBlock;
    uint32_t nextBlock;

    while (currentBlock != END_OF_FILE && currentBlock < vcb->table_size) {
        nextBlock = fatTable[currentBlock];


        releaseSingleBlock(currentBlock);
        
        currentBlock = nextBlock;
    }
}

// Helper function to calculate FAT size
uint64_t calculateFATSize(uint64_t numberOfBlocks, uint64_t blockSize) {
    uint64_t fatEntries = numberOfBlocks; // One entry per block
    uint64_t fatSizeInBytes = fatEntries * sizeof(uint32_t); // 4 bytes per FAT entry
    return (fatSizeInBytes + blockSize - 1) / blockSize; // Round up to the nearest block
}
//thisi s just to calculate the number of blocks and it will round up. 
int toBlocks(int bytes){
    int blocks = (bytes + vcb->block_size - 1) / vcb->block_size;
    return blocks;
}