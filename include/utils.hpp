#pragma once
#include <vector>
#include <map>
#include <set>
#include <sstream>

using std::vector;
using std::map;
using std::set;
using std::string;

template <class T>
vector<T> *all(vector<T> *a, vector<T> *b)
{
    a->insert(a->end(), b->begin(), b->end());
    return a;
}

template <class K>
map<K,int> vec2map(const vector<K>& vec) {
    map<K, int> map;
    for (int i = 0; i < vec.size(); ++i)
    {
        map[vec[i]] = i;
    }
    return map;
}

template <class V>
set<V> vec2set(const vector<V>& vec) {
    set<V> set;
    for (auto el : vec)
    {
        set.insert(el);
    }
    return set;
}

template <class V>
set<V> intersect(const set<V>& s1, const set<V> s2) {
    set<V> res;
    for (auto e : s1) {
        if (s2.find(e) != s2.end()) {
            res.insert(e);
        }
    }
    return res;
}

template <class V>
set<V> intersect_all(const set<set<V>>& ss) {

    if (ss.empty()) {
        return {};
    }
    
    set<V> res = *(ss.begin());

    for (auto s : ss) {
        res = intersect(res, s);
    }

    return res;
}
