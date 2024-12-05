#ifndef DEFINES_H
#define DEFINES_H

// Constants
#define NULL_DATA -1

// Record defines
#define RECORD_INT_COUNT 10 // do not change
#define MAX_KEY_VALUE 100000
#define RECORD_SIZE_IN_BYTES 44

// Block defines
#define RECORD_BLOCK_COUNT 8
#define RECORD_BLOCK_SIZE RECORD_SIZE_IN_BYTES *RECORD_BLOCK_COUNT // in bytes

// B tree definies
#define BTREE_D_FACTOR 4

// Main program file names
#define DISKFILE_FILENAME "disk.bin"
#define DISKFILE_PLAINTEXT_FILENAME "disk.txt"

// Debug file names
#define TEST_FILENAME "test.bin"

// Type of record
typedef int RecordType;
typedef char byte;

#endif