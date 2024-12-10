#include "BtreeHandler.h"
#include <algorithm>

BtreeHandler::BtreeHandler(std::string indexFilename, std::string dataFilename)
{
    BlockInputOutput::resetAllBlockStats();
    RecordBlockIO::resetAllBlockStats();
    IndexBlockIO::resetAllBlockStats();
    dataFile = std::make_unique<RecordBlockIO>(dataFilename);
    indexFile = std::make_unique<IndexBlockIO>(indexFilename);
}

void BtreeHandler::forceFlush()
{
    pageBuffer.flush(indexFile.get());
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

    int lastPagePtr = NULL_DATA;

    while (nextPagePtr != NULL_DATA)
    {
        currentPagePtr = nextPagePtr;
        readPage(currentPagePtr, currentPage);

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
        lastPagePtr = currentPagePtr;
    }

    return std::nullopt;
}

void BtreeHandler::insertRecord(const Record &record)
{
    // if first record
    if (rootPagePtr == NULL_DATA)
    {
        indexLastPageOffset = 0;
        dataLastRecordOffset = 0;
        setHeight(height + 1);
        dataFile->writeRecordAt(dataLastRecordOffset, record);
        BtreePage rootPageStructure(NULL_DATA,
                                    {BtreeNode(NULL_DATA, NULL_DATA, NULL_DATA),
                                     BtreeNode(NULL_DATA, record.key, dataLastRecordOffset)});
        dataLastRecordOffset++;
        writePage(indexLastPageOffset, rootPageStructure);
        rootPagePtr = indexLastPageOffset;
        indexLastPageOffset++;
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

    insertNode(currentNode);
}

void BtreeHandler::updateRecord(int key, const Record &record)
{

    OptionalRecord searchResult = searchRecord(key);
    if (searchResult == std::nullopt)
    {
        std::cout << "Aborting update. Record with key " << key << " doesn't exist in database\n";
        return;
    }

    if (record.key != key)
    {
        OptionalRecord searchResult = searchRecord(record.key);
        if (searchResult != std::nullopt)
        {
            std::cout << "Aborting update. Record with key " << record.key << " already exists in database\n";
            return;
        }
        deleteRecord(key);
        insertRecord(record);
    }
    else
    {
        int recordOffset = currentPage.bisectionSearchForKey(key).first - 1;
        dataFile->writeRecordAt(currentPage.getRecordOffset(recordOffset), record);
    }
}

void BtreeHandler::deleteRecord(int key)
{
    OptionalRecord searchResult = searchRecord(key);
    if (searchResult == std::nullopt)
    {
        std::cout << "Aborting delete. Record with key " << key << " doesn't exist in database\n";
        return;
    }

    // after this operation:
    // currentNode contains the deleted node from leaf page
    // currentPage contains the page the node was deleted from
    replaceKeyWithLeaf(currentPage, key);

    // if the current page is root and has no records, remove it
    if (currentPage.getThisPageOffset() == rootPagePtr && currentPage.getRecordsOnPageCount() == 0)
    {
        setHeight(height - 1);
        rootPagePtr = NULL_DATA;
        return;
    }

    deleteNode(currentNode);
}

void BtreeHandler::deleteNode(BtreeNode node)
{
    // if simple deletion
    if (currentPage.getRecordsOnPageCount() >= BTREE_D_FACTOR ||
        (currentPage.getThisPageOffset() == rootPagePtr && currentPage.getRecordsOnPageCount() >= 1))
        return;

    if (doCompensation && compensation(currentPage, node, UNDERFLOW))
    {
        return;
    }

    // merge calls deleteNode on parent of currentPage so recurrsive call happens
    merge(currentPage, node);
}

void BtreeHandler::replaceKeyWithLeaf(BtreePage page, int key)
{
    int keyIndexToReplace = page.bisectionSearchForKey(key).first;
    BtreeNode leftSubtreeNode = page.getNode(keyIndexToReplace - 1); // node with ptr to left subtree
    currentNode = page.getNode(keyIndexToReplace);                   // node to delete

    Record NullRecord;
    NullRecord.key = NULL_DATA;
    NullRecord.fill(NULL_DATA);

    // if already in leaf:
    if (leftSubtreeNode.pagePtr == NULL_DATA)
    {
        currentPage.removeNode(keyIndexToReplace);
        writePage(currentPage.getThisPageOffset(), currentPage);
        dataFile->writeRecordAt(currentNode.recordOffset, NullRecord);
        return;
    }

    // go to the largest key in left subtree
    while (leftSubtreeNode.pagePtr != NULL_DATA)
    {
        readPage(leftSubtreeNode.pagePtr, currentPage);
        leftSubtreeNode = currentPage.getNode(currentPage.getRecordsOnPageCount());
    }

    // write null to data file at records offset
    dataFile->writeRecordAt(currentNode.recordOffset, NullRecord);

    // replace key with largest key in left subtree
    page.setKey(keyIndexToReplace - 1, leftSubtreeNode.key);
    page.setRecordOffset(keyIndexToReplace - 1, leftSubtreeNode.recordOffset);

    // delete the largest key in left subtree
    currentPage.removeNode(currentPage.getRecordsOnPageCount());

    // write changes to pages
    writePage(page.getThisPageOffset(), page);
    writePage(currentPage.getThisPageOffset(), currentPage);
}

void BtreeHandler::merge(BtreePage page, BtreeNode node)
{
    bool isRoot = (page.getParentOffset() == NULL_DATA);

    // handle special case when root has become empty
    if (isRoot && page.getRecordsOnPageCount() == 0)
    {
        readPage(page.getNode(0).pagePtr, page);
        page.setParentOffset(NULL_DATA);
        writePage(page.getThisPageOffset(), page);
        rootPagePtr = page.getThisPageOffset();
        setHeight(height - 1);
        return;
    }

    BtreePage parent;
    BtreePage sibling;

    readPage(page.getParentOffset(), parent);

    // get the parent node that points to the page
    std::pair<int, bool> result = parent.bisectionSearchForPtr(page.getThisPageOffset());

    BtreePage *lowerPage = &sibling;
    BtreePage *higherPage = &page;
    int side = LEFT_SIBLING;
    int pagePtrPosition = -1;

    // read left sibling if possible
    if (result.first != 0)
    {
        readPage(parent.getPtr(result.first - 1), sibling, true);
        pagePtrPosition = result.first;
    }

    // read right sibling if posssible and necessary
    if (pagePtrPosition == -1 &&
        result.first != parent.getRecordsOnPageCount())
    {
        readPage(parent.getPtr(result.first + 1), sibling, true);
        pagePtrPosition = result.first + 1;
        side = RIGHT_SIBLING;
        lowerPage = &page;
        higherPage = &sibling;
    }

    // sort nodes from both pages
    std::vector<BtreeNode> sortedNodes;
    for (int i = 1; i <= lowerPage->getRecordsOnPageCount(); i++)
        sortedNodes.push_back(lowerPage->getNode(i));

    for (int i = 1; i <= higherPage->getRecordsOnPageCount(); i++)
        sortedNodes.push_back(higherPage->getNode(i));

    // add parent node to sorting
    if (side == LEFT_SIBLING)
    {
        sortedNodes.push_back(parent.getNode(result.first));
        parent.removeNode(result.first);
    }
    else if (side == RIGHT_SIBLING)
    {
        sortedNodes.push_back(parent.getNode(result.first + 1));
        parent.removeNode(result.first + 1);
    }

    // set ptr of node taken from parent to the leftmost ptr of the right page
    sortedNodes.back().pagePtr = higherPage->getNode(0).pagePtr;

    std::sort(sortedNodes.begin(), sortedNodes.end(), [](BtreeNode a, BtreeNode b)
              { return a.key < b.key; });

    // manually set the first node (ptr) of lower page
    BtreeNode firstNode;
    firstNode = lowerPage->getNode(0);
    lowerPage->clearNodes();
    lowerPage->addNode(firstNode);
    for (int i = 0; i < sortedNodes.size(); i++)
    {
        lowerPage->addNode(sortedNodes[i]);
    }

    // write changes to pages
    // (higher page is deleted, lower page is updated)
    writePage(higherPage->getThisPageOffset(), *higherPage, true);
    writePage(lowerPage->getThisPageOffset(), *lowerPage);
    writePage(parent.getThisPageOffset(), parent);

    updateParentPtrOfChildren(*lowerPage);

    // recursively call merge on parent (currentPage is global with respect to this class instance)
    currentPage = parent;
    deleteNode(node);
}

void BtreeHandler::insertNode(BtreeNode node)
{
    // if m < 2d
    if (currentPage.getRecordsOnPageCount() < 2 * BTREE_D_FACTOR)
    {
        currentPage.insertNode(node);
        writePage(currentPagePtr, currentPage);
        return;
    }

    // if overflow
    if (doCompensation && compensation(currentPage, node, OVERFLOW))
    {
        return;
    }

    // split recursively calls insertNode
    split(currentPage, node);
}

bool BtreeHandler::compensation(BtreePage page, BtreeNode node, bool overflow)
{
    // cant do compensation on root
    if (page.getParentOffset() == NULL_DATA)
        return false;

    BtreePage parent;
    readPage(page.getParentOffset(), parent);

    std::pair<int, bool> result;

    // get the parent node that points to the page (why did i do this to myself)
    if (overflow == OVERFLOW)
        result = parent.bisectionSearchForKey(node.key);
    else
        result = parent.bisectionSearchForPtr(page.getThisPageOffset());

    BtreePage sibling;

    // read left sibling if possible
    int parentMidNode = -1;
    if (result.first != 0)
    {
        readPage(parent.getPtr(result.first - 1), sibling, true);
        parentMidNode = result.first;
    }

    // read right sibling if posssible and necessary
    if ((((overflow == OVERFLOW && sibling.getRecordsOnPageCount() == 2 * BTREE_D_FACTOR) ||
          (overflow == UNDERFLOW && sibling.getRecordsOnPageCount() == BTREE_D_FACTOR)) ||
         parentMidNode == -1) &&
        result.first != parent.getRecordsOnPageCount())
    {
        readPage(parent.getPtr(result.first + 1), sibling, true);
        parentMidNode = result.first + 1;
    }

    if ((overflow == OVERFLOW && sibling.getRecordsOnPageCount() < 2 * BTREE_D_FACTOR) ||
        overflow == UNDERFLOW && sibling.getRecordsOnPageCount() > BTREE_D_FACTOR)
    {
        // sort nodes from sibling and page
        std::vector<BtreeNode> sortedNodes;
        for (int i = 1; i <= page.getRecordsOnPageCount(); i++)
            sortedNodes.push_back(page.getNode(i));

        for (int i = 1; i <= sibling.getRecordsOnPageCount(); i++)
            sortedNodes.push_back(sibling.getNode(i));

        // add parent node to sorting
        BtreeNode parentNode = parent.getNode(parentMidNode);
        parentNode.pagePtr = page.getKey(0) < sibling.getKey(0) ? sibling.getPtr(0) : page.getPtr(0);
        sortedNodes.push_back(parentNode);

        // if overflow, add the node to be inserted to sorting
        if (overflow == OVERFLOW)
            sortedNodes.push_back(node);

        std::sort(sortedNodes.begin(), sortedNodes.end(), [](BtreeNode a, BtreeNode b)
                  { return a.key < b.key; });

        int mid = sortedNodes.size() / 2;

        // figure out which page has lower value keys and which has higher
        BtreePage *lowerPage = (page.getKey(0) > sibling.getKey(0)) ? &sibling : &page;
        BtreePage *higherPage = (page.getKey(0) > sibling.getKey(0)) ? &page : &sibling;

        // manually set the first node (ptr) of lower page
        BtreeNode firstNode = lowerPage->getNode(0);
        lowerPage->clearNodes();
        lowerPage->addNode(firstNode);

        for (int i = 0; i < mid; i++)
            lowerPage->addNode(sortedNodes[i]);

        // set parent record to the mid element
        parent.setKey(parentMidNode - 1, sortedNodes[mid].key);
        parent.setRecordOffset(parentMidNode - 1, sortedNodes[mid].recordOffset);

        // manually set the first node (ptr) of higher page, update its ptr
        firstNode = higherPage->getNode(0);
        firstNode.pagePtr = sortedNodes[mid].pagePtr;
        higherPage->clearNodes();
        higherPage->addNode(firstNode);

        for (int i = mid + 1; i < sortedNodes.size(); i++)
            higherPage->addNode(sortedNodes[i]);

        // write changes to pages
        writePage(sibling.getThisPageOffset(), sibling, true);
        writePage(page.getThisPageOffset(), page);
        writePage(parent.getThisPageOffset(), parent);

        // update parentPtr of children of the sibling pages
        updateParentPtrOfChildren(page);
        updateParentPtrOfChildren(sibling);
    }
    else
        return false;
    return true;
}

void BtreeHandler::split(BtreePage page, BtreeNode node)
{
    bool isRoot = (page.getParentOffset() == NULL_DATA);
    int nodeIndex = page.bisectionSearchForKey(node.key).first;

    // sort nodes from page and add the new node
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

    if (isRoot)
    {
        setHeight(height + 1);
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

        // fixing ptrs
        newRootPage.getNodes()[1].pagePtr = newPage.getThisPageOffset();
        page.setParentOffset(newRootPage.getThisPageOffset());

        // changing root
        rootPagePtr = newRootPage.getThisPageOffset();

        writePage(page.getThisPageOffset(), page);
        writePage(newRootPage.getThisPageOffset(), newRootPage);
        writePage(newPage.getThisPageOffset(), newPage, true);

        updateParentPtrOfChildren(newPage);
    }
    else
    {
        // create new sibling page
        BtreePage newPage(page.getParentOffset(),
                          {BtreeNode(sortedNodes[mid].pagePtr, NULL_DATA, NULL_DATA)});
        for (int i = mid + 1; i < sortedNodes.size(); i++)
        {
            newPage.addNode(sortedNodes[i]);
        }
        newPage.setThisPageOffset(indexLastPageOffset);
        indexLastPageOffset++;

        // read parent page to current page, so it can be used as base for the next insertNode()
        currentPagePtr = page.getParentOffset();
        readPage(currentPagePtr, currentPage);

        writePage(page.getThisPageOffset(), page);
        writePage(newPage.getThisPageOffset(), newPage, true);

        // update parent offset of children of new page
        updateParentPtrOfChildren(newPage);

        // recursion
        insertNode(BtreeNode(newPage.getThisPageOffset(), sortedNodes[mid].key, sortedNodes[mid].recordOffset));
    }
}

void BtreeHandler::readPage(int offset, BtreePage &page, bool lowPriority)
{
    std::optional<BtreePage *> result = pageBuffer.getPage(offset, lowPriority);
    if (result != std::nullopt)
    {
        cacheHits++;
        page = *result.value();
        return;
    }
    cacheMisses++;

    int status = indexFile->readPageAt(offset, page);
    if (status == BLOCK_OPERATION_FAILED)
    {
        throw std::runtime_error("Error: Could not read page at offset " + std::to_string(rootPagePtr));
    }

    pageBuffer.pushPage(page, offset, indexFile.get(), false, lowPriority);
}

void BtreeHandler::writePage(int offset, BtreePage &page, bool lowPriority)
{
    std::optional<BtreePage *> result = pageBuffer.getPage(offset, lowPriority);
    if (result != std::nullopt)
    {
        cacheHits++;
        pageBuffer.pushPage(page, offset, indexFile.get(), true, lowPriority);
        return;
    }
    cacheMisses++;

    pageBuffer.pushPage(page, offset, indexFile.get(), true, lowPriority);
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

void BtreeHandler::printAllRecords(bool moreInfo, bool groupPages)
{
    if (rootPagePtr == NULL_DATA)
    {
        std::cout << "No records in database\n";
        return;
    }

    lastKey = -1;
    printRecordCount = 0;
    printPagesCount = 0;
    readPage(rootPagePtr, currentPage, true);
    printPage(currentPage, moreInfo, groupPages);

    std::cout << "-----------------------------------\n";
    std::cout << "Total records: " << printRecordCount << "\n";
    std::cout << "Total pages count: " << printPagesCount << "\n";
    std::cout << "Height: " << height << "\n";
    std::cout << "-----------------------------------\n";
}

void BtreeHandler::printPage(BtreePage page, bool moreInfo, bool groupPages)
{
    std::vector<BtreeNode> nodes = page.getNodes();

    printPagesCount++;

    if (nodes[0].pagePtr != NULL_DATA)
    {
        readPage(nodes[0].pagePtr, currentPage, true);
        printPage(currentPage, moreInfo, groupPages);
    }

    if (groupPages)
    {
        std::cout << "-------- Page " << page.getThisPageOffset() << " ----------" << "\n";
    }

    for (int i = 1; i < nodes.size(); i++)
    {
        Record record = fetchRecord(nodes[i].recordOffset);
        printRecordCount++;
        record.key = nodes[i].key;
        if (!groupPages && record.key <= lastKey)
        {
            std::cout << "Error: Keys are not sorted\n";
            throw std::runtime_error("Keys are not sorted");
        }
        lastKey = record.key;
        if (!moreInfo)
            std::cout << record.toString() << "\n";
        else
        {
            std::cout << "RecordOffset: " << nodes[i].recordOffset
                      << " | PageOffset: " << page.getThisPageOffset()
                      << " | ParentPageOffset: " << page.getParentOffset()
                      << " | LeftPagePtr: " << nodes[i - 1].pagePtr
                      << " | RightPagePtr: " << nodes[i].pagePtr << "\n";
            std::cout << record.toString() << "\n\n";
        }

        if (!groupPages && nodes[i].pagePtr != NULL_DATA)
        {
            readPage(nodes[i].pagePtr, currentPage, true);
            printPage(currentPage, moreInfo, groupPages);
        }
    }

    if (groupPages)
    {
        for (int i = 1; i < nodes.size(); i++)
        {
            if (nodes[i].pagePtr == NULL_DATA)
            {
                continue;
            }
            readPage(nodes[i].pagePtr, currentPage, true);
            printPage(currentPage, moreInfo, groupPages);
        }
    }
}

void BtreeHandler::setHeight(int height)
{
    pageBuffer.setNewHeight(height);
    this->height = height;
}

void BtreeHandler::updateParentPtrOfChildren(BtreePage &page)
{
    BtreePage temp;
    for (int i = 0; i <= page.getRecordsOnPageCount(); i++)
    {
        if (page.getNode(i).pagePtr == NULL_DATA)
            continue;
        readPage(page.getNode(i).pagePtr, temp, true);
        if (temp.getParentOffset() == page.getThisPageOffset())
            continue;
        temp.setParentOffset(page.getThisPageOffset());
        writePage(page.getNode(i).pagePtr, temp, true);
    }
}
