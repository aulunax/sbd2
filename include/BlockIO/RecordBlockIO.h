#include "BlockInputOutput.h"
#include "Record.h"
#include "defines.h"

#include <iostream>
#include <memory>

#ifndef RECORDBLOCKIO_H
#define RECORDBLOCKIO_H

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

    static void resetAllBlockStats()
    {
        allRecordBlockWrites = 0;
        allRecordBlockReads = 0;
    }

    RecordBlockIO(std::string filename);
    ~RecordBlockIO();
};

#endif // RECORDBLOCKIO_H