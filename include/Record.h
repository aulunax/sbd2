#include <iostream>
#include <random>
#include "defines.h"

class Record
{
public:
    RecordType arr[RECORD_INT_COUNT];
    int key;

    void Randomize();
    int sum();
    void print(bool doIncludeSum = false);
    std::string toString(bool doIncludeSum = false);
    bool operator==(const Record &other) const;
    bool operator>(const Record &other) const;
    bool operator<(const Record &other) const;
};
