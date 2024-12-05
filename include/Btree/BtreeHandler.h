#include "Record.h"
#include "RecordBlockIO.h"
#include "IndexBlockIO.h"
#include <memory>
#include <optional>

class BtreeHandler
{
    std::unique_ptr<RecordBlockIO> dataFile;
    std::unique_ptr<IndexBlockIO> indexFile;

    int rootPagePtr = NULL_DATA;
    int indexLastPageOffset = 0;
    int dataLastRecordOffset = 0;

    std::pair<int, bool> bisectionSearchForKey(const BtreePage &page, int key);

    Record fetchRecord(int offset);

public:
    int getRootPageOffset();

    BtreeHandler(std::string indexFilename, std::string dataFilename);

    void forceFlush();
    std::optional<Record> searchRecord(int key);
    void insertRecord(const Record &record);
};