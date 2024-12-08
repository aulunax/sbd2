#include "Record.h"
#include "RecordBlockIO.h"
#include "IndexBlockIO.h"
#include "BtreeBuffer.h"
#include <memory>
#include <optional>

class BtreeHandler
{
    std::unique_ptr<RecordBlockIO> dataFile;
    std::unique_ptr<IndexBlockIO> indexFile;

    // toggles
    bool doCompensation = true;

    // important
    BtreeBuffer pageBuffer;
    BtreeNode currentNode;
    int currentPagePtr;
    BtreePage currentPage;

    int cacheHits = 0;
    int cacheMisses = 0;

    int printRecordCount = 0;
    int printPagesCount = 0;
    int lastKey = -1;

    int rootPagePtr = NULL_DATA;
    int indexLastPageOffset = 0;
    int dataLastRecordOffset = 0;

    int height = 0;

    Record fetchRecord(int offset);

    bool compensation(BtreePage page, BtreeNode node);
    void split(BtreePage page, BtreeNode node);

    void readPage(int offset, BtreePage &page, bool lowPriority = false);
    void writePage(int offset, BtreePage &page, bool lowPriority = false);
    void insertNode(BtreeNode node);

    void printPage(BtreePage page, bool moreInfo = false, bool groupPages = false);
    void setHeight(int height);

public:
    int getRootPageOffset();

    void setDoCompensation(bool doCompensation)
    {
        this->doCompensation = doCompensation;
    }

    BtreeHandler(std::string indexFilename, std::string dataFilename);

    void forceFlush();
    OptionalRecord searchRecord(int key);
    void insertRecord(const Record &record);

    void printAllRecords(bool moreInfo = false, bool groupPages = false);
    void printRecordsInPages(bool moreInfo = false);
    void printCacheStats()
    {
        std::cout << "Cache hits: " << cacheHits
                  << "\nCache misses: " << cacheMisses << "\n";
    }
};