#include <iostream>
#include <fstream>
#include <memory>
#include "defines.h"

#ifndef BLOCKINPUTOUTPUT_H
#define BLOCKINPUTOUTPUT_H

/// @brief Enum class to determine if the file should be read or written
enum class RWStatus
{
    Read,
    Write
};

class BlockInputOutput
{
protected:
    // internal variables for static block statistics
    static int allBlockWrites;
    static int allBlockReads;

    // internal variables for block statistics
    int blockWrites;
    int blockReads;

    // internal variables for file handling
    std::string filename;
    std::fstream file;

    // internal variables for block handling
    int currentBlockIndex;
    int blockSize;
    std::unique_ptr<char[]> block;
    int filledBlockIndex;
    int blockIndex;
    bool modifiedBlock = false;

    bool isFinished = false;

    /// @brief Writes the current block to the file
    virtual void writeBlock();

    /// @brief Reads the next block from the file
    virtual void readBlock();

    void readBlockAt(int blockOffset);
    void writeBlockAt(int blockOffset);

    void writeBlockAtEnd();

public:
    std::fstream *getRawFileStreamPtr();

    void flush();
    void finish();
    bool isDone()
    {
        return isFinished;
    }

    static int getAllBlockWrites()
    {
        return allBlockWrites;
    }

    static int getAllBlockReads()
    {
        return allBlockReads;
    }

    static void resetAllBlockStats()
    {
        allBlockReads = 0;
        allBlockWrites = 0;
    }

    int getBlockWrites()
    {
        return blockWrites;
    }

    int getBlockReads()
    {
        return blockReads;
    }

    std::string getFilename()
    {
        return filename;
    }

    /// @brief Construct a new BlockInputOutput object
    /// @param filename Filename of the file to read/write from/to
    /// @param rwStatus Variable to determine if the file should be read or written
    BlockInputOutput(std::string filename, int blockSize);

    /// @brief Destroy the BlockInputOutput object
    /// @note Closes the file if it is open
    virtual ~BlockInputOutput();
};

#endif
