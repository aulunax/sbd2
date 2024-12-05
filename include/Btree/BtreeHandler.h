#include "Record.h"
#include "RecordBlockIO.h"
#include "IndexBlockIO.h"
#include <memory>

class BtreeHandler
{
    std::unique_ptr<RecordBlockIO> dataFile;
    std::unique_ptr<IndexBlockIO> indexFile;

    int getRootPageOffset();
    void createRootPtr();

public:
    BtreeHandler(std::string indexFilename, std::string dataFilename);

    void insertRecord(const Record &record);
    void readRecord(int key);
};