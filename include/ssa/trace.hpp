#pragma once

#include <fstream>
#include "cfg/cfg.hpp"

void trace_names(map<string, set<Node *>> &names)
{
    std::ofstream names_def_out("out/logs/names_def");
    for (auto name_pair : names)
    {
        names_def_out << name_pair.first << ": {";
        for (auto n : name_pair.second)
        {
            names_def_out << n->get_name() << ",";
        }
        names_def_out << "}" << std::endl;
    }
    names_def_out.close();
}
