#include "defines.h"
#include "BlockInputOutput.h"
#include "BtreePage.h"

class IndexBlockIO : public BlockInputOutput
{
    static int allIndexBlockWrites;
    static int allIndexBlockReads;

    // void writeBlock() override;
    // void readBlock() override;

public:
    int readPageAt(int offset, BtreePage &page);
    int writePageAt(int offset, const BtreePage &page);

    static int getAllIndexBlockWrites()
    {
        return allIndexBlockWrites;
    }

    static int getAllIndexBlockReads()
    {
        return allIndexBlockReads;
    }

    IndexBlockIO(std::string filename);
    ~IndexBlockIO();
};