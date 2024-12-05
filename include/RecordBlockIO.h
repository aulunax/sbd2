#include "BlockInputOutput.h"
#include "Record.h"
#include "defines.h"

#include <iostream>
#include <memory>

class RecordBlockIO : public BlockInputOutput
{
    static int allRecordBlockWrites;
    static int allRecordBlockReads;

public:
    Record readRecordAt(int offset);
    void writeRecordAt(int offset, const Record &record);

    static int getAllRecordBlockWrites()
    {
        return allRecordBlockWrites;
    }

    static int getAllRecordBlockReads()
    {
        return allRecordBlockReads;
    }

    RecordBlockIO(std::string filename);
};