/**
Class:  CSC-415-03 Fall 2023
Name:Conrad Choi
Student ID:911679059
GitHub Name:ChoiConrad 
Project: File System Project
*
* File: freespace.h
*
* Description: This file is the h file for freespace this doesn't contain all of the functions that will be added 
* and some are in process.  
*
**/
#ifndef MY_EOF
#define MY_EOF 0xFFFFFFFF
#endif
#ifndef freespace_H
#define freespace_H

#include <stdint.h>
#include <sys/types.h>
#include <fsInit.h>


#define BOOTINGBLOCK 0
#define STARTLOCATION 1
#define OCCUPIEDBLOCK 0xFFFFFF7F
#define FREEBLOCK 0x00000000

// Function prototypes for FAT operations

// Initialize the File Allocation Table
void initFAT(uint32_t numberofBlocks, uint32_t size);

void FATupdate();//updates the fat on the disk x

int readFAT();// this is to take the fat from the disk onto the memory 

uint32_t allocateBlocks(int numberofBlocks);//this will make new blocks in the fat 

uint32_t releaseBlocks(uint32_t beginBlock); // this will release the blocks from the fat 




void additionalBlocks (uint32_t beginBlock, int blockstoAllocate); // allocates more blocks 



uint32_t getNextBlock(int currentBlock); // get next block from FAT 



uint32_t findFree(); // this will find the first free block it encounters. 

int isFree(uint32_t block); // given the block to see if it is free or not


uint32_t totalFreeBlock();// returns the total free blocks 

//calculates blocks from the bytes 
int toBlocks(int bytes);

uint32_t findFreeBlock(); //find the next free block 


// Free a block in the FAT in progress
void freeBlock(uint32_t blockNum);

#endif // freespace_H
