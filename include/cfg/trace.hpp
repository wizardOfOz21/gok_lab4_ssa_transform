#pragma once

#include <fstream>
#include "cfg/cfg.hpp"
#include "cfg/dfs.hpp"

void print_cfg(vector<Node *> nodes, std::ostream &out, bool info = false)
{
    out << "digraph {" << std::endl;
    for (auto n : nodes)
    {
        const int name = n->get_name();
        string label;
        string xlabel;
        if (info)
        {
            label = n->stringify();
            xlabel = std::to_string(name);
        }
        else
        {
            label = std::to_string(name);
        }
        out << name;
        out << "[label=\"" << label << "\"";
        out << ", xlabel=\"" << xlabel << "\"";
        out << "];" << std::endl;
        for (auto s : n->succs)
        {
            const int s_name = s->get_name();
            out << name << " -> " << s_name << ";" << std::endl;
        }
    }
    out << "}" << std::endl;
}

void print_dtree(vector<Node *> nodes, std::ostream &out)
{
    out << "digraph {" << std::endl;
    for (auto n : nodes)
    {
        const int name = n->get_name();
        out << name << "[label=<" << name << ">];" << std::endl;
        for (auto c : n->dtree_childs)
        {
            const int c_name = c->get_name();
            out << name << " -> " << c_name << ";" << std::endl;
        }
    }
    out << "}" << std::endl;
}

void trace_cfg(const vector<Node *> &nodes, bool info = false)
{
    std::ofstream cfg_out("out/logs/cfg");
    print_cfg(nodes, cfg_out, info);
    cfg_out.close();
}

void trace_dtree(const vector<Node *> &nodes)
{
    std::ofstream dtree_out("out/logs/dtree");
    print_dtree(nodes, dtree_out);
    dtree_out.close();
}
