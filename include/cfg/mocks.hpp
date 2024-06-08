#pragma once

#include "cfg.hpp"

Node * get_cfg_mock(int num)
{
    switch (num)
    {
    case 1:
    {
        vector<Node *> n(7);
        for (int i = 1; i <= 6; ++i)
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
        return n[6];
    }
    case 2:
    {
        vector<Node *> n(6);
        for (int i = 1; i <= 5; ++i)
        {
            n[i] = new Node();
        }
        connect(n[5], n[4]);
        connect(n[5], n[3]);
        connect(n[4], n[1]);

        connect(n[3], n[2]);
        connect(n[1], n[2]);
        connect(n[2], n[1]);
        return n[6];
    }
    default:
        break;
    }
}
