
#pragma once

#include "cfg/cfg.hpp"

Node *get_cfg_mock(int num)
{
    switch (num)
    {
    case 1:
    {
        const int size = 6;
        vector<Node *> n(size + 1);
        for (int i = 1; i <= size; ++i)
        {
            n[i] = new Node();
        }
        connect(n[6], n[4]);
        connect(n[6], n[5]);
        connect(n[5], n[1]);

        connect(n[4], n[3]);
        connect(n[4], n[2]);

        connect(n[3], n[2]);
        connect(n[2], n[3]);
        connect(n[1], n[2]);
        connect(n[2], n[1]);
        return n[size];
    }
    case 2:
    {
        const int size = 5;
        vector<Node *> n(size + 1);
        for (int i = 1; i <= size; ++i)
        {
            n[i] = new Node();
        }
        connect(n[5], n[4]);
        connect(n[5], n[3]);
        connect(n[4], n[1]);

        connect(n[3], n[2]);
        connect(n[1], n[2]);
        connect(n[2], n[1]);
        return n[size];
    }
    case 3:
    {
        const int size = 4;
        vector<Node *> n(size + 1);
        for (int i = 1; i <= size; ++i)
        {
            n[i] = new Node();
        }
        connect(n[4], n[2]);
        connect(n[4], n[3]);
        connect(n[2], n[1]);
        connect(n[3], n[1]);
        return n[size];
    }
    default:
        break;
    }
}
