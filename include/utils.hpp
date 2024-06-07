#pragma once
#include <vector>

using std::vector;

template <class T>
vector<T> *all(vector<T> *a, vector<T> *b)
{
    a->insert(a->end(), b->begin(), b->end());
    return a;
}
