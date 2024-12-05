#include "defines.h"
#include "BtreeNode.h"
#include <iostream>
#include <vector>

class BtreePage
{
    int parentOffset;
    int recordsOnPageCount;

    // Node 0 is special, in that it only contains the pointer
    // to page with keys lower than node 1 key
    std::vector<BtreeNode> nodes;

public:
    BtreePage();
    bool isRoot();
};