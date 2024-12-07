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

    // important
    BtreeBuffer pageBuffer;
    BtreeNode currentNode;
    int currentPagePtr;
    BtreePage currentPage;

    int printRecordCount = 0;
    int printPagesCount = 0;
    int lastKey = -1;

    int rootPagePtr = NULL_DATA;
    int indexLastPageOffset = 0;
    int dataLastRecordOffset = 0;

    int height = 0;

    Record fetchRecord(int offset);

    bool compensation(BtreePage &page, Record record);
    void split(BtreePage &page, BtreeNode node);

    void readPage(int offset, BtreePage &page);
    void writePage(int offset, BtreePage &page);
    void insertNode(BtreeNode node);

    void printPage(BtreePage page, bool moreInfo = false, bool groupPages = false);
    void setHeight(int height);

public:
    int getRootPageOffset();

    BtreeHandler(std::string indexFilename, std::string dataFilename);

    void forceFlush();
    OptionalRecord searchRecord(int key);
    void insertRecord(const Record &record);

    void printAllRecords(bool moreInfo = false, bool groupPages = false);
    void printRecordsInPages(bool moreInfo = false);
};