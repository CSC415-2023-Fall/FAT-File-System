#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h>
#include "directory.h"
#include "fsInit.h"
#include "mfs.h"
extern struct volume_control_block *vcb;
// extern struct FATEntry *fatTable;

int IsADirectory(DirectoryEntry *dirEntry) {
    printf("[IsADirectory] Called\n");
    if (dirEntry == NULL) {
        printf("[IsADirectory] dirEntry is NULL\n");
        return 0; // Return 0 as it's not a valid directory
    }

    // Check if the entry is a directory
    printf("[IsADirectory] isDirectory value: %d\n", dirEntry->isDirectory);
    return dirEntry->isDirectory == 1;
}

int writeDirectoryToDisk(DirectoryEntry *dir, uint32_t startBlock, int numEntries) {
    printf("[writeDirectoryToDisk] Called with startBlock: %u, numEntries: %d\n", startBlock, numEntries);
    if (dir == NULL) {
        printf("[writeDirectoryToDisk] dir is NULL\n");
        return -1;
    }

    int blockSize = vcb->block_size;
    int dirSize = numEntries * sizeof(DirectoryEntry);
    int blocksToWrite = (dirSize + blockSize - 1) / blockSize;
    printf("[writeDirectoryToDisk] BlockSize: %d, dirSize: %d, blocksToWrite: %d\n", blockSize, dirSize, blocksToWrite);
    uint64_t blocksWritten = LBAwrite((unsigned char*) dir, blocksToWrite, startBlock);
    if (blocksWritten != blocksToWrite) {
        fprintf(stderr, "Failed to write directory to disk.\n");
        return -1;
    }

    return 0;
}

int readDirectoryFromDisk(uint32_t startBlock, DirectoryEntry *dir, int numEntries) {
    printf("[readDirectoryFromDisk] Called with startBlock: %u, numEntries: %d\n", startBlock, numEntries);
    if (dir == NULL) {
        printf("[readDirectoryFromDisk] dir is NULL\n");
        return -1;
    }

    int blockSize = vcb->block_size;
    int dirSize = numEntries * sizeof(DirectoryEntry);
    int blocksToRead = (dirSize + blockSize - 1) / blockSize;
    printf("[readDirectoryFromDisk] blockSize: %d, dirSize: %d, blocksToRead: %d\n", blockSize, dirSize, blocksToRead);

    uint64_t blocksRead = LBAread((unsigned char*) dir, blocksToRead, startBlock);
    if (blocksRead != blocksToRead) {
        fprintf(stderr, "Failed to read directory from disk.\n");
        return -1;
    }

    return 0;
}

int FindEntryInDir(DirectoryEntry *parent, char *token) {
    printf("[FindEntryInDir] Searching for token: '%s'\n", token);
    if (parent == NULL || token == NULL) {
        printf("[FindEntryInDir] parent or token is NULL\n");
        return -1;
    }

    for (int i = 0; i < 40; i++) {
        if (strcmp(parent[i].file_name, token) == 0) {
            printf("[FindEntryInDir] Found token at index: %d\n", i);
            return i;  
        }
    }

    printf("[FindEntryInDir] Token not found\n");
    return -1;  
}

DirectoryEntry *loadDirectory(DirectoryEntry dir) {
    printf("[loadDirectory] Called for directory with location: %llu\n", dir.location);

    int block_size = vcb->block_size; 
    int blocks_need = (dir.file_size + block_size - 1) / block_size;
    printf("[loadDirectory] block_size: %d, blocks_need: %d\n", block_size, blocks_need);
    
    DirectoryEntry *entry = malloc(blocks_need * block_size);
    if (!entry) {
        printf("[loadDirectory] Memory allocation failed\n");
        return NULL;
    }

    if (readDirectoryFromDisk(dir.location, entry, dir.file_size / sizeof(DirectoryEntry)) == -1) {
        printf("[LOAD DIR] can't load dir\n");
        return NULL;
    }
    return entry;
}

int ParsePath(char *path, ppinfo *ppi) {
    printf("[ParsePath] Called with path: '%s'\n", path);
    if (path == NULL || ppi == NULL) {
        printf("[ParsePath] path or ppi is NULL\n");
        return -1; 
    }
    printf("im in parse");
    if (path == NULL || ppi == NULL) return -1; 

    char *pathCopy = strdup(path); 
    if (pathCopy == NULL) return -4; 
    printf("\nPathcopy: %p", pathCopy);
    DirectoryEntry *startDir;
    if (pathCopy[0] == '/') {
        startDir = rootDir;
        printf("\nStartDir: %p", startDir);
    } else {
        startDir = cwd;
        printf("\nIm in the else statement");

    }

    DirectoryEntry *parent = startDir;
    char *saveptr;
    char *token1 = strtok_r(pathCopy, "/", &saveptr);
    printf("\nBefore ppi stuff\n");
    if (token1 == NULL) {
        if (strcmp(pathCopy, "/") == 0) {
            ppi->parent = parent;
            ppi->index = -1;
            ppi->lastElement = NULL;
            printf("[ParsePath] Path is root ('/'), setting ppi->parent: %p, ppi->index: %d, ppi->lastElement: %p\n", (void*)ppi->parent, ppi->index, (void*)ppi->lastElement);
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
            printf("[ParsePath] Path is root ('/'), setting ppi->parent: %p, ppi->index: %d, ppi->lastElement: %p\n", (void*)ppi->parent, ppi->index, (void*)ppi->lastElement);
            free(pathCopy); 
            return 0; 
        }

        if (index == -1) {
            free(pathCopy);
            return -2; // Error code -2: Entry not found in directory
        }
        if (!IsADirectory(&(parent[index]))) {
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
    printf("[ParsePath] Returning -1 (should not reach here)\n");
    return -1; // Should not reach here
}

int findEmptyEntry(DirectoryEntry *directory) {
    printf("[findEmptyEntry] Called\n");
    if (directory == NULL) {
        printf("[findEmptyEntry] directory is NULL\n");
        return -1;
    }

    for (int i = 0; i < MAXDIRENTRIES; ++i) {
        if (strcmp(directory[i].file_name, "") == 0) {
            printf("[findEmptyEntry] Found empty entry at index: %d\n", i);
            return i;
        }
    }

    printf("[findEmptyEntry] No empty entry found\n");
    return -1;
}

int fs_mkdir(const char *pathname, mode_t mode) {
    printf("[MKDIR] Called with pathname: '%s'\n", pathname);

    if (pathname == NULL) {
        printf("[MKDIR] Error: Pathname is NULL\n");
        return -1;
    }

    ppinfo ppi;
    int parseResult = ParsePath((char *)pathname, &ppi);
    printf("[MKDIR] ParsePath result: %d\n", parseResult);
    if (parseResult != 0) {
        printf("[MKDIR] Error: Invalid path\n");
        return -1;
    }
    if (ppi.parent == NULL) {
        printf("[MKDIR] Error: Parent directory is NULL\n");
        return -1;
    }
    if (ppi.index != -1) {
        printf("[MKDIR] Error: Directory already exists\n");
        return -1;
    }
    int count;
    count = vcb->start_block;
    printf("Count: %d", count);
    // Using initDirectory to create the new directory
    printf("\n\nBefore initDir");
    DirectoryEntry* newDir = initDirectory(DEFAULT_ENTRIES, NULL, ppi.parent, ppi.lastElement);
    if (newDir == NULL) {
        printf("[MKDIR] Error: Failed to initialize new directory\n");
        return -1;
    }
    printf("\nAfter initDir");

    // Update the name of the new directory
    strncpy(newDir->file_name, ppi.lastElement, MAX_FILE_NAME_LENGTH);
    newDir->file_name[MAX_FILE_NAME_LENGTH - 1] = '\0';
    

    printf("[MKDIR] Updated new directory name to: '%s'\n", newDir->file_name);

    // Now update the parent directory to include the new directory
    int newIndex = findEmptyEntry(ppi.parent);
    printf("[MKDIR] New index in parent directory: %d\n", newIndex);
    if (newIndex == -1) {
        printf("[MKDIR] Error: No space in parent directory\n");
        // Release the allocated blocks if necessary
        // releaseSingleBlock(newDir->location);  // Uncomment if needed
        return -1;
    }

    ppi.parent[newIndex] = *newDir;

    // Write the updated parent directory back to disk
    printf("[MKDIR] Writing updated parent directory to disk\n");
    if (writeDirectoryToDisk(ppi.parent, ppi.parent[0].location, 10) != 0) {
        printf("[MKDIR] Error: Failed to write to disk\n");
        // Release the allocated blocks if necessary
        // releaseSingleBlock(newDir->location);  // Uncomment if needed
        return -1;
    }

    printf("[MKDIR] Directory '%s' created successfully.\n", newDir->file_name);
    return 0;
}

int fs_isDir(char *pathname) {
    if (pathname == NULL) {
        printf("[IS DIR] Pathname is NULL\n");
        return 0; // 0 indicates not a directory
    }

    ppinfo ppi;
    if (ParsePath(pathname, &ppi) != 0) {
        printf("[IS DIR] ParsePath error for %s\n", pathname);
        return 0; 
    }

    if (ppi.parent == NULL || ppi.index == -1) {
        printf("[IS DIR] %s does not exist or not a valid entry\n", pathname);
        return 0; 

    if (IsADirectory(&ppi.parent[ppi.index])) {
        return 1; 
    } else {
        return 0; // Not a directory
    }
}
}


fdDir *fs_opendir(const char *pathname) {
        printf("[fs_opendir] Called with pathname: %s\n", pathname);
    // Check if the pathname is a directory
    if (!fs_isDir(pathname)) {
        printf("Error: %s is not a directory\n", pathname);
        return NULL;
    }

    ppinfo ppi;
    if (ParsePath(pathname, &ppi) != 0) {
        printf("Error: Unable to parse the path\n");
        return NULL;
    }

    fdDir *dirStream = (fdDir *)malloc(sizeof(fdDir));
    if (!dirStream) {
        printf("Error: Unable to allocate memory for directory stream\n");
        return NULL;
    }

    dirStream->directory = loadDirectory(*ppi.parent);
    dirStream->dirEntryPosition = 0;
    dirStream->d_reclen = MAXDIRENTRIES;  // Assuming MAXDIRENTRIES is the max number of entries in a directory

    return dirStream;
}



struct fs_diriteminfo *fs_readdir(fdDir *dirp) {
        printf("[fs_readdir] Called\n");
    if (dirp == NULL || dirp->directory == NULL) {
        return NULL;
    }

    while (dirp->dirEntryPosition < dirp->d_reclen) {
        DirectoryEntry *entry = &dirp->directory[dirp->dirEntryPosition++];
        if (strcmp(entry->file_name, "") != 0) {  // Check if the entry is not empty
            struct fs_diriteminfo *dirItem = (struct fs_diriteminfo *)malloc(sizeof(struct fs_diriteminfo));
            strncpy(dirItem->d_name, entry->file_name, 255);
            dirItem->d_name[255] = '\0';  // Ensure null termination
            dirItem->fileType = entry->isDirectory ? FT_DIRECTORY : FT_REGFILE;
            return dirItem;
        }
    }
    return NULL;
}


int fs_closedir(fdDir *dirp) {
        printf("[fs_closedir] Called\n");
    if (dirp != NULL) {
        if (dirp->directory != NULL) {
            free(dirp->directory);  // Free loaded directory
        }
        free(dirp);
        return 0;
    }
    return -1;
}


int fs_isFile(char *pathname) {
    if (pathname == NULL) {
        printf("[IS FILE] Pathname is NULL\n");
        return 0;  // 0 indicates not a file
    }

    ppinfo ppi;
    if (ParsePath(pathname, &ppi) != 0) {
        printf("[IS FILE] ParsePath error for %s\n", pathname);
        return 0; 
    }

    if (ppi.parent == NULL || ppi.index == -1) {
        printf("[IS FILE] %s does not exist or is not a valid entry\n", pathname);
        return 0;
    }

    if (!IsADirectory(&ppi.parent[ppi.index])) {
        return 1;  // 1 indicates it is a file
    } else {
        return 0;  // Not a file
    }
}

char* fs_getcwd(char* buf, size_t size) {
    if (cwd == NULL) {
        return NULL;
    }

    // Check if provided buffer is large enough
    if (size < strlen(cwd->file_name) + 1) {
        return NULL;
    }

    strncpy(buf, cwd->file_name, size);
    return buf;
}

int fs_setcwd(char* path) {
    if (path == NULL) {
        return -1;
    }

    // Use your path parsing function to find the directory entry
    ppinfo ppi;
    if (ParsePath(path, &ppi) != 0 || ppi.parent == NULL || ppi.index == -1) {
        return -1; // Path parsing failed or path does not exist
    }

    // Check if the path is indeed a directory
    if (!IsADirectory(&ppi.parent[ppi.index])) {
        return -1; // Not a directory
    }

    cwd = &ppi.parent[ppi.index]; // Set the CWD to the new directory
    printf("INSIDE CWD\n");
    return 0;
}

int fs_stat(const char* path, struct fs_stat* buf) {
    if (path == NULL || buf == NULL) {
        return -1;
    }

    ppinfo ppi;
    if (ParsePath(path, &ppi) != 0 || ppi.parent == NULL || ppi.index == -1) {
        return -1; // Path parsing failed or path does not exist
    }

    DirectoryEntry* entry = &ppi.parent[ppi.index];

    buf->st_size = entry->file_size;
    buf->st_blksize = vcb->block_size; // Assuming block size is stored in VCB
    buf->st_blocks = (entry->file_size + vcb->block_size - 1) / vcb->block_size;
    buf->st_accesstime = entry->atime;
    buf->st_modtime = entry->mtime;
    buf->st_createtime = entry->ctime;

    return 0;
}

int fs_delete(char *filename) {
    // Check if filename is NULL
    if (filename == NULL) {
        printf("Error: Filename is NULL\n");
        return -1;
    }

    // Check if the file is actually a directory
    if (fs_isDir(filename)) {
        printf("Error: Cannot delete a directory\n");
        return -1;
    }

    // Parse the path to get the directory entry
    ppinfo ppi;
    if (ParsePath(filename, &ppi) != 0) {
        printf("Error parsing path: %s\n", filename);
        return -1;
    }

    // Check if file is found in the directory
    if (ppi.index == -1 || ppi.parent == NULL) {
        printf("Error: File not found\n");
        return -1;
    }

    // Free the blocks allocated to the file
    releaseMultipleBlocks(ppi.parent[ppi.index].location);

    // Clear the directory entry
    memset(&ppi.parent[ppi.index], 0, sizeof(DirectoryEntry));
    strcpy(ppi.parent[ppi.index].file_name, "");

    // Write the updated directory back to the disk
    int blocksToWrite = (vcb->block_size + sizeof(DirectoryEntry) * MAXDIRENTRIES - 1) / vcb->block_size;
    if (writeDirectoryToDisk(ppi.parent, ppi.parent[0].location, blocksToWrite) != 0) {
        printf("Error: Failed to update directory on disk\n");
        return -1;
    }

    printf("File %s deleted successfully\n", filename);
    return 0;
}

int fs_rmdir(const char *pathname) {
    // Check if the pathname is NULL
    if (pathname == NULL) {
        printf("[RMDIR] Error: Pathname is NULL\n");
        return -1;
    }

    // Parse the path to get directory information
    ppinfo ppi;
    int parseResult = ParsePath(pathname, &ppi);
    if (parseResult != 0) {
        printf("[RMDIR] Error: ParsePath failed\n");
        return -1;
    }

    // Check if the directory exists and is a directory
    if (ppi.index == -1 || ppi.parent == NULL || !IsADirectory(&ppi.parent[ppi.index])) {
        printf("[RMDIR] Error: %s is not a valid directory\n", pathname);
        return -1;
    }

    // Check if the directory is empty
    DirectoryEntry *dir = loadDirectory(ppi.parent[ppi.index]);
    if (dir == NULL) {
        printf("[RMDIR] Error: Failed to load directory\n");
        return -1;
    }
    
    for (int i = 0; i < MAXDIRENTRIES; i++) {
        if (strcmp(dir[i].file_name, "") != 0) {
            printf("[RMDIR] Error: Directory is not empty\n");
            free(dir);
            return -1;
        }
    }
    free(dir);

    // Remove the directory entry from its parent
    memset(&ppi.parent[ppi.index], 0, sizeof(DirectoryEntry));

    // Release the blocks allocated to the directory
    releaseMultipleBlocks(ppi.parent[ppi.index].location);

    // Write the updated parent directory back to disk
    if (writeDirectoryToDisk(ppi.parent, ppi.parent->location, MAXDIRENTRIES) != 0) {
        printf("[RMDIR] Error: Failed to write updated parent directory to disk\n");
        return -1;
    }

    printf("[RMDIR] Directory %s removed successfully\n", pathname);
    return 0;
}
