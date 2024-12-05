#include "IndexBlockIO.h"

int IndexBlockIO::allIndexBlockWrites = 0;
int IndexBlockIO::allIndexBlockReads = 0;

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
