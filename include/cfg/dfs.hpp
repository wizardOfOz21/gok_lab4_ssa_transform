#pragma once

#include "cfg/cfg.hpp"

// простой класс для обхода cfg 
class DfsProvider
{
public:
    vector<Node *> order;
    set<Node *> visited;

    bool is_visited(Node *n)
    {
        return visited.find(n) != visited.end();
    }

    void traverse(Node *n)
    {
        visited.insert(n);
        for (auto s : n->succs)
        {
            if (!is_visited(s))
            {
                traverse(s);
            }
        }
        n->name = order.size();
        order.push_back(n);
    }

    vector<Node *> get_postorder(Node *root)
    {
        order = {};
        visited = {};

        traverse(root);

        return order;
    }
};
