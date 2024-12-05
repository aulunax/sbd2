#include "RecordBlockIO.h"
#include <cstring>

int RecordBlockIO::allRecordBlockWrites = 0;
int RecordBlockIO::allRecordBlockReads = 0;

void RecordBlockIO::writeBlock()
{
    BlockInputOutput::writeBlock();
    allRecordBlockWrites++;
}

void RecordBlockIO::readBlock()
{
    BlockInputOutput::readBlock();
    allRecordBlockReads++;
}

int RecordBlockIO::readRecordAt(int offset, Record &record)
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
        return BLOCK_OPERATION_FAILED;
    }

    for (int i = 0; i < RECORD_INT_COUNT; i++)
    {
        record.arr[i] = *reinterpret_cast<int *>(block.get() + blockIndex);
        blockIndex += sizeof(int);
    }

    return BLOCK_OPERATION_SUCCESSFUL;
}

int RecordBlockIO::writeRecordAt(int offset, const Record &record)
{
    int newBlockIndex = offset / RECORD_BLOCK_COUNT;
    blockIndex = (offset % RECORD_BLOCK_COUNT) * RECORD_SIZE_IN_BYTES;

    if (newBlockIndex != currentBlockIndex)
    {
        if (modifiedBlock)
        {
            writeBlock();
        }
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

    modifiedBlock = true;

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

    return BLOCK_OPERATION_SUCCESSFUL;
}

int RecordBlockIO::writeRecordAtEnd(const Record &record)
{
    // writeBlockAtEnd();
    return BLOCK_OPERATION_SUCCESSFUL;
}

RecordBlockIO::RecordBlockIO(std::string filename)
    : BlockInputOutput(filename, RECORD_BLOCK_SIZE)
{
}

RecordBlockIO::~RecordBlockIO()
{
    if (modifiedBlock)
    {
        allRecordBlockWrites++;
    }
}
