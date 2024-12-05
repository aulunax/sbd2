#include "defines.h"

class BtreeNode
{
public:
    int pagePtr;
    int key;
    int recordOffset;

    BtreeNode() : pagePtr(NULL_DATA), key(NULL_DATA), recordOffset(NULL_DATA) {};
    BtreeNode(int pagePtr, int key, int recordOffset)
        : pagePtr(pagePtr), key(key), recordOffset(recordOffset) {};
};