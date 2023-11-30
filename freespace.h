/**
Class: CSC-415-03 Fall 2023
Name: Conrad Choi
Student ID: 911679059
GitHub Name: ChoiConrad
Project: File System Project
*
* File: freespace.h
*
* Description: Header file for managing free space in the file system.
**/
#ifndef FREESPACE_H
#define FREESPACE_H
#include <stdint.h>
#include <sys/types.h>
#include "fsInit.h"


#define BOOTINGBLOCK 0
#define END_OF_FILE -1

typedef struct {
   uint8_t status;
   uint32_t nextBlock;
}FATEntry;

// Function prototypes for FAT operations
// Initialize the File Allocation Table
void initFAT(uint32_t numberofBlocks);
// Update the FAT on the disk
void FATupdate();
// Allocate blocks in the FAT
uint32_t allocateBlocks(int numberOfBlocks);
// release a single block
void releaseSingleBlock(uint32_t blockNum);
// Release blocks from the FAT
void releaseMultipleBlocks(uint32_t startBlock);
// Find the next free block in the FAT
uint32_t findNextFreeBlock();
// Release a single block in the FAT
void freeBlock(uint32_t blockNum);
// Calculate the number of blocks for a given size in bytes
int toBlocks(int bytes);
#endif // FREESPACE_H