#include "Index.h"

Index::Index()
{
    key = -2;
    page = -1;
}

Index::Index(int key, int page)
{
    this->key = key;
    this->page = page;
}

bool Index::isEmpty()
{
    if(key == -2)
        return true;
    return false;
}
