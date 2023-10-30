/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: fsInit.h
*
* Description: Header for the main driver of the file system assignment.
*
**************************************************************/

#ifndef FSINIT_H
#define FSINIT_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "fsLow.h"
#include "mfs.h"

#define MAX_NAME_LENGTH 256
#define FILE_SYSTEM_TYPE_LENGTH 32
#define Unique_ID 0xAA55
#define CLUSTER_SIZE_IN_BLOCKS 8

struct volume_control_block {
    uint64_t magicNumber;
    char volume_name[MAX_NAME_LENGTH];
    char fileSystemType[FILE_SYSTEM_TYPE_LENGTH];
    uint64_t block_size;
    uint64_t start_block;
    uint64_t table_size;
    uint64_t first_free_block;
    uint64_t free_block_count;
    uint64_t last_allocated_block;
    uint64_t root_directory_start_block;
};

#include "freespace.h"
#include "directory.h"

// Function prototypes
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize);
void exitFileSystem();

#endif // FSINIT_H
