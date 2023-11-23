/**************************************************************
* Class:  CSC-415-03 Fall 2021
* Names:Akshat Sohal
* Student IDs:917815046
* GitHub Name:sohal786
* Group Name:Tryhards
* Project: Basic File System
*
* File: directory.h
*
* Description: Header for directory file
*
* This file is where you will start and initialize your system
*
**************************************************************/
#ifndef DIRECTORY_H
#define DIRECTORY_H
// Include necessary headers for types, time, and integer definitions
#include <sys/types.h>
#include <time.h>
#include <stdint.h>
// Include custom headers for free space and initialization
#include "freespace.h"
#include "fsInit.h"
// Define constants for default and maximum directory entries and maximum file name length
#define DEFAULT_ENTRIES 16
#define MAXDIRENTRIES 50
#define MAX_FILE_NAME_LENGTH 256
// Forward declarations for custom types
typedef struct FreeSpace FreeSpace;
typedef struct FileAllocationTable FileAllocationTable;
// Define the structure for a directory entry with essential attributes
struct DirectoryEntry {
    char file_name[MAX_FILE_NAME_LENGTH];
    uint64_t file_size;
    time_t mtime;
    time_t atime;
    time_t ctime;
    char isDirectory;
    uint64_t location;
};

// Declare the prototype for the root directory initialization function
int initDirectory(int defaultEntries, struct DirectoryEntry* dirEntry, struct DirectoryEntry* parent, char* name);

#endif // DIRECTORY_H