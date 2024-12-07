#include "BtreeBuffer.h"
void BtreeBuffer::setCurrentDepthTo(int depth, IndexBlockIO *indexFile)
{
    if (depth < pageBuffer.size())
    {
        for (int i = pageBuffer.size() - 1; i >= depth; i--)
        {
            indexFile->writePageAt(pageBuffer[i].pageOffset, pageBuffer[i].page);
        }
        pageBuffer.resize(depth);
    }
    else
    {
        throw std::runtime_error("Error: Could not set buffer depth to " + std::to_string(depth));
    }
}

void BtreeBuffer::addPage(BtreePage &page, int pageOffset)
{
    if (pageBuffer.size() > 6)
    {
        throw std::runtime_error("Error: Thats a large buffer");
    }
    pageBuffer.push_back({page, pageOffset});
}

std::optional<BtreePage *> BtreeBuffer::getPage(int pageOffset, IndexBlockIO *indexFile)
{
    int curDepth = 0;
    BufferPage &bufferPage = pageBuffer[0];
    for (int i = 0; i < pageBuffer.size(); i++)
    {
        curDepth++;
        bufferPage = pageBuffer[i];
        if (bufferPage.pageOffset == pageOffset)
        {
            if (realDepth > curDepth)
            {
                setCurrentDepthTo(curDepth, indexFile);
            }
            realDepth = curDepth;
            bufferPage.page.setThisPageOffset(bufferPage.pageOffset);
            return &bufferPage.page;
        }
    }
    if (realDepth != 0)
    {
        realDepth = 0;
        setCurrentDepthTo(0, indexFile);
    }
    return std::nullopt;
}
