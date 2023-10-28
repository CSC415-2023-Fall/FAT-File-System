#ifndef freespace_H
#define freespace_H

#include <stdint.h>
#include <sys/types.h>

// Structure defining a FAT entry
typedef struct FATentry {
    uint8_t status; // Status of the block (ex: 0 for free, 1 for allocated)
    uint32_t next;  // Index of the next block in the file 
} FATentry;

// Structure defining the File Allocation Table (FAT)
typedef struct FileAllocationTable {
    FATentry* entries; // Pointer to an array of FAT entries
    uint32_t size;     // Total number of entries in the FAT
} FileAllocationTable;

// Structure for managing free space within the file system
typedef struct FreeSpace {// (need to add more metadata if needed )
    size_t SIZE;             // Total size of the free space
    uint32_t startingBlock;  // Index of the first free block
    uint32_t freeBlocksCount;// Total number of free blocks available
} FreeSpace;

// Function prototypes for managing free space

// Initialize the free space structure
void initializeFreeSpace(FreeSpace* space, uint32_t size);

// Allocate a block of free space
// Returns the starting block number on success, -1 on failure
uint32_t allocateFreeSpace(FreeSpace* space, size_t size);

// Release a block of space back to the free space in progress 
// void releaseFreeSpace(FreeSpace* space, uint32_t blockNum, size_t size);

// Get the total count of free blocks available
size_t getFreeSpaceCount(const FreeSpace* space);

// Function prototypes for FAT operations

// Initialize the File Allocation Table
void initializeFAT(FileAllocationTable* fatTable, uint32_t size);

// Allocate a block in the FAT
// Returns the allocated block number, or -1 if allocation fails
uint32_t allocateBlock(FileAllocationTable* fatTable);

// Free a block in the FAT in progress
//void freeBlock(FileAllocationTable* fatTable, uint32_t blockNum);

#endif // freespace_H
