#include "BlockInputOutput.h"

int BlockInputOutput::allBlockWrites = 0;
int BlockInputOutput::allBlockReads = 0;

void BlockInputOutput::writeBlock()
{
    blockWrites++;
    allBlockWrites++;
    file.write(block.get(), blockSize);
    file.flush();
}

void BlockInputOutput::readBlock()
{
    blockReads++;
    allBlockReads++;
    file.read(block.get(), blockSize);
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

BlockInputOutput::BlockInputOutput(std::string filename, int blockSize)
    : blockWrites(0), blockReads(0),
      blockIndex(-1), filledBlockIndex(-1), blockSize(blockSize), currentBlockIndex(-1)
{
    file.open(filename, std::ios::out | std::ios::in | std::ios::binary | std::ios::app);

    if (!file.is_open() || !file.good())
    {
        throw std::runtime_error("Could not open file");
    }

    this->filename = filename;
}

BlockInputOutput::~BlockInputOutput()
{
    finish();
}

void BlockInputOutput::finish()
{
    if (file.is_open())
    {
        file.flush();
        file.close();
    }
    isFinished = true;
}

std::fstream *BlockInputOutput::getRawFileStreamPtr()
{
    return &file;
}
