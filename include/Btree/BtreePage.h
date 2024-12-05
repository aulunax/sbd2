#include "defines.h"
#include "BtreeNode.h"
#include <iostream>
#include <vector>
#include <memory>

class BtreePage
{
public:
    int parentOffset;

    std::vector<BtreeNode> nodes;

    BtreePage() {};

public:
    // BtreePage();
    BtreePage(int parentOffset) : parentOffset(parentOffset) {}
    BtreePage(int parentOffset, std::vector<BtreeNode> nodes);
    void addNode(BtreeNode node);

    int getRecordsOnPageCount() const;
    int getParentOffset() const;
    int serialize(std::unique_ptr<char[]> &serializedPage) const;

    void setParentOffset(int parentOffset);

    std::vector<int> getSiblingNodes(int index);

    bool compensation();
    bool split();

    BtreeNode getNode(int index) const;
    int getKey(int index) const;

    bool isRoot();

    static BtreePage deserialize(const std::unique_ptr<char[]> &serializedPage);
};