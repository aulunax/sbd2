#include "defines.h"
#include "BtreeNode.h"
#include <iostream>
#include <vector>
#include <memory>

#ifndef BTREEPAGE_H
#define BTREEPAGE_H

class BtreePage
{
public:
    int parentOffset;
    int thisPageOffset;
    std::vector<BtreeNode> nodes;

    BtreePage() {};

public:
    // BtreePage();
    BtreePage(int parentOffset) : parentOffset(parentOffset) {}
    BtreePage(int parentOffset, std::vector<BtreeNode> nodes);
    void addNode(BtreeNode node);
    void insertNode(BtreeNode node);
    void clearNodes();

    int getRecordsOnPageCount() const;
    int getParentOffset() const;
    void setParentOffset(int parentOffset);
    int getThisPageOffset() const { return thisPageOffset; }
    void setThisPageOffset(int thisPageOffset) { this->thisPageOffset = thisPageOffset; }
    int serialize(std::unique_ptr<char[]> &serializedPage) const;

    std::pair<int, bool> bisectionSearchForKey(int key);

    std::vector<BtreeNode> &getNodes();

    bool compensation();
    bool split();

    BtreeNode getNode(int index) const;
    int getKey(int index) const;
    void setKey(int index, int value);
    int getPtr(int index) const;
    int getRecordOffset(int index) const;
    void setRecordOffset(int index, int value);

    bool isRoot();

    static BtreePage deserialize(const std::unique_ptr<char[]> &serializedPage);
};

#endif // BTREEPAGE_H