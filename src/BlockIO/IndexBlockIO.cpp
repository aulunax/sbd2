#include "IndexBlockIO.h"
#include <cstring>

int IndexBlockIO::allIndexBlockWrites = 0;
int IndexBlockIO::allIndexBlockReads = 0;

void IndexBlockIO::writeBlock()
{
    BlockInputOutput::writeBlock();
    allIndexBlockWrites++;
}

void IndexBlockIO::readBlock()
{
    BlockInputOutput::readBlock();
    allIndexBlockReads++;
}

int IndexBlockIO::readPageAt(int offset, BtreePage &page)
{
    int newBlockIndex = offset;
    blockIndex = 0;

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
    retObj.setThisPageOffset(offset);
    page = std::move(retObj);

    return BLOCK_OPERATION_SUCCESSFUL;
}

int IndexBlockIO::writePageAt(int offset, const BtreePage &page)
{
    int newBlockIndex = offset;
    blockIndex = 0;

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

    modifiedBlock = true;

    std::unique_ptr<char[]> serializedPage;
    int bytes = page.serialize(serializedPage);
    if (bytes != BTREE_PAGE_MAX_SIZE_IN_BYTES)
    {
        std::cout << "Error: Page wasn't correctly serialized\n";
        return BLOCK_OPERATION_FAILED;
    }

    memcpy(block.get() + blockIndex, serializedPage.get(), bytes);

    if (blockIndex + bytes > filledBlockIndex)
    {
        filledBlockIndex = blockIndex + bytes;
    }

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
