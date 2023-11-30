/**************************************************************
* Class:  CSC-415-03 Fall 2021
* Names: Amandeep Singh, Akshat Sohal 
* Student IDs: 921287533, 917815046
* GitHub Name: Amandeep-Singh-24, sohal786
* Group Name: Tryhards
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Implementations of all file functions which 
* allows for fsshell.c to run properly. This is the driver
* for the file functions in shell.
*
**************************************************************/
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h>
#include "directory.h"
#include "fsInit.h"
#include "mfs.h"
#define MAX_PATH_LENGTH 256
char currentPath[MAX_PATH_LENGTH] = "/"; 

extern struct volume_control_block *vcb;
// extern struct FATEntry *fatTable;


/**
 * This function checks if a given directory entry is a directory.
 * It takes a pointer to a DirectoryEntry and returns 1 if it's a directory,
 * otherwise returns 0.
 *
 * @param dirEntry - Pointer to a DirectoryEntry to be checked.
 * 
 * @return int - Returns 1 if dirEntry is a directory, 0 otherwise.
 * 
 */
int IsADirectory(DirectoryEntry *dirEntry) {
    printf("[IsADirectory] Called\n");
    if (dirEntry == NULL) {
        printf("[IsADirectory] dirEntry is NULL\n");
        return 0; // Return 0 as it's not a valid directory
    }

    // Check if the entry is a directory
    printf("[IsADirectory] isDirectory value: %d\n", dirEntry->isDirectory);
    return dirEntry->isDirectory == 1;
    //return 1;
}

/**
 * This function writes a directory to the disk. It takes a pointer to the directory
 * to be written and the start block number, then writes the directory to the disk.
 *
 * @param dir - Pointer to the directory to be written.
 * 
 * @return int - Returns 0 on success, -1 on failure.
 * 
 */
int writeDirectoryToDisk(DirectoryEntry *dir/*, uint32_t startBlock, int numEntries*/) {
    uint32_t startBlock;
    startBlock = dir[0].location;
    int numEntries = dir[0].file_size / sizeof(DirectoryEntry);
    printf("[writeDirectoryToDisk] Called with startBlock: %u, numEntries: %d\n", startBlock, numEntries);
    if (dir == NULL) {
        printf("[writeDirectoryToDisk] dir is NULL\n");
        return -1;
    }

    int blockSize = vcb->block_size;
    int dirSize = dir[0].file_size;
    int blocksToWrite = (dirSize + blockSize - 1) / blockSize;
    printf("[writeDirectoryToDisk] BlockSize: %d, dirSize: %d, blocksToWrite: %d\n", blockSize, dirSize, blocksToWrite);
    uint64_t blocksWritten = LBAwrite((unsigned char*) dir, blocksToWrite, startBlock);
    if (blocksWritten != blocksToWrite) {
        fprintf(stderr, "Failed to write directory to disk.\n");
        return -1;
    }

    return 0;
}

/**
 * This function reads a directory from the disk. It takes the start block number
 * and a pointer to a buffer to store the read directory, then reads the directory
 * from the disk into the buffer.
 *
 * @param startBlock - The start block number on disk where the directory is stored.
 * 
 * @param dir - Pointer to the buffer to store the read directory.
 * 
 * @param numEntries - Number of entries in the directory.
 * 
 * @return int - Returns 0 on success, -1 on failure.
 * 
 */
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

/**
 * This function finds an entry in a directory. It searches for a token
 * in the parent directory and returns its index if found.
 *
 * @param parent - Pointer to the parent directory where the search is performed.
 * 
 * @param token - The name of the entry to be searched.
 * 
 * @return int - Returns the index of the found entry, or -1 if not found.
 * 
 */
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

/**
 * This function loads a directory from disk. It takes a DirectoryEntry struct
 * and loads the directory from the disk based on the location specified in the struct.
 *
 * @param dir - The DirectoryEntry struct that contains the location of the directory.
 * 
 * @return DirectoryEntry* - Returns a pointer to the loaded directory, or NULL on failure.
 * 
 */
DirectoryEntry *loadDirectory(DirectoryEntry dir) {
    printf("[loadDirectory] Called for directory with location: %lu\n", dir.location);

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

/**
 * This function parses a file path and fills a ppinfo struct with the parsed information.
 * It analyzes the given path and updates the ppinfo structure accordingly.
 *
 * @param path - The file path to be parsed.
 * 
 * @param ppi - Pointer to the ppinfo struct to be filled with parsed path information.
 * 
 * @return int - Returns 0 on successful parsing, otherwise returns an error code.
 * 
 */
int ParsePath(const char *path, ppinfo *ppi) {
    
    printf("[ParsePath] Called with path: '%s'\n", path);
    if (path == NULL || ppi == NULL) {
        printf("[ParsePath] path or ppi is NULL\n");
        return -1; 
    }
    printf("im in parse");

    //   if (path[0] != '/') {
    //     char *newPath = malloc(strlen(path) + 2); // Allocate memory for new path with '/' and null-terminator
    //     if (newPath == NULL) return -4; // Allocation failed
    //     sprintf(newPath, "/%s", path); // Prepend '/' to the path
    //     path = newPath; // Update the original path pointer
    // }
    if (path == NULL || ppi == NULL) return -1; 

    char *pathCopy = strdup(path); 
    if (pathCopy == NULL) return -4; 
    printf("\nPathcopy: %p", pathCopy);
    DirectoryEntry *startDir;
    if (pathCopy[0] == '/') {
        startDir = rootDir;
        printf("\nStartDir: %p", startDir);
    } else {
        startDir = currentDir;
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
            printf("[ParsePath] Path is root ('/'), \nsetting ppi->parent: %p, p\npi->index: %d, \nppi->lastElement: %p\n", (void*)ppi->parent, ppi->index, (void*)ppi->lastElement);
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
            printf("[ParsePath] Path is root ('/'), \nsetting ppi->parent: %p, \nppi->index: %d, \nppi->lastElement: %p\n", (void*)ppi->parent, ppi->index, (void*)ppi->lastElement);
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

/**
 * This function finds an empty entry in a directory. It searches for an empty
 * space in the given directory and returns the index of the first empty entry found.
 *
 * @param directory - Pointer to the directory where the search is performed.
 * 
 * @return int - Returns the index of the first empty entry, or -1 if no empty entry is found.
 * 
 */
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

/**
 * This function creates a new directory with the specified pathname and mode.
 * It checks for the validity of the path and creates a new directory entry in the file system.
 *
 * @param pathname - The path where the new directory is to be created.
 * 
 * @param mode - The mode of the new directory.
 * 
 * @return int - Returns 0 on successful creation, -1 on failure.
 * 
 */
int fs_mkdir(const char *pathname, mode_t mode) {
    printf("[MKDIR] Called with pathname: '%s'\n", pathname);

    if (pathname == NULL) {
        printf("[MKDIR] Error: Pathname is NULL\n");
        return -1;
    }

    char fullPath[MAX_PATH_LENGTH];
    // Handle relative path
    if (pathname[0] != '/') {
        snprintf(fullPath, sizeof(fullPath), "%s/%s", currentPath, pathname);
    } else {
        strncpy(fullPath, pathname, sizeof(fullPath));
    }
    fullPath[sizeof(fullPath) - 1] = '\0';  // Ensure null termination

    ppinfo ppi;
    int parseResult = ParsePath(fullPath, &ppi);
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
    printf("\n\nBefore initDir\n");
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
    if (writeDirectoryToDisk(ppi.parent /*, ppi.parent[0].location, 10*/) != 0) {
        printf("[MKDIR] Error: Failed to write to disk\n");
        // Release the allocated blocks if necessary
        // releaseSingleBlock(newDir->location);  // Uncomment if needed
        return -1;
    }

    printf("[MKDIR] Directory '%s' created successfully.\n", newDir->file_name);
    return 0;
}

/**
 * Checks if the path refers to a directory. 
 *
 * @param pathname - A pointer to a string containing the path to check.
 * 
 * @return int - Returns 1 if the path refers to a directory, 0 otherwise.
 * 
 */
int fs_isDir(const char *pathname) {
    printf("remove reached here\n");
    if (pathname == NULL) {
        printf("[IS DIR] Pathname is NULL\n");
        return 0; // 0 indicates not a directory
    }

    ppinfo ppi;
    
    // if (ParsePath(pathname, &ppi) != 0) {
    //     printf("[IS DIR] ParsePath error for %s\n", pathname);
    //     return 0; 
    // }

    if (ppi.parent == NULL || ppi.index == -1) {
        printf("[IS DIR] %s does not exist or not a valid entry\n", pathname);
        return 0; 

         printf("[IS DIR] ppi.parent: %p\n", (void*)ppi.parent); // Print the address of the parent
    if (ppi.parent != NULL) {
        // If parent is not NULL, print details of the parent directory entry
        printf("[IS DIR] ppi.parent details - \nfile_name: %s, \nisDirectory: %d\n", ppi.parent->file_name, ppi.parent->isDirectory);
    }

    printf("[IS DIR] ppi.lastElement: %s\n", ppi.lastElement); // Print the last element of the path
    printf("[IS DIR] ppi.index: %d\n", ppi.index); // Print the index of the parent of the last element


    if (IsADirectory(&ppi.parent[ppi.index])) {
        return 1; 
    } else {
        return 0; // Not a directory
    }
}
}

/**
 * Opens a directory stream corresponding to the directory name, and returns a pointer to the directory stream.
 *
 * @param pathname - A pointer to a string containing the name of the directory to be opened.
 * 
 * @return fdDir* - Returns a pointer to the opened directory stream, or NULL on error.
 * 
 */
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

    dirStream->directory = loadDirectory(ppi.parent[ppi.index]);
    dirStream->dirEntryPosition = 0;
    dirStream->d_reclen = MAXDIRENTRIES;  
    return dirStream;
}

/**
 * Reads a directory entry from the directory stream pointed to by dirp.
 *
 * @param dirp - A pointer to the directory stream.
 * 
 * @return struct fs_diriteminfo* - Returns a pointer to a directory entry, or NULL on reaching the end of the directory or on error.
 * 
 */
struct fs_diriteminfo *fs_readdir(fdDir *dirp) {
    //    printf("[fs_readdir] Called\n");
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

/**
 * Closes the directory stream associated with dirp.
 *
 * @param dirp - A pointer to the directory stream.
 * 
 * @return int - Returns 0 on success, or -1 on failure.
 * 
 */
int fs_closedir(fdDir *dirp) {
        printf("\n[fs_closedir] Called\n");
    if (dirp != NULL) {
        if (dirp->directory != NULL) {
            free(dirp->directory);  // Free loaded directory
        }
        free(dirp);
        return 0;
    }
    return -1;
}

/**
 * Checks if the path refers to a file.
 *
 * @param pathname - A pointer to a string containing the path to check.
 * 
 * @return int - Returns 1 if the path refers to a file, 0 otherwise.
 * 
 */
int fs_isFile(char *pathname) {
    printf("is_file is reached by the remove function\n");
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

/**
 * Retrieves the current working directory and stores it in the buffer pointed to by buf.
 *
 * @param buf - A pointer to a buffer where the current working directory will be stored.
 * 
 * @param size - The size of the buffer.
 * 
 * @return char* - Returns a pointer to buf on success, or NULL on failure.
 * 
 */
char* fs_getcwd(char* buf, size_t size) {
    if (currentPath == NULL) {
        return NULL;
    }

    // Check if the provided buffer is large enough for the current path
    if (size < strlen(currentPath) + 1) {
        return NULL;
    }

    // Copy the current path to the provided buffer
    strncpy(buf, currentPath, size);
    buf[size - 1] = '\0'; // Ensure null-termination
    return buf;
}

/**
 * Changes the current working directory to that specified in path.
 *
 * @param path - A pointer to a string containing the path of the new working directory.
 * 
 * @return int - Returns 0 on success, or -1 on failure.
 * 
 */
int fs_setcwd(char* path) {
    printf("\n[fs_setcwd] Current working directory before change: %s\n", currentPath);
    if (path == NULL) {
        return -1;
    }

    ppinfo ppi;
    if (ParsePath(path, &ppi) != 0 || ppi.parent == NULL || ppi.index == -1) {
        return -1; // Path parsing failed or path does not exist
    }
    printf("[fs_setcwd] New path to set as CWD: %s\n", path);

    if (!IsADirectory(&ppi.parent[ppi.index])) {
        return -1; // Not a directory
    }

    if (path[0] == '/') {
        strncpy(currentPath, path, MAX_PATH_LENGTH);
        currentPath[MAX_PATH_LENGTH - 1] = '\0'; // Ensure null-termination
    } else {
        size_t currentLength = strlen(currentPath);
        if (currentPath[currentLength - 1] != '/') {
            // Add a slash if the current path doesn't end with one
            strcat(currentPath, "/");
            currentLength++;
        }
        if (currentLength + strlen(path) < MAX_PATH_LENGTH) {
            strcat(currentPath, path);
        } 
    }

    printf("[fs_setcwd] Current working directory after change: %s\n", currentPath);

    currentDir = loadDirectory(ppi.parent[ppi.index]) ; 
    printf("\n CWD: %p", cwd);
    printf("\n");
    return 0;
}

// DirectoryEntry* getChildDirectory(DirectoryEntry *parent, char *childName) {
//     if (parent == NULL || childName == NULL) {
//         return NULL;
//     }

//     int index = FindEntryInDir(parent, childName);
//     if (index == -1) {
//         return NULL; // Child directory not found
//     }

//     return &parent[index]; // Return the child directory
// }


// DirectoryEntry* getParentDirectory(ppinfo *ppi) {
//     if (ppi == NULL || ppi->parent == NULL) {
//         return NULL; // Invalid input or root directory
//     }

//     return ppi->parent; // Return the parent directory
// }


// int fs_setcwd(char* path) {
//     if (path == NULL) {
//         return -1;
//     }

//     DirectoryEntry *targetDir;
//     char *nextDirName;
//     char tempPath[MAX_PATH_LENGTH];

//       ppinfo ppi;
//     if (ParsePath(path, &ppi) != 0 || ppi.parent == NULL || ppi.index == -1) {
//         return -1; // Path parsing failed or path does not exist
//    }

//     if (path[0] == '/') {
//         // Absolute path: start from the root
//         targetDir = rootDir;
//         strcpy(tempPath, "/");
//     } else {
//         // Relative path: start from the current working directory
//         targetDir = cwd;
//         strcpy(tempPath, currentPath);
//     }

//     // Tokenize the path and traverse directories
//     nextDirName = strtok(path, "/");
//     while (nextDirName != NULL) {
//         if (strcmp(nextDirName, "..") == 0) {
//             // Move up to the parent directory
//             targetDir = getParentDirectory(&ppi);
//         } else if (strcmp(nextDirName, ".") != 0) {
//             // Move into the child directory
//             targetDir = getChildDirectory(targetDir, nextDirName);
//             if (targetDir == NULL) {
//                 return -1; // Directory not found
//             }
//         }

//         // Update tempPath accordingly
//         // (You'll need to handle the concatenation and length check here)

//         nextDirName = strtok(NULL, "/");
//     }

//     // Update cwd and currentPath
//     cwd = targetDir;
//     strncpy(currentPath, tempPath, MAX_PATH_LENGTH);
//     currentPath[MAX_PATH_LENGTH - 1] = '\0'; // Ensure null-termination

//     return 0;
// }

// Implement `getParentDirectory` and `getChildDirectory` based on your filesystem structure.


/**
 * Obtains information about the file or directory at the specified path.
 *
 * @param path - A pointer to a string containing the path to check.
 * 
 * @param buf - A pointer to a struct fs_stat where the information is to be stored.
 * 
 * @return int - Returns 0 on success, or -1 on failure.
 * 
 */
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

/**
 * Deletes the file specified by filename.
 *
 * @param filename - A pointer to a string containing the name of the file to be deleted.
 * 
 * @return int - Returns 0 on success, or -1 on failure.
 * 
 */
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

    //Check if file is found in the directory
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
   // int blocksToWrite = (vcb->block_size + sizeof(DirectoryEntry) * MAXDIRENTRIES - 1) / vcb->block_size;
    if (writeDirectoryToDisk(ppi.parent/*, ppi.parent[0].location, blocksToWrite*/) != 0) {
        printf("Error: Failed to update directory on disk\n");
        return -1;
    }

    printf("File %s deleted successfully\n", filename);
    return 0;
}

/**
 * Removes the directory specified by pathname.
 *
 * @param pathname - A pointer to a string containing the path of the directory to be removed.
 * 
 * @return int - Returns 0 on success, or -1 on failure.
 * 
 */
int fs_rmdir(const char *pathname) {
    // Check if the pathname is NULL
    if (pathname == NULL) {
        printf("[RMDIR] Error: Pathname is NULL\n");
        return -1;
    }

     // Parse the path to get directory information
     ppinfo ppi;
    int parseResult = ParsePath(pathname, &ppi);
    // char fullPath[MAX_PATH_LENGTH];
    // if (pathname[0] != '/') {
    //     // For relative paths, prepend the currentPath
    //     snprintf(fullPath, MAX_PATH_LENGTH, "%s/%s", currentPath, pathname);
    // } else {
    //     // For absolute paths, use the path as is
    //     strncpy(fullPath, pathname, MAX_PATH_LENGTH);
    // }
    // fullPath[MAX_PATH_LENGTH - 1] = '\0'; 

    // Use fullPath in ParsePath
    //ppinfo ppi;
    //int parseResult = (ParsePath(fullPath, &ppi));
    if (parseResult != 0) {
        printf("[RMDIR] Error: ParsePath failed\n");
        return -1;
    }

    //Check if the directory exists and is a directory
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
    
    // for (int i = 0; i < MAXDIRENTRIES; i++) {
    //     if (strcmp(dir[i].file_name, "") != 0) {
    //         printf("[RMDIR] Error: Directory is not empty\n");
    //         free(dir);
    //         return -1;
    //     }
    // }
    free(dir);

    // Remove the directory entry from its parent
    memset(&ppi.parent[ppi.index], 0, sizeof(DirectoryEntry));

    // Release the blocks allocated to the directory
    releaseMultipleBlocks(ppi.parent[ppi.index].location);

    // Write the updated parent directory back to disk
    if (writeDirectoryToDisk(ppi.parent/*, ppi.parent->location, MAXDIRENTRIES*/) != 0) {
        printf("[RMDIR] Error: Failed to write updated parent directory to disk\n");
        return -1;
    }



    char removedDirPath[MAX_PATH_LENGTH];
    strcpy(removedDirPath, pathname);
    if (strstr(currentPath, removedDirPath) == currentPath) {
        // The removed directory is part of the current path
        // Find the parent path of the removed directory
        char *lastSlash = strrchr(currentPath, '/');
        if (lastSlash != NULL) {
            *lastSlash = '\0'; 
        }
        if (strlen(currentPath) == 0) {
            strcpy(currentPath, "/"); 
        }
    }

    printf("[RMDIR] Directory %s removed successfully\n", pathname);
    return 0;
}
