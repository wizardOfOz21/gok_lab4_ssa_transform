#pragma once

#include <fstream>
#include "cfg/cfg.hpp"

void trace_frontiers(const vector<Node *> &nodes)
{
    std::ofstream frontiers_out("out/logs/frontiers");
    for (auto n : nodes)
    {
        frontiers_out << n->get_name() << " : {";
        for (auto d : n->frontier)
        {
            frontiers_out << d->get_name() << ",";
        }
        frontiers_out << "}" << std::endl;
    }
    frontiers_out.close();
}

void trace_doms(const vector<Node *> &nodes)
{
    std::ofstream doms_out("out/logs/doms");
    for (auto n : nodes)
    {
        doms_out << n->get_name() << " : {";
        for (auto d : n->Dom)
        {
            doms_out << d->get_name() << ",";
        }
        doms_out << "}" << std::endl;
    }
    doms_out.close();
}
