#include "BtreeBuffer.h"
#include <algorithm>

void BtreeBuffer::pushPage(BtreePage &page, int pageOffset, IndexBlockIO *indexFile, bool writing, bool lowPriority)
{
    auto it = std::find_if(pageBuffer.begin(), pageBuffer.end(),
                           [pageOffset](const BufferPage &bufferPage)
                           {
                               return bufferPage.pageOffset == pageOffset;
                           });

    if (it != pageBuffer.end())
    {
        BufferPage existingPage = *it;
        pageBuffer.erase(it);
        existingPage.page = page;
        if (writing)
        {
            existingPage.modified = true;
        }

        if (lowPriority)
        {
            pageBuffer.push_back(existingPage);
        }
        else
        {
            pageBuffer.insert(pageBuffer.begin(), existingPage);
        }
    }
    else
    {
        BufferPage newBufferPage;
        newBufferPage.page = page;
        newBufferPage.pageOffset = pageOffset;
        if (writing)
        {
            newBufferPage.modified = true;
        }

        // checking height before adding, so buffer size is height+1
        if (pageBuffer.size() > height)
        {
            if (pageBuffer.back().modified)
            {
                indexFile->writePageAt(pageBuffer.back().pageOffset, pageBuffer.back().page);
            }
            pageBuffer.pop_back();
        }

        if (lowPriority)
        {
            pageBuffer.push_back(newBufferPage);
        }
        else
        {
            pageBuffer.insert(pageBuffer.begin(), newBufferPage);
        }
    }
}

std::optional<BtreePage *> BtreeBuffer::getPage(int pageOffset, bool lowPriority)
{
    auto it = std::find_if(pageBuffer.begin(), pageBuffer.end(),
                           [pageOffset](const BufferPage &bufferPage)
                           {
                               return bufferPage.pageOffset == pageOffset;
                           });

    if (it != pageBuffer.end())
    {
        it->page.setThisPageOffset(it->pageOffset);

        if (!lowPriority)
        {
            BufferPage foundPage = *it;
            pageBuffer.erase(it);
            pageBuffer.insert(pageBuffer.begin(), foundPage);
            return &(pageBuffer.front().page);
        }

        return &(it->page);
    }

    return std::nullopt;
}
