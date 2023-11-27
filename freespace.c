// #include <stdlib.h>
// #include <string.h>
// #include <sys/types.h>
// #include "freespace.h"
// #include "fsInit.h"


// #define FREEBLOCK 0x00000000
// #define OCCUPIEDBLOCK 0xFFFFFF7F
// #define END_OF_FILE 0xFFFFFFFE






// uint32_t *fatTable = NULL;
// extern struct volume_control_block *vcb;






// // void initFAT(uint32_t numberOfBlocks) {
// //     // Allocate memory for the FAT table
// //     fatTable = (uint32_t*)malloc(numberOfBlocks * sizeof(uint32_t));
// //     if (fatTable == NULL) {
// //         fprintf(stderr, "initFAT: Failed to allocate memory for the FAT table.\n");
// //         return;
// //     }


// //     // The first block (VCB) is typically reserved and not part of the linked list
// //     fatTable[0] = OCCUPIEDBLOCK;


// //     // Initialize FAT entries to point to the next block in sequence
// //     for (uint32_t i = 1; i < numberOfBlocks; i++) {
// //         fatTable[i] =  i+1;
      
// //         if (i == numberOfBlocks - 1) {
// //             // Mark the last block with a special value indicating the end of the list
// //             fatTable[i] = END_OF_FILE;
// //         }
  
// //     }


// //     // Update FAT table on disk
// //     FATupdate();
// // }


// void initFAT(uint32_t numberOfBlocks) {
//     // Allocate memory for the FAT table
//     fatTable = (uint32_t*)malloc(numberOfBlocks * sizeof(uint32_t));
//     if (fatTable == NULL) {
//         fprintf(stderr, "initFAT: Failed to allocate memory for the FAT table.\n");
//         return;
//     }


//     // Initially mark all blocks as FREEBLOCK
//     for (uint32_t i = 0; i < numberOfBlocks; i++) {
//         fatTable[i] = FREEBLOCK;
//     }


//     // The first block (VCB) is typically reserved and not part of the free list
//     fatTable[0] = OCCUPIEDBLOCK;


//     // Initialize FAT entries to point to the next block in sequence for the first 400 blocks
//     uint32_t listSize = 400; // Or whatever your list size should be
//     for (uint32_t i = 1; i < listSize; i++) {
//         fatTable[i] = i + 1;
//     }


//     // Mark the last block of the initial list with a special value indicating the end of the list
//     if (listSize < numberOfBlocks) {
//         fatTable[listSize] = END_OF_FILE;
//     }


//     // Update FAT table on disk
//     FATupdate();
// }












// void FATupdate() {
//     // Calculate the number of blocks needed for the FAT
//     uint64_t fatBlocks = toBlocks(vcb->table_size * sizeof(uint32_t));


//     // Write the FAT table to the disk from memory
//     if (LBAwrite(fatTable, fatBlocks, vcb->start_block) != fatBlocks) {
//         fprintf(stderr, "Failed to update FAT on disk.\n");
//     } else {
//         printf("FAT successfully updated on disk.\n");
//     }
// }


// // uint32_t findFreeBlock() {
// //     // Start searching for a free block after the VCB
// //     for (uint32_t i = 1; i < vcb->table_size; i++) {
// //         if (fatTable[i] == FREEBLOCK) {
// //             return i;  // Free block found, return its index.
// //         }
// //     }
// //     printf("Failure to find a free block...\n");
// //     return END_OF_FILE;  // No free block found
// // }
// // Allocate a new chain or extend an existing one in the FAT


// // uint32_t allocateBlocks(int numberOfBlocks, uint32_t startBlock) {
// //     if (numberOfBlocks <= 0) {
// //         return END_OF_FILE; // Invalid request
// //     }


// //     uint32_t currentBlock = startBlock;
// //     uint32_t previousBlock = END_OF_FILE;


// //     // If extending an existing chain, find the last block of the current chain
// //     if (currentBlock != END_OF_FILE) {
// //         while (currentBlock != END_OF_FILE) {
// //             previousBlock = currentBlock;
// //             currentBlock = fatTable[currentBlock];
// //         }
// //     }


// //     int blocksAllocated = (startBlock == END_OF_FILE) ? 0 : 1;


// //     while (blocksAllocated < numberOfBlocks) {
// //         uint32_t nextBlock = findNextFreeBlock();


// //         if (nextBlock == END_OF_FILE) {
// //             // No more free blocks
// //             if (startBlock == END_OF_FILE) {
// //                 return END_OF_FILE; // Failed to allocate any blocks
// //             } else {
// //                 // Partial allocation, release allocated blocks and return END_OF_FILE
// //                 releaseMultipleBlocks(startBlock);
// //                 return END_OF_FILE;
// //             }
// //         }


// //         // If extending an existing chain, update the previous block's reference
// //         if (currentBlock != END_OF_FILE) {
// //             fatTable[currentBlock] = nextBlock;
// //         } else {
// //             // If it's a new chain, update the startBlock
// //             startBlock = nextBlock;
// //         }


// //         currentBlock = nextBlock;
// //         blocksAllocated++;
// //     }


// //     // Mark the end of the chain
// //     fatTable[currentBlock] = END_OF_FILE;
// //     FATupdate();


// //     if (startBlock == END_OF_FILE) {
// //         // If a new chain was created, return the starting block
// //         return currentBlock;
// //     } else {
// //         // If extending an existing chain, return the original starting block
// //         return startBlock;
// //     }
// // }


// uint32_t allocateBlocks(int numberOfBlocks) {


//     if (numberOfBlocks <= 0) {


//         return END_OF_FILE;


//     }
//     printf("After first check");


//     uint32_t firstAllocatedBlock = findNextFreeBlock();
//     printf("After find freeblock");
//     printf("First allocated block: %d", firstAllocatedBlock);
//     if (firstAllocatedBlock == END_OF_FILE) {


//         return END_OF_FILE; // No free block to start the chain


//     }


//     printf("After second check");




//     uint32_t currentBlock = firstAllocatedBlock;


//     int blocksAllocated = 1; // Start with the current block already allocated






//     while (blocksAllocated < numberOfBlocks) {


//         uint32_t nextBlock = findNextFreeBlock();


//         if (nextBlock == END_OF_FILE) {


//             releaseMultipleBlocks(firstAllocatedBlock); // Release partially allocated blocks


//             return END_OF_FILE;


//         }






//         fatTable[currentBlock] = nextBlock; // Link current block to next block


//         currentBlock = nextBlock;


//         blocksAllocated++;


//     }


//     printf("After third check");




//     fatTable[currentBlock] = END_OF_FILE; // Mark the end of the chain


//     FATupdate(); // Update the FAT table on the disk






//     return firstAllocatedBlock; // Return the first block of the newly allocated chain


// }




// uint32_t findNextFreeBlock() {
//     for (uint32_t i = 0; i < 19531; i++) {
//         if (fatTable[i] == FREEBLOCK) {
//             printf("Free block is %d", i);
//             return i; // Found a free block.
//         }
//     }
//     return END_OF_FILE; // No free block found.
// }


// uint32_t allocateSingleBlock() {
//     uint32_t freeBlock = findNextFreeBlock();
//     if (freeBlock != END_OF_FILE) {
//         fatTable[freeBlock] = OCCUPIEDBLOCK; // Mark the block as occupied.
//         FATupdate(); // Update the FAT table on the disk.
//     }
//     return freeBlock;
// }






// void appendBlocksToChain(uint32_t startBlock, int numberOfBlocks) {
//     uint32_t currentBlock = startBlock;


//     while (fatTable[currentBlock] != END_OF_FILE) {


//         currentBlock = fatTable[currentBlock];
//     }


//     for (int i = 0; i < numberOfBlocks; i++) {
//         uint32_t newBlock = allocateSingleBlock();




//         if (newBlock == END_OF_FILE) {
//             break; // No more blocks available.
//         }




//         fatTable[currentBlock] = newBlock;


//         currentBlock = newBlock;
//     }






//     fatTable[currentBlock] = END_OF_FILE; // Mark the end of the chain.
  
  
//     FATupdate(); // Update the FAT table on the disk.
// }


// void releaseSingleBlock(uint32_t blockNum) { // this one releases a single block


//     if (blockNum < vcb->table_size) {
//         fatTable[blockNum] = FREEBLOCK; // Mark the block as free.
//         FATupdate(); // Update the FAT table on the disk.
//     }
// }


// void releaseMultipleBlocks(uint32_t startBlock) { // this frees blocks function where it  takes the specified block number and releases
//     uint32_t currentBlock = startBlock;
//     uint32_t nextBlock;


//     while (currentBlock != END_OF_FILE && currentBlock < vcb->table_size) {
//         nextBlock = fatTable[currentBlock];
//         fatTable[currentBlock]=FREEBLOCK;


      
//         currentBlock = nextBlock;
//     }
//     FATupdate();
// }


// // Helper function to calculate FAT size
// // uint64_t calculateFATSize(uint64_t numberOfBlocks, uint64_t blockSize) {
// //     uint64_t fatEntries = numberOfBlocks; // One entry per block
// //     uint64_t fatSizeInBytes = fatEntries * sizeof(uint32_t); // 4 bytes per FAT entry
// //     return (fatSizeInBytes + blockSize - 1) / blockSize; // Round up to the nearest block
// // }
// //thisi s just to calculate the number of blocks and it will round up.
// int toBlocks(int bytes){
//     int blocks = (bytes + vcb->block_size - 1) / vcb->block_size;
//     return blocks;
// }


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
   if (fatTable == NULL) {
       fprintf(stderr, "initFAT: Failed to allocate memory for the FAT table.\n");
       return;
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
   FATupdate();
}

void FATupdate() {

    printf("FatUpdate: Blocksize = %ld and startblock is %ld\n", vcb->block_size, vcb->start_block);
// Calculate the number of blocks needed for the FAT
   //uint64_t fatBlocks = toBlocks(vcb->table_size * sizeof(FATEntry));
   size_t fatBlockCountReturn = LBAwrite(fatTable, fatBlockCount, vcb->start_block);

   // Iterate through the FATEntry array and write each entry to the disk
   //for (uint32_t blockNum = 0; blockNum < fatBlocks; blockNum++) {
       // Calculate the block address where this FAT entry should be written
   //    uint64_t blockAddress = vcb->start_block + blockNum;

       // Write the FAT entry to the disk
       if (fatBlockCountReturn != fatBlockCount) {
           fprintf(stderr, "Failed to update FAT %u blocks at block %u on disk.\n", fatBlockCount, vcb->start_block);
           return;  // Exit early on failure
       }
   //}
   printf("FAT successfully updated on disk.\n");
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

   for (uint32_t i = 0; i < vcb->table_size; i++) {
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