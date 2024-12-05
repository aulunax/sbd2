#include "Btree/BtreeHandler.h"

BtreeHandler::BtreeHandler(std::string indexFilename, std::string dataFilename)
{
    dataFile = std::make_unique<RecordBlockIO>(dataFilename);
    indexFile = std::make_unique<IndexBlockIO>(indexFilename);
}