/**
Class: CSC-415-03 Fall 2023
Name: Conrad Choi
Student ID: 911679059
GitHub Name: ChoiConrad
Project: File System Project
*
* File: freespace.h
*
* Description: Main file for managing free space in the file system.
**/
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "freespace.h"
#define FREEBLOCK 0
#define OCCUPIEDBLOCK 1

FATEntry *fatTable = NULL; // Declare fatTable as an array of FATEntry structures
size_t fatBlockCount;
extern struct volume_control_block *vcb;


void initFAT(uint32_t numberOfBlocks) {
   // Calculate the number of bytes needed for the FAT table
   size_t fatSize = numberOfBlocks * sizeof(FATEntry);
   // Allocate memory for the FAT table
   printf("initFat: Blocksize = %ld and startblock is %ld\n", vcb->block_size, vcb->start_block);
   fatBlockCount = (fatSize + vcb->block_size - 1) / vcb->block_size;
   fatTable = (FATEntry*)malloc(fatBlockCount * vcb->block_size);
   vcb->table_size = fatBlockCount;
   if(fatTable == NULL) {
       fprintf(stderr, "initFAT: Failed to allocate memory for the FAT table.\n");
       return;
   }
   else {
   printf("initFAT: Memory allocation for FAT table successful\n");
}
   // The first block (VCB) is typically reserved and not part of the linked list
   fatTable[0].status = OCCUPIEDBLOCK;
   fatTable[0].nextBlock = END_OF_FILE;  // End of the list
   // Initialize FAT entries to point to the next block in sequence
   for (uint32_t i = 1; i < numberOfBlocks; i++) {
       fatTable[i].status = FREEBLOCK;
       fatTable[i].nextBlock = (i == numberOfBlocks - 1) ? END_OF_FILE : i + 1;
   }
   // Update FAT table on disk
      printf("initFAT: Completed successfully\n");
          printf("initFAT: Initialization complete. Number of FAT blocks: %lu\n", fatBlockCount);
   FATupdate();
}

// void FATupdate() {

//     printf("FatUpdate: Blocksize = %ld and startblock is %ld\n", vcb->block_size, vcb->start_block);
// // Calculate the number of blocks needed for the FAT
//    //uint64_t fatBlocks = toBlocks(vcb->table_size * sizeof(FATEntry));
//    size_t fatBlockCountReturn = LBAwrite(fatTable, fatBlockCount, vcb->start_block);

//    // Iterate through the FATEntry array and write each entry to the disk
//    //for (uint32_t blockNum = 0; blockNum < fatBlocks; blockNum++) {
//        // Calculate the block address where this FAT entry should be written
//    //    uint64_t blockAddress = vcb->start_block + blockNum;

//        // Write the FAT entry to the disk
//        if (fatBlockCountReturn != fatBlockCount) {
//            fprintf(stderr, "Failed to update FAT %u blocks at block %u on disk.\n", fatBlockCount, vcb->start_block);
//            return;  // Exit early on failure
//        }
//    //}
//    printf("FAT successfully updated on disk.\n");
// }

void FATupdate() {
    printf("FATupdate: Blocksize = %lu and startblock is %lu\n", vcb->block_size, vcb->start_block);
    size_t fatBlockCountReturn = LBAwrite(fatTable, fatBlockCount, vcb->start_block);

    if (fatBlockCountReturn != fatBlockCount) {
        fprintf(stderr, "Failed to update FAT %lu blocks at block %lu on disk.\n", fatBlockCount, vcb->start_block);
    } else {
        printf("FAT successfully updated on disk. Written %lu blocks.\n", fatBlockCountReturn);
    }
}


uint32_t allocateBlocks(int numberOfBlocks) {
   if (numberOfBlocks <= 0) {
       return END_OF_FILE;
   }

   uint32_t firstAllocatedBlock = findNextFreeBlock();
   if (firstAllocatedBlock == END_OF_FILE) {
       return END_OF_FILE; // No free block to start the chain
   }

   uint32_t currentBlock = firstAllocatedBlock;
   int blocksAllocated = 1; // Start with the current block already allocated

   while (blocksAllocated < numberOfBlocks) {
       uint32_t nextBlock = findNextFreeBlock();
       if (nextBlock == END_OF_FILE) {
           releaseMultipleBlocks(firstAllocatedBlock); // Release partially allocated blocks
           return END_OF_FILE;
       }



       // Update the FAT entry for the current block
       fatTable[currentBlock].status = OCCUPIEDBLOCK;
       fatTable[currentBlock].nextBlock = nextBlock;
       currentBlock = nextBlock;
       blocksAllocated++;
   }
   // Mark the last allocated block as the end of the chain
   fatTable[currentBlock].status = OCCUPIEDBLOCK;
   fatTable[currentBlock].nextBlock = END_OF_FILE;
   FATupdate(); // Update the FAT table on the disk
   return firstAllocatedBlock; // Return the first block of the newly allocated chain
}



uint32_t findNextFreeBlock() {
    if (fatTable == NULL) {
        fprintf(stderr, "findNextFreeBlock: fatTable is not initialized.\n");
        return END_OF_FILE;
    }

    if (vcb == NULL || vcb->table_size == 0) {
        fprintf(stderr, "findNextFreeBlock: vcb is not initialized or table_size is invalid.\n");
        return END_OF_FILE;
    }

    // Add diagnostic print
    printf("findNextFreeBlock: Checking FAT entries... (vcb->table_size: %lu)\n", vcb->table_size);
    for (uint32_t i = 0; i < vcb->table_size && i < 10; i++) { // print first 10 entries for check
        printf("FAT Entry %u: status = %d, nextBlock = %u\n", i, fatTable[i].status, fatTable[i].nextBlock);
    }

    for (uint32_t i = 402; i < 19531; i++) {
        if (fatTable[i].status == FREEBLOCK) {
            return i; // Found a free block.
        }
    }
    return END_OF_FILE; // No free block found.
}




uint32_t allocateSingleBlock() {
   uint32_t freeBlock = findNextFreeBlock();
   if (freeBlock != END_OF_FILE) {
       fatTable[freeBlock].status = OCCUPIEDBLOCK; // Mark the block as occupied.
       FATupdate(); // Update the FAT table on the disk.
   }
   return freeBlock;
}

void releaseSingleBlock(uint32_t blockNum) {
   if (blockNum < vcb->table_size) {
       fatTable[blockNum].status = FREEBLOCK; // Mark the block as free.
       FATupdate(); // Update the FAT table on the disk.
   }
}

void releaseMultipleBlocks(uint32_t startBlock) {
   uint32_t currentBlock = startBlock;
   uint32_t nextBlock;
   while (currentBlock != END_OF_FILE && currentBlock < vcb->table_size) {
       nextBlock = fatTable[currentBlock].nextBlock;
       fatTable[currentBlock].status = FREEBLOCK;
       currentBlock = nextBlock;
   }
   FATupdate();
}

int toBlocks(int bytes) {
   int blocks = (bytes + vcb->block_size - 1) / vcb->block_size;
   return blocks;
}