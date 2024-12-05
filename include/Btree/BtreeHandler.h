#include "Record.h"
#include "BlockIO/RecordBlockIO.h"
#include "BlockIO/IndexBlockIO.h"
#include <memory>

class BtreeHandler
{
    std::unique_ptr<RecordBlockIO> dataFile;
    std::unique_ptr<IndexBlockIO> indexFile;

    int getRootPageOffset();

public:
    BtreeHandler(std::string indexFilename, std::string dataFilename);

    void insertRecord(const Record &record);
    void readRecord(int key);
};