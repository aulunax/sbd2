#include "RecordBlockIO.h"

#include <cstring>

int RecordBlockIO::allRecordBlockWrites = 0;
int RecordBlockIO::allRecordBlockReads = 0;

Record RecordBlockIO::readRecordAt(int offset)
{
    int newBlockIndex = offset / RECORD_BLOCK_COUNT;
    blockIndex = (offset % RECORD_BLOCK_COUNT) * RECORD_SIZE_IN_BYTES;

    if (newBlockIndex != currentBlockIndex)
    {
        currentBlockIndex = newBlockIndex;
        readBlockAt(currentBlockIndex);
        readBlock();
    }

    if (filledBlockIndex <= blockIndex)
    {
        throw std::runtime_error("Block index is beyond the block");
    }

    Record record;

    record.key = *reinterpret_cast<int *>(block.get() + blockIndex);
    blockIndex += sizeof(int);

    for (int i = 0; i < RECORD_INT_COUNT; i++)
    {
        record.arr[i] = *reinterpret_cast<int *>(block.get() + blockIndex);
        blockIndex += sizeof(int);
    }

    return record;
}

void RecordBlockIO::writeRecordAt(int offset, const Record &record)
{
    int newBlockIndex = offset / RECORD_BLOCK_COUNT;
    blockIndex = (offset % RECORD_BLOCK_COUNT) * RECORD_SIZE_IN_BYTES;

    if (newBlockIndex != currentBlockIndex)
    {
        currentBlockIndex = newBlockIndex;
        readBlockAt(currentBlockIndex);
        readBlock();
    }

    writeBlockAt(currentBlockIndex);

    bool isBlockFilled = true;
    if (filledBlockIndex <= blockIndex)
    {
        isBlockFilled = false;
    }

    memcpy(block.get() + blockIndex, &record.key, sizeof(int));
    blockIndex += sizeof(int);

    for (int i = 0; i < RECORD_INT_COUNT; i++)
    {
        memcpy(block.get() + blockIndex, &record.arr[i], sizeof(int));
        blockIndex += sizeof(int);
    }

    // if block is not filled, fill the rest of it with NULL_DATA
    if (!isBlockFilled)
    {
        for (int i = blockIndex; i < RECORD_BLOCK_SIZE; i++)
        {
            block[i] = NULL_DATA;
        }
    }

    writeBlock();
}

RecordBlockIO::RecordBlockIO(std::string filename)
    : BlockInputOutput(filename, RECORD_BLOCK_SIZE)
{
    block = std::make_unique<char[]>(blockSize);
}