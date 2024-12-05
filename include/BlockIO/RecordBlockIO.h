#include "BlockInputOutput.h"
#include "Record.h"
#include "defines.h"

#include <iostream>
#include <memory>

class RecordBlockIO : public BlockInputOutput
{
    static int allRecordBlockWrites;
    static int allRecordBlockReads;

    void writeBlock() override;
    void readBlock() override;

public:
    int readRecordAt(int offset, Record &record);
    int writeRecordAt(int offset, const Record &record);
    int writeRecordAtEnd(const Record &record);

    static int getAllRecordBlockWrites()
    {
        return allRecordBlockWrites;
    }

    static int getAllRecordBlockReads()
    {
        return allRecordBlockReads;
    }

    RecordBlockIO(std::string filename);
    ~RecordBlockIO();
};