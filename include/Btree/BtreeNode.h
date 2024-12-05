#include "defines.h"

class BtreeNode
{
public:
    int pagePtr;
    int key;
    int recordOffset;

    BtreeNode(int pagePtr, int key, int recordOffset)
        : pagePtr(pagePtr), key(key), recordOffset(recordOffset) {};
};