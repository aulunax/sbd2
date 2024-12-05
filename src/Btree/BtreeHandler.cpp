#include "BtreeHandler.h"

BtreeHandler::BtreeHandler(std::string indexFilename, std::string dataFilename)
{
    dataFile = std::make_unique<RecordBlockIO>(dataFilename);
    indexFile = std::make_unique<IndexBlockIO>(indexFilename);
}

void BtreeHandler::forceFlush()
{
    dataFile->flush();
    indexFile->flush();
}

std::optional<Record> BtreeHandler::searchRecord(int key)
{
    if (rootPagePtr == NULL_DATA)
    {
        return std::nullopt;
    }

    int currentPagePtr = rootPagePtr;
    BtreePage currentPage;

    while (currentPagePtr != NULL_DATA)
    {
        int status = indexFile->readPageAt(currentPagePtr, currentPage);
        if (status == BLOCK_OPERATION_FAILED)
        {
            throw std::runtime_error("Error: Could not read page at offset " + std::to_string(rootPagePtr));
        }

        std::pair<int, bool> bisectionResult;
        bisectionResult = bisectionSearchForKey(currentPage, key);
        if (bisectionResult.second)
        {
            int recordOffset =
                currentPage.getNode(bisectionResult.first).recordOffset;

            Record result = fetchRecord(recordOffset);
            result.key = key;
            return result;
        }
        else
        {
            currentPagePtr = currentPage.getNode(bisectionResult.first).pagePtr;
        }
    }

    return std::nullopt;
}

void BtreeHandler::insertRecord(const Record &record)
{
    if (rootPagePtr == NULL_DATA)
    {
        dataFile->writeRecordAt(dataLastRecordOffset, record);
        BtreePage rootPageStructure(NULL_DATA,
                                    {BtreeNode(NULL_DATA, NULL_DATA, NULL_DATA),
                                     BtreeNode(NULL_DATA, record.key, dataLastRecordOffset)});
        dataLastRecordOffset++;
        indexFile->writePageAt(indexLastPageOffset, rootPageStructure);
        indexLastPageOffset++;
        rootPagePtr = 0;
    }
}

std::pair<int, bool> BtreeHandler::bisectionSearchForKey(const BtreePage &page, int key)
{
    int left = 0, right = page.getRecordsOnPageCount() - 1, mid;

    while (left <= right)
    {
        mid = left + (right - left) / 2;
        if (page.getKey(mid) == key)
        {
            return {mid + 1, true};
        }
        else if (page.getKey(mid) < key)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    return {left, false};
}

Record BtreeHandler::fetchRecord(int offset)
{
    Record record;

    int status = dataFile->readRecordAt(offset, record);
    if (status == BLOCK_OPERATION_FAILED)
    {
        throw std::runtime_error("Error: Could not read record at offset " + std::to_string(offset));
    }
    return record;
}

int BtreeHandler::getRootPageOffset()
{
    return rootPagePtr;
}