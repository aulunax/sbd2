#include <iostream>
#include <random>
#include <optional>
#include "defines.h"

#ifndef RECORD_H
#define RECORD_H

class Record
{
public:
    int arr[RECORD_INT_COUNT];
    int key;

    void Randomize();
    int sum();
    void fill(int n);
    void print(bool doIncludeSum = false);
    std::string toString(bool doIncludeSum = false);
    bool operator==(const Record &other) const;
    bool operator>(const Record &other) const;
    bool operator<(const Record &other) const;
};

typedef std::optional<Record> OptionalRecord;

#endif // RECORD_H