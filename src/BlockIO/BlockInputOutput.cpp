#include "BlockInputOutput.h"

int BlockInputOutput::allBlockWrites = 0;
int BlockInputOutput::allBlockReads = 0;

void BlockInputOutput::writeBlock()
{
    blockWrites++;
    allBlockWrites++;
    file.write(block.get(), blockSize);
    if (file.eof())
    {
        file.clear();
    }
    if (file.fail())
    {
        std::cerr << "Error: Writing to file failed.\n";
    }

    modifiedBlock = false;
    file.flush();
}

void BlockInputOutput::readBlock()
{
    blockReads++;
    allBlockReads++;
    file.read(block.get(), blockSize);
    if (file.eof())
    {
        file.clear();
    }
    if (file.fail())
    {
        std::cerr << "Error: Writing to file failed.\n";
    }
    filledBlockIndex = file.gcount();
}

void BlockInputOutput::readBlockAt(int blockOffset)
{
    file.seekg(blockOffset * blockSize);
}

void BlockInputOutput::writeBlockAt(int blockOffset)
{
    file.seekp(blockOffset * blockSize);
}

void BlockInputOutput::writeBlockAtEnd()
{
    file.seekp(0, std::ios::end);
}

BlockInputOutput::BlockInputOutput(std::string filename, int blockSize)
    : blockWrites(0), blockReads(0),
      blockIndex(0), filledBlockIndex(0), blockSize(blockSize), currentBlockIndex(-1)
{
    file.open(filename, std::ios::out | std::ios::in | std::ios::binary | std::ios::app);
    if (!file.is_open() || !file.good())
    {
        throw std::runtime_error("Could not open file " + filename);
    }
    file.close();
    file.open(filename, std::ios::out | std::ios::in | std::ios::binary);

    if (!file.is_open() || !file.good())
    {
        throw std::runtime_error("Could not open file " + filename);
    }

    this->filename = filename;
    this->block = std::make_unique<char[]>(blockSize);
}

BlockInputOutput::~BlockInputOutput()
{
    finish();
}

void BlockInputOutput::finish()
{
    if (file.is_open())
    {
        if (modifiedBlock)
        {
            writeBlock();
        }
        file.flush();
        file.close();
    }
    isFinished = true;
}

std::fstream *BlockInputOutput::getRawFileStreamPtr()
{
    return &file;
}
