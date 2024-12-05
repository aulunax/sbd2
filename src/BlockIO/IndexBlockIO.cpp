#include "IndexBlockIO.h"
#include <cstring>

int IndexBlockIO::allIndexBlockWrites = 0;
int IndexBlockIO::allIndexBlockReads = 0;

int IndexBlockIO::readPageAt(int offset, BtreePage &page)
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

    BtreePage retObj = BtreePage::deserialize(block);
    page = std::move(retObj);

    return BLOCK_OPERATION_SUCCESSFUL;
}

int IndexBlockIO::writePageAt(int offset, const BtreePage &page)
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

    std::unique_ptr<char[]> serializedPage;
    int bytes = page.serialize(serializedPage);
    if (bytes != BTREE_PAGE_MAX_SIZE_IN_BYTES)
    {
        std::cout << "Error: Page wasn't correctly serialized\n";
        return BLOCK_OPERATION_FAILED;
    }

    memcpy(block.get() + blockIndex, serializedPage.get(), bytes);

    return BLOCK_OPERATION_SUCCESSFUL;
}

IndexBlockIO::IndexBlockIO(std::string filename)
    : BlockInputOutput(filename, BTREE_PAGE_MAX_SIZE_IN_BYTES)
{
}

IndexBlockIO::~IndexBlockIO()
{
    if (modifiedBlock)
    {
        allIndexBlockWrites++;
    }
}
