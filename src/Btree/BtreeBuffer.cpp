#include "BtreeBuffer.h"
#include <algorithm>

void BtreeBuffer::pushPage(BtreePage &page, int pageOffset, IndexBlockIO *indexFile, bool writing)
{
    auto it = std::find_if(pageBuffer.begin(), pageBuffer.end(),
                           [pageOffset](const BufferPage &bufferPage)
                           {
                               return bufferPage.pageOffset == pageOffset;
                           });

    if (it != pageBuffer.end())
    {
        // If the page exists, move it to the front (most recent)
        BufferPage existingPage = *it;
        pageBuffer.erase(it);
        existingPage.page = page;
        if (writing)
        {
            existingPage.modified = true;
        }
        pageBuffer.insert(pageBuffer.begin(), existingPage);
    }
    else
    {
        // If the page does not exist, create a new BufferPage
        BufferPage newBufferPage;
        newBufferPage.page = page;
        newBufferPage.pageOffset = pageOffset;
        if (writing)
        {
            newBufferPage.modified = true;
        }

        // Add the new page to the front
        pageBuffer.insert(pageBuffer.begin(), newBufferPage);

        // If the buffer exceeds the maximum size, remove the least recent page
        if (pageBuffer.size() > height + 1)
        {
            if (pageBuffer.back().modified)
            {
                indexFile->writePageAt(pageBuffer.back().pageOffset, pageBuffer.back().page);
            }
            pageBuffer.pop_back();
        }
    }
}

std::optional<BtreePage *> BtreeBuffer::getPage(int pageOffset)
{
    // Search for the page in the buffer
    auto it = std::find_if(pageBuffer.begin(), pageBuffer.end(),
                           [pageOffset](const BufferPage &bufferPage)
                           {
                               return bufferPage.pageOffset == pageOffset;
                           });

    // If the page is found
    if (it != pageBuffer.end())
    {
        it->page.setThisPageOffset(it->pageOffset);
        return &(it->page);
    }

    // If not found, return std::nullopt
    return std::nullopt;
}
