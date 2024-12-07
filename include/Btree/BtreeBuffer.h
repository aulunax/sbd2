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
    bool modified = false;
};

class BtreeBuffer
{
    std::vector<BufferPage> pageBuffer;

    int height = 0;

public:
    void setNewHeight(int height) { this->height = height; }

    void flush(IndexBlockIO *indexFile)
    {
        for (BufferPage &bufferPage : pageBuffer)
        {
            indexFile->writePageAt(bufferPage.pageOffset, bufferPage.page);
        }
        pageBuffer.clear();
    }

    void pushPage(BtreePage &page, int pageOffset, IndexBlockIO *indexFile, bool writing);
    std::optional<BtreePage *> getPage(int pageOffset);
};

#endif // BTREEBUFFER_H