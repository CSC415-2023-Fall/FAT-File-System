/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	char* fileName; // holds the full file path or the name 
	int filesize; // the size of the file but it's in bytes.

	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open

b_io_fd b_open(char *filename, int flags) {
    b_io_fd returnFd;

    if (startup == 0) b_init();  // Initialize our system

    // Get a file control block (FCB)
    returnFd = b_getFCB();
    if (returnFd == -1) {
        printf("[OPEN] No free file descriptors available\n");
        return -1;
    }

    const char *notAllowedSymbols = "<>:\"/|?*";
    if (filename == NULL || filename[0] == '\0' || strpbrk(filename, notAllowedSymbols)) {
        printf("[OPEN] Invalid or empty filename, or contains disallowed characters\n");
        return -1;
    }

    // Validate flags
    if ((flags & O_RDONLY) && (flags & (O_TRUNC | O_CREAT | O_APPEND | O_RDWR))) {
        printf("[OPEN] Invalid combination of read-only and write flags\n");
        return -1;
    }
    if ((flags & O_TRUNC) && (flags & O_APPEND)) {
        printf("[OPEN] Invalid combination of truncate and append flags\n");
        return -1;
    }

    // File creation or truncation
    if ((flags & O_CREAT) && !fs_isFile(filename)) {
        if (mk_file(filename) != 0) {
            printf("[OPEN] Failed to create file: %s\n", filename);
            return -1;
        }
    } else if ((flags & O_TRUNC) && fs_isFile(filename)) {
        fs_delete(filename);
        if (mk_file(filename) != 0) {
            printf("[OPEN] Failed to truncate file: %s\n", filename);
            return -1;
        }
    }

    // Allocate and initialize the buffer
    fcbArray[returnFd].buf = malloc(B_CHUNK_SIZE);
    if (fcbArray[returnFd].buf == NULL) {
        printf("[OPEN] Buffer allocation failed\n");
        return -1;
    }
    fcbArray[returnFd].index = 0;
    fcbArray[returnFd].buflen = 0;
    fcbArray[returnFd].fileName = strdup(filename);
    if (fcbArray[returnFd].fileName == NULL) {
        printf(" Filename strdup failed\n");
        free(fcbArray[returnFd].buf);
        return -1;
    }

    // Retrieve file statistics
    struct fs_stat fsstat;
    if (fs_stat(filename, &fsstat) != 0) {
        printf(" Could not get file statistics for %s\n", filename);
        free(fcbArray[returnFd].buf);
        free(fcbArray[returnFd].fileName);
        return -1;
    }
    fcbArray[returnFd].filesize = fsstat.st_size;

    return returnFd;
}







// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
		
	return (0); //Change this
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
		
	return (0); //Change this
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	return (0);	//Change this
	}
	
// Interface to Close the file	
// int b_close (b_io_fd fd)
// 	{

// 	}
int b_close(b_io_fd fd) {
    // Check for valid file descriptor
    if (fd < 0 || fd >= MAXFCBS) {
        printf("[CLOSE] Invalid file descriptor\n");
        return -1; // Invalid file descriptor
    }

    // Check the file descriptor is in use
    if (fcbArray[fd].buf == NULL) {
        printf("[CLOSE] File descriptor not in use\n");
        return -1; // File descriptor not in use
    }

    // Free buffer if it exists
    if (fcbArray[fd].buf != NULL) {
        free(fcbArray[fd].buf);
        fcbArray[fd].buf = NULL;
    }

    // Free stored filename if it exists
    if (fcbArray[fd].fileName != NULL) {
        free(fcbArray[fd].fileName);
        fcbArray[fd].fileName = NULL;
    }

    // Reset elements of the file control block
    fcbArray[fd].index = 0;
    fcbArray[fd].buflen = 0;
    fcbArray[fd].filesize = 0;

   
    return 0;
}
