#include "Record.h"
#include <cmath>



Record::Record()
{
    key = -2;
    a = b = c = 0;
    overflowPointer = -1;
    deleted = false;
}

Record::Record(int key, double a, double b, double c)
{
    this->key = key;
    this->a = a;
    this->b = b;
    this->c = c;
    overflowPointer = -1;
    deleted = false;
}

bool Record::isEmpty() {
    if (key == -2)
        return true;
    return false;
}

bool Record::isGuard() {
    if (key == -1 && deleted)
        return true;
    return false;
}

