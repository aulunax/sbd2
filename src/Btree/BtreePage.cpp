#include "BtreePage.h"
#include "defines.h"
#include <cstring>

BtreePage::BtreePage(int parentOffset, std::vector<BtreeNode> nodes)
{
    this->parentOffset = parentOffset;
    this->nodes = nodes;
}

void BtreePage::addNode(BtreeNode node)
{
    if (nodes.size() == BTREE_MAX_CHILDREN + 1)
    {
        std::cout << "Error: Page is full, cannot add more nodes\n";
        return;
    }
    nodes.push_back(node);
}

void BtreePage::insertNode(BtreeNode node)
{
    std::pair<int, bool> result = bisectionSearchForKey(node.key);
    if (result.second)
    {
        throw std::invalid_argument("This should never happen");
        return;
    }

    nodes.insert(nodes.begin() + result.first + 1, node);
}

void BtreePage::clearNodes()
{
    nodes.clear();
}

int BtreePage::getRecordsOnPageCount() const
{
    return nodes.size() - 1;
}

int BtreePage::getParentOffset() const
{
    return parentOffset;
}

void BtreePage::setParentOffset(int parentOffset)
{
    this->parentOffset = parentOffset;
}

BtreeNode BtreePage::getNode(int index) const
{
    if (index < nodes.size())
    {
        return nodes[index];
    }
    throw std::invalid_argument("Index out of bounds");
}

void BtreePage::setNode(int index, BtreeNode &node)
{
    if (index < nodes.size())
    {
        nodes[index] = node;
        return;
    }
    throw std::invalid_argument("Index out of bounds");
}

int BtreePage::getKey(int index) const
{
    if (index + 1 < nodes.size())
    {
        return nodes[index + 1].key;
    }
    throw std::invalid_argument("Index out of bounds");
}

void BtreePage::setKey(int index, int value)
{
    if (index + 1 < nodes.size())
    {
        nodes[index + 1].key = value;
        return;
    }
    throw std::invalid_argument("Index out of bounds");
}

void BtreePage::setPtr(int index, int value)
{
    if (index < nodes.size())
    {
        nodes[index].pagePtr = value;
        return;
    }
    throw std::invalid_argument("Index out of bounds");
}

int BtreePage::getPtr(int index) const
{
    if (index < nodes.size())
    {
        return nodes[index].pagePtr;
    }
    throw std::invalid_argument("Index out of bounds");
}

int BtreePage::getRecordOffset(int index) const
{
    if (index + 1 < nodes.size())
    {
        return nodes[index + 1].recordOffset;
    }
    throw std::invalid_argument("Index out of bounds");
}

void BtreePage::setRecordOffset(int index, int value)
{
    if (index + 1 < nodes.size())
    {
        nodes[index + 1].recordOffset = value;
        return;
    }
    throw std::invalid_argument("Index out of bounds");
}

bool BtreePage::isRoot()
{
    if (parentOffset == NULL_DATA)
    {
        return true;
    }
    return false;
}

BtreePage BtreePage::deserialize(const std::unique_ptr<char[]> &serializedPage)
{
    BtreePage retObj;
    int serialIndex = 0;

    retObj.parentOffset = *reinterpret_cast<int *>(serializedPage.get() + serialIndex);
    serialIndex += sizeof(int);

    int recordsCount = *reinterpret_cast<int *>(serializedPage.get() + serialIndex);
    serialIndex += sizeof(int);

    int p0 = *reinterpret_cast<int *>(serializedPage.get() + serialIndex);
    serialIndex += sizeof(int);

    retObj.addNode(BtreeNode(p0, NULL_DATA, NULL_DATA));

    for (int i = 0; i < recordsCount; i++)
    {
        int key = *reinterpret_cast<int *>(serializedPage.get() + serialIndex);
        serialIndex += sizeof(int);
        int recordOffset = *reinterpret_cast<int *>(serializedPage.get() + serialIndex);
        serialIndex += sizeof(int);
        int pagePtr = *reinterpret_cast<int *>(serializedPage.get() + serialIndex);
        serialIndex += sizeof(int);

        retObj.addNode(BtreeNode(pagePtr, key, recordOffset));
    }

    return retObj;
}

int BtreePage::serialize(std::unique_ptr<char[]> &serializedPage) const
{
    serializedPage = std::make_unique<char[]>(BTREE_PAGE_MAX_SIZE_IN_BYTES);

    int serialIndex = 0;

    memcpy(serializedPage.get() + serialIndex, &parentOffset, sizeof(int));
    serialIndex += sizeof(int);

    int recordsCount = getRecordsOnPageCount();
    memcpy(serializedPage.get() + serialIndex, &recordsCount, sizeof(int));
    serialIndex += sizeof(int);

    // first pointer
    memcpy(serializedPage.get() + serialIndex, &nodes[0].pagePtr, sizeof(int));
    serialIndex += sizeof(int);

    // nodes
    for (int i = 0; i < getRecordsOnPageCount(); i++)
    {
        memcpy(serializedPage.get() + serialIndex, &nodes[i + 1].key, sizeof(int));
        serialIndex += sizeof(int);
        memcpy(serializedPage.get() + serialIndex, &nodes[i + 1].recordOffset, sizeof(int));
        serialIndex += sizeof(int);
        memcpy(serializedPage.get() + serialIndex, &nodes[i + 1].pagePtr, sizeof(int));
        serialIndex += sizeof(int);
    }

    int nullData = NULL_DATA;
    while (serialIndex < BTREE_PAGE_MAX_SIZE_IN_BYTES)
    {
        memcpy(serializedPage.get() + serialIndex, &nullData, sizeof(int));
        serialIndex += sizeof(int);
    }

    return serialIndex;
}

std::pair<int, bool> BtreePage::bisectionSearchForKey(int key)
{
    int left = 0, right = getRecordsOnPageCount() - 1, mid;

    while (left <= right)
    {
        mid = left + (right - left) / 2;
        if (getKey(mid) == key)
        {
            return {mid + 1, true};
        }
        else if (getKey(mid) < key)
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

std::vector<BtreeNode> &BtreePage::getNodes()
{
    return nodes;
}
