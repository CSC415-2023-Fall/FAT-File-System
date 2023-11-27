
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h>
#include "directory.h"
#include "fsInit.h"
#include "mfs.h"
extern struct volume_control_block *vcb;

int ParsePath(char *path, ppinfo *ppi) {
    if (path == NULL || ppi == NULL) return -1; 

    char *pathCopy = strdup(path); 
    if (pathCopy == NULL) return -4; 

    DirectoryEntry *startDir;
    if (pathCopy[0] == '/') {
        startDir = rootDir;
    } else {
        startDir = cwd;
    }

    DirectoryEntry *parent = startDir;
    char *saveptr;
    char *token1 = strtok_r(pathCopy, "/", &saveptr);

    if (token1 == NULL) {
        if (strcmp(pathCopy, "/") == 0) {
            ppi->parent = parent;
            ppi->index = -1;
            ppi->lastElement = NULL;
            free(pathCopy); // Free the copied path
            return 0; 
        }
        free(pathCopy); // Free the copied path
        return -1; // Error code -1: Path is empty
    }

    while (token1 != NULL) {
        int index = FindEntryInDir(parent, token1);
        char *token2 = strtok_r(NULL, "/", &saveptr);

        if (token2 == NULL) {
            ppi->parent = parent;
            ppi->index = index;
            ppi->lastElement = strdup(token1);
            free(pathCopy); 
            return 0; 
        }

        if (index == -1) {
            free(pathCopy);
            return -2; // Error code -2: Entry not found in directory
        }
        if (!IsADirectory(parent[index])) {
            free(pathCopy); 
            return -2; // Error code -2: Entry is not a directory
        }

        DirectoryEntry *temp = loadDirectory(parent[index]);
        if (temp == NULL) {
            free(pathCopy); 
            return -3; // Error code -3: Failed to initialize directory
        }

        parent = temp;
        token1 = token2;
    }

    free(pathCopy); 
    return -1; // Should not reach here
}



int FindEntryInDir(DirectoryEntry *parent, char *token) {
    if (parent == NULL || token == NULL) {
        return -1;
    }

    
    for (int i = 0; i < 40; i++) {
        if (strcmp(parent[i].file_name, token) == 0) {
            return i;  
        }
    }

    return -1;  
}

//Size of entries/size of dir entries = number of entries

DirectoryEntry *loadDirectory(DirectoryEntry dir)
{
    // check if we want root or current dir
    // if (strcmp(dir.path, "/") == 0)
    //     return rootDir;
    // if (strcmp(dir.path, cwd[0].path) == 0)
    //     return cwd;

    // if not load it to memory

    //Block_size and blocks_need are used to forumalate amount of memory needed
    //for a the target directory to have
    int block_size = 512; 
    int blocks_need = (dir.file_size + block_size - 1) / block_size;
    DirectoryEntry *entry = malloc(blocks_need * block_size);

    //Runs through read Disk which runs through the fat implementation of getNew
    //Returns of successful or failure (-0,-1)
    if (read_from_disk(entry, dir.location, blocks_need, block_size) == -1)
    {
        printf("[LOAD DIR] can't load dir\n");
        return NULL;
    }
    return entry;
}



// Function to read directory from disk
int readDirectoryFromDisk(uint32_t startBlock,DirectoryEntry *dir, int numEntries) {
    if (dir == NULL) return -1;

    int blockSize = vcb->block_size;
    int dirSize = numEntries * sizeof(DirectoryEntry);
    int blocksToRead = (dirSize + blockSize - 1) / blockSize;

    uint64_t blocksRead = LBAread((unsigned char*) dir, blocksToRead, startBlock);
    if (blocksRead != blocksToRead) {
        fprintf(stderr, "Failed to read directory from disk.\n");
        return -1;
    }

    return 0;
}

//run a loop 
//whichblock: given a logical block number(block0) of a file , what block number on disk is that actually exists
//this returns the block number 
int writeDirectoryToDisk(DirectoryEntry *dir, uint32_t startBlock, int numEntries) {
    if (dir == NULL) return -1;

    int blockSize = vcb->block_size;
    int dirSize = numEntries * sizeof(DirectoryEntry);
    int blocksToWrite = (dirSize + blockSize - 1) / blockSize;

    uint64_t blocksWritten = LBAwrite((unsigned char*) dir, blocksToWrite, startBlock);
    if (blocksWritten != blocksToWrite) {
        fprintf(stderr, "Failed to write directory to disk.\n");
        return -1;
    }

    return 0;
}


int fs_mkdir(const char *pathname, mode_t mode) {
    if (pathname == NULL) return -1;

    ppinfo ppi;
    int parseResult = ParsePath((char)pathname, &ppi);
    if (parseResult != 0) {
        printf(" [MKDIR] invalid path\n");
        return -1;
    }

    if (ppi.index != -1 || ppi.parent == NULL) {
        printf("[MKDIR] Directory already exists or parent is NULL\n");
        return -1;
    }

    // Create a new directory entry
    DirectoryEntry newDir;
    // Initialize newDir with appropriate values
    // ...

    // Find an empty entry in the parent directory
    int newIndex = findEmptyEntry(ppi.parent);
    if (newIndex == -1) {
        printf("[MKDIR] No space in parent directory\n");
        return -1;
    }

    // Update the parent directory with the new directory entry
    ppi.parent[newIndex] = newDir;

    // Write the updated parent directory back to disk
    int blocks = (sizeof(DirectoryEntry) * 40 + 512 - 1) / 512;
    if (writeDirectoryToDisk(ppi.parent, ppi.parent[0].location, 40) != 0) {
        printf("[MKDIR] Failed to write to disk\n");
        return -1;
    }

    return 0; // Success
}



//  DirectoryEntry newDir;
//  //call inti directory
//  //helper function to find non used entry : loop the directory loaded in memory
//  //parent directory , return an index

//  //before return from mkdir call writetodisk,
//  //blocks = size+(BlockSize-1)/Blocksize 
//  //first parameter
//  //second is 1 
//  //
//  //can create another function called Next Block, first block ->>> startBlock


// }

