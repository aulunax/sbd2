#include "defines.h"
#include "BtreePage.h"
#include "IndexBlockIO.h"
#include <iostream>
#include <optional>

#ifndef BTREEBUFFER_H
#define BTREEBUFFER_H

class BufferPage
{
public:
    BtreePage page;
    int pageOffset;
};

class BtreeBuffer
{
    std::vector<BufferPage> pageBuffer;

    int realDepth = 0;

public:
    void setCurrentDepthTo(int depth, IndexBlockIO *indexFile);
    int getCurrentBufferDepth() { return pageBuffer.size(); }

    void addPage(BtreePage &page, int pageOffset);
    std::optional<BtreePage *> getPage(int pageOffset, IndexBlockIO *indexFile);
};

#endif // BTREEBUFFER_H