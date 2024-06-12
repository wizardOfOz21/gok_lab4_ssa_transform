#pragma once
#include "cfg/cfg.hpp"

// TODO: убрать в cpp файл
set<set<Node *>> get_preds_doms(map<Node *, int> &postorder,
                                map<Node *, set<Node *>> &Dom,
                                const vector<Node *> &preds)
{
    set<set<Node *>> res;
    for (auto p : preds)
    {
        res.insert(Dom[p]);
    }
    return res;
}

// считает множества доминаторов для каждого узла и кладет в сам узел
// неоптимальная простая версия The Iterative Dominator Algorithm в статье
void count_doms(const vector<Node *> &order)
{
    auto order_map = vec2map(order);
    const int size = order.size();
    map<Node *, set<Node *>> Dom;

    // init
    auto universum = vec2set(order);
    for (auto n : order)
    {
        Dom[n] = universum;
        n->Dom = universum;
    }
    bool Changed = true;

    while (Changed)
    {
        Changed = false;
        for (int i = size - 1; i >= 0; --i)
        {
            Node *n = order[i];
            auto preds_doms = get_preds_doms(order_map, Dom, n->preds);
            auto new_set = intersect_all(preds_doms);
            new_set.insert(n);
            if (new_set != Dom[n])
            {
                Dom[n] = new_set;
                n->Dom = new_set;
                Changed = true;
            }
        }
    }
}
