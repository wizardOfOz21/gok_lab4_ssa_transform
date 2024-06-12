#pragma once

#include "dominator/dtree.hpp"

void count_frontiers(vector<Node *> &nodes)
{
    for (auto b : nodes)
    {
        if (b->preds.size() >= 2)
        {
            for (auto p : b->preds)
            {
                auto runner = p;
                while (runner != b->idom)
                {
                    runner->frontier.insert(b);
                    runner = runner->idom;
                }
            }
        }
    }
}

// Dominance Frontier для множества узлов S
set<Node *> get_frontier(set<Node *> &S)
{
    set<Node *> res;

    for (auto n : S)
    {
        for (auto b : n->frontier)
        {
            res.insert(b);
        }
    }

    return res;
}

// итерированный Dominance Frontier (DF+) для множества узлов S
set<Node *> get_iterate_frontier(set<Node *> &S)
{

    set<Node *> F = S;

    bool changed = true;
    while (changed)
    {
        changed = false;
        auto uFS = F;
        for (auto n : S)
        {
            uFS.insert(n);
        }
        auto nF = get_frontier(uFS);
        if (nF != F)
        {
            changed = true;
            F = nF;
        }
    }
    return F;
}
