#include "defines.h"
#include "BtreeNode.h"
#include <iostream>
#include <vector>
#include <memory>

#ifndef BTREEPAGE_H
#define BTREEPAGE_H

class BtreePage
{
    int parentOffset;
    int thisPageOffset;
    std::vector<BtreeNode> nodes;

public:
    BtreePage() : parentOffset(NULL_DATA), thisPageOffset(NULL_DATA) {};
    BtreePage(int parentOffset) : parentOffset(parentOffset) {}
    BtreePage(int parentOffset, std::vector<BtreeNode> nodes);

    void addNode(BtreeNode node);
    void insertNode(BtreeNode node);
    void clearNodes();

    int getRecordsOnPageCount() const;
    int getParentOffset() const;
    int getThisPageOffset() const { return thisPageOffset; }
    std::vector<BtreeNode> &getNodes();

    void setParentOffset(int parentOffset);
    void setThisPageOffset(int thisPageOffset) { this->thisPageOffset = thisPageOffset; }

    std::pair<int, bool> bisectionSearchForKey(int key);

    bool isRoot();

    BtreeNode getNode(int index) const;
    int getKey(int index) const;
    int getPtr(int index) const;
    int getRecordOffset(int index) const;

    void setKey(int index, int value);
    void setRecordOffset(int index, int value);

    int serialize(std::unique_ptr<char[]> &serializedPage) const;
    static BtreePage deserialize(const std::unique_ptr<char[]> &serializedPage);
};

#endif // BTREEPAGE_H