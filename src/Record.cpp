#include "Record.h"

void Record::Randomize()
{
    key = rand() % MAX_KEY_VALUE;
    for (int i = 0; i < RECORD_INT_COUNT; i++)
    {
        arr[i] = rand() % 1000;
    }
}

int Record::sum()
{
    int sum = 0;
    for (int i = 0; i < RECORD_INT_COUNT; i++)
    {
        sum += arr[i];
    }
    return sum;
}

void Record::fill(int n)
{
    for (int i = 0; i < RECORD_INT_COUNT; i++)
    {
        arr[i] = n;
    }
}

void Record::print(bool doIncludeSum)
{
    if (doIncludeSum)
    {
        std::cout << "(" << sum() << ") ";
    }

    for (int i = 0; i < RECORD_INT_COUNT; i++)
    {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
}

std::string Record::toString(bool doIncludeSum)
{
    std::string str;

    str = "(" + std::to_string(key) + ") ";
    for (int i = 0; i < RECORD_INT_COUNT; i++)
    {
        str += std::to_string(arr[i]) + " ";
    }
    return str;
}

bool Record::operator==(const Record &other) const
{
    if (key != other.key)
    {
        return false;
    }
    return true;
}

bool Record::operator>(const Record &other) const
{
    if (key > other.key)
    {
        return true;
    }
    return false;
}

bool Record::operator<(const Record &other) const
{
    return !(key > other.key);
}
