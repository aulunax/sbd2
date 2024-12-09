#ifndef DEFINES_H
#define DEFINES_H

// Constants
#define NULL_DATA -1

// Record defines
#define RECORD_INT_COUNT 10 // do not change
#define MAX_KEY_VALUE 100000
#define RECORD_SIZE_IN_BYTES 44

// B tree definies
#define BTREE_D_FACTOR 2
#define BTREE_MAX_CHILDREN 2 * BTREE_D_FACTOR
#define BTREE_MIN_CHILDREN BTREE_D_FACTOR

#define BTREE_HEADER_SIZE_IN_BYTES 2 * sizeof(int)
#define BTREE_PAGE_MAX_SIZE_IN_BYTES 3 * BTREE_MAX_CHILDREN * sizeof(int) + sizeof(int) + BTREE_HEADER_SIZE_IN_BYTES

// Block defines
#define RECORD_BLOCK_COUNT 8
#define RECORD_BLOCK_SIZE RECORD_SIZE_IN_BYTES *RECORD_BLOCK_COUNT // in bytes

#define BTREE_PAGE_BLOCK_COUNT 1

#define BLOCK_OPERATION_SUCCESSFUL 1
#define BLOCK_OPERATION_FAILED 0

#define OVERFLOW 1
#define UNDERFLOW 0

#define LEFT_SIBLING -1
#define RIGHT_SIBLING 1

// Main program file names
#define DATAFILE_FILENAME "data.bin"
#define DATAFILE_PLAINTEXT_FILENAME "data.txt"

#define INDEXFILE_FILENAME "index.bin"
#define INDEXFILE_PLAINTEXT_FILENAME "index.txt"

// Debug file names
#define TEST_FILENAME "test.bin"

// Type of record
typedef int RecordType;
typedef char byte;

#endif