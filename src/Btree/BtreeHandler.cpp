#include "BtreeHandler.h"
#include <algorithm>

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

OptionalRecord BtreeHandler::searchRecord(int key)
{
    if (rootPagePtr == NULL_DATA)
    {
        return std::nullopt;
    }

    currentPage = BtreePage();
    int nextPagePtr = rootPagePtr;

    while (nextPagePtr != NULL_DATA)
    {
        currentPagePtr = nextPagePtr;
        int status = indexFile->readPageAt(currentPagePtr, currentPage);
        if (status == BLOCK_OPERATION_FAILED)
        {
            throw std::runtime_error("Error: Could not read page at offset " + std::to_string(rootPagePtr));
        }

        std::pair<int, bool> bisectionResult;
        bisectionResult = currentPage.bisectionSearchForKey(key);
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
            nextPagePtr = currentPage.getNode(bisectionResult.first).pagePtr;
        }
    }

    return std::nullopt;
}

void BtreeHandler::insertRecord(const Record &record)
{
    // if first record
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
        return;
    }

    // check if key already exists
    OptionalRecord searchResult = searchRecord(record.key);
    if (searchResult != std::nullopt)
    {
        std::cout << "Aborting insertion. Record with key " << record.key << " already exists in database\n";
        return;
    }

    dataFile->writeRecordAt(dataLastRecordOffset, record);
    dataLastRecordOffset++;

    currentNode = BtreeNode(NULL_DATA, record.key, dataLastRecordOffset - 1);

    // if m < 2d
    if (currentPage.getRecordsOnPageCount() < 2 * BTREE_D_FACTOR)
    {
        currentPage.insertNode(currentNode);
        indexFile->writePageAt(currentPagePtr, currentPage);
        return;
    }

    // overflow
    if (compensation(currentPage, record))
    {
        return;
    }

    split(currentPage, currentNode);
}

bool BtreeHandler::compensation(BtreePage &page, Record record)
{
    if (page.getParentOffset() == NULL_DATA)
        return false;

    BtreePage parent;
    indexFile->readPageAt(page.getParentOffset(), parent);

    std::pair<int, bool> result = parent.bisectionSearchForKey(record.key);
    std::optional<BtreePage> leftSibling = std::nullopt;
    std::optional<BtreePage> rightSibling = std::nullopt;

    if (result.first != 0)
        indexFile->readPageAt(parent.getPtr(result.first - 1), leftSibling.value());
    if (result.first != parent.getRecordsOnPageCount())
        indexFile->readPageAt(parent.getPtr(result.first + 1), rightSibling.value());

    if (leftSibling != std::nullopt && leftSibling.value().getRecordsOnPageCount() < 2 * BTREE_D_FACTOR)
    {
        std::vector<std::pair<int, int>> keyOffsetPair;
        keyOffsetPair.push_back(
            {record.key, dataLastRecordOffset - 1});

        for (int i = 0; i < leftSibling.value().getRecordsOnPageCount(); i++)
        {
            keyOffsetPair.push_back(
                {leftSibling.value().getKey(i), leftSibling.value().getRecordOffset(i)});
        }

        keyOffsetPair.push_back(
            {parent.getKey(result.first - 1),
             parent.getRecordOffset(result.first - 1)});

        for (int i = 0; i < page.getRecordsOnPageCount(); i++)
        {
            keyOffsetPair.push_back({page.getKey(i), page.getRecordOffset(i)});
        }

        std::sort(keyOffsetPair.begin(), keyOffsetPair.end());
        int mid = keyOffsetPair.size() / 2;

        page.clearNodes();
        leftSibling.value().clearNodes();

        for (int i = 0; i < mid; i++)
        {
            leftSibling.value().insertNode(BtreeNode(NULL_DATA, keyOffsetPair[i].first, keyOffsetPair[i].second));
        }
        parent.setKey(result.first - 1, keyOffsetPair[mid].first);
        parent.setRecordOffset(result.first - 1, keyOffsetPair[mid].second);

        for (int i = mid + 1; i < keyOffsetPair.size(); i++)
        {
            page.insertNode(BtreeNode(NULL_DATA, keyOffsetPair[i].first, keyOffsetPair[i].second));
        }

        indexFile->writePageAt(parent.getPtr(result.first + 1), rightSibling.value());
        indexFile->writePageAt(parent.getPtr(result.first), page);
        indexFile->writePageAt(page.getParentOffset(), parent);
    }
    else if (rightSibling != std::nullopt && rightSibling.value().getRecordsOnPageCount() < 2 * BTREE_D_FACTOR)
    {
        std::vector<std::pair<int, int>> keyOffsetPair;
        keyOffsetPair.push_back(
            {record.key, dataLastRecordOffset - 1});

        for (int i = 0; i < page.getRecordsOnPageCount(); i++)
        {
            keyOffsetPair.push_back({page.getKey(i), page.getRecordOffset(i)});
        }

        keyOffsetPair.push_back(
            {parent.getKey(result.first + 1),
             parent.getRecordOffset(result.first + 1)});

        for (int i = 0; i < rightSibling.value().getRecordsOnPageCount(); i++)
        {
            keyOffsetPair.push_back(
                {rightSibling.value().getKey(i), rightSibling.value().getRecordOffset(i)});
        }

        std::sort(keyOffsetPair.begin(), keyOffsetPair.end());
        int mid = keyOffsetPair.size() / 2;

        page.clearNodes();
        rightSibling.value().clearNodes();

        for (int i = 0; i < mid; i++)
        {
            page.insertNode(BtreeNode(NULL_DATA, keyOffsetPair[i].first, keyOffsetPair[i].second));
        }
        parent.setKey(result.first + 1, keyOffsetPair[mid].first);
        parent.setRecordOffset(result.first + 1, keyOffsetPair[mid].second);

        for (int i = mid + 1; i < keyOffsetPair.size(); i++)
        {
            rightSibling.value().insertNode(BtreeNode(NULL_DATA, keyOffsetPair[i].first, keyOffsetPair[i].second));
        }

        indexFile->writePageAt(parent.getPtr(result.first + 1), rightSibling.value());
        indexFile->writePageAt(parent.getPtr(result.first), page);
        indexFile->writePageAt(page.getParentOffset(), parent);
    }
    else
    {
        return false;
    }

    return true;
}

void BtreeHandler::split(BtreePage &page, BtreeNode node)
{
    bool isRoot = (page.getParentOffset() == NULL_DATA);
    int nodeIndex = page.bisectionSearchForKey(node.key).first;

    if (isRoot)
    {
        std::vector<BtreeNode> sortedNodes;
        for (int i = 0; i <= page.getRecordsOnPageCount(); i++)
        {
            sortedNodes.push_back(page.getNode(i));
        }
        sortedNodes.push_back(node);

        std::sort(sortedNodes.begin(), sortedNodes.end(), [](BtreeNode a, BtreeNode b)
                  { return a.key < b.key; });

        int mid = sortedNodes.size() / 2;

        // fill old page
        page.clearNodes();
        for (int i = 0; i < mid; i++)
        {
            page.addNode(sortedNodes[i]);
        }

        // create new root
        // p0 points to the old page (old page is lower)
        // mid element is mid
        // p1 is new sibling
        BtreePage newRootPage(NULL_DATA,
                              {BtreeNode(page.getThisPageOffset(), NULL_DATA, NULL_DATA)});
        newRootPage.addNode(sortedNodes[mid]);
        newRootPage.setThisPageOffset(indexLastPageOffset);
        indexLastPageOffset++;

        // create new sibling page
        // last node ptr of root starts pointing to this
        // the previous last node ptr of root is now p0 of this
        BtreePage newPage(newRootPage.getThisPageOffset(),
                          {BtreeNode(sortedNodes[mid].pagePtr, NULL_DATA, NULL_DATA)});
        for (int i = mid + 1; i < sortedNodes.size(); i++)
        {
            newPage.addNode(sortedNodes[i]);
        }
        newPage.setThisPageOffset(indexLastPageOffset);
        indexLastPageOffset++;

        newRootPage.getNodes()[1].pagePtr = newPage.getThisPageOffset();
        page.setParentOffset(newRootPage.getThisPageOffset());

        rootPagePtr = newRootPage.getThisPageOffset();

        indexFile->writePageAt(page.getThisPageOffset(), page);
        indexFile->writePageAt(newRootPage.getThisPageOffset(), newRootPage);
        indexFile->writePageAt(newPage.getThisPageOffset(), newPage);
    }
    else
    {
        // create new sibling page

        // distribute keys between old page, new page and parent
    }
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