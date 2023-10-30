/**************************************************************
* Class:  CSC-415-03 Fall 2021
* Names:Amandeep Singh
* Student IDs:921287533
* GitHub Name:Amandeep-Singh-24
* Group Name:Tryhards
* Project: Basic File System
*
* File: fsInit.h
*
* Description: Header for the main driver of the file system assignment.
*
**************************************************************/

#ifndef FSINIT_H
#define FSINIT_H

// Commonly used standard libraries for file operations, types, and string manipulations
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Include low-level file system functions and main file system header
#include "fsLow.h"
#include "mfs.h"

// Define constants for name length, file system type, and the unique identifier
#define MAX_NAME_LENGTH 256
#define FILE_SYSTEM_TYPE_LENGTH 32
#define Unique_ID 0xAA55
#define CLUSTER_SIZE_IN_BLOCKS 8

// Structure representing the Volume Control Block (VCB) of the file system
struct volume_control_block {
    uint64_t magicNumber;
    char volume_name[MAX_NAME_LENGTH];
    char fileSystemType[FILE_SYSTEM_TYPE_LENGTH];
    uint64_t block_size;
    uint64_t start_block;
    uint64_t table_size;
    uint64_t root_directory_start_block;
    uint64_t first_free_block;
    uint64_t last_allocated_block;
    uint64_t free_block_count;
};

// Include headers for free space management and directory operations
#include "freespace.h"
#include "directory.h"

// Expose functions for initializing and exiting the file system
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize);
void exitFileSystem();

#endif // FSINIT_H
