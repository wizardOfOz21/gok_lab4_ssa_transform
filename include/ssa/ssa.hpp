#pragma once

#include "cfg/cfg.hpp"
#include "cfg/dfs.hpp"

#include "dominator/dtree.hpp"
#include "dominator/frontier.hpp"

#include "dominator/trace.hpp"
#include "cfg/trace.hpp"
#include "ssa/trace.hpp"

void insert_phis(map<string, set<Node *>> &names_defs)
{
    for (auto name_pair : names_defs)
    {
        auto phi_insert_blocks = get_iterate_frontier(name_pair.second);
        for (auto n : phi_insert_blocks)
        {
            n->phis.insert(name_pair.first);
            vector<OperatorAST *> &n_op_list = n->content->operators;
            n->phis_ops.push_back(new PhiOperatorAST(name_pair.first, n->preds.size()));
        }
    }
}

void traverse(Node *v, string p, vector<int> &stack, int &counter)
{
    Block *content = v->content;
    int ver_counter = 0; // количество версий переменной p в блоке
    for (auto phi_op : v->phis_ops)
    {
        const int i = counter;
        if (phi_op->rename_lhs(p, i))
        {
            ver_counter++;
            stack.push_back(i);
            counter++;
        };
    }
    for (auto op : content->operators)
    {
        if (!op->is_phi())
        {
            op->rename_rhs(p, stack.back());
        }
        if (op->is_assign())
        {
            const int i = counter;
            if (op->rename_lhs(p, i))
            {
                ver_counter++;
                stack.push_back(i);
                counter++;
            };
        }
    }
    if (v->cond)
    {
        v->cond->rename(p, stack.back());
    }
    for (auto s : v->succs)
    {
        int j = s->which_pred(v);
        const int i = stack.back();
        for (auto phi : s->phis_ops)
        {
            if (phi->var_name == p)
            {
                phi->args[j] = i;
            }
        }
    }
    for (auto child : v->dtree_childs)
    {
        traverse(child, p, stack, counter);
    }
    // снимаем все версии определенные в блоке
    // при переходе к обходу его сиблинга
    for (int i = 0; i < ver_counter; ++i)
    {
        stack.pop_back();
    }
}

void rename_var(Node *root_node, string var)
{
    int counter = 0;
    vector<int> stack;
    stack.push_back(-1);
    // дно стека не используется для переименования,
    // так как семантически перед любым использованием
    // переменной должно быть присваивание ей, то есть в стеке что-то будет,
    // дно – необходимость реализации
    traverse(root_node, var, stack, counter);
}

Node * ssa_transform(Block *b, bool trace = false)
{
    map<string, set<Node *>> names_defs;
    auto cfg_nodes = make_cfg(b, names_defs);

    Node *root_node = cfg_nodes.first;
    // Node *root_node = get_cfg_mock(3);

    DfsProvider bp;
    auto nodes = bp.get_postorder(root_node);

    count_dtree(nodes);
    count_frontiers(nodes);

    insert_phis(names_defs);

    // rename_var(root_node, "a");
    // rename_var(root_node, "c");
    for (auto name : names_defs)
    {
        rename_var(root_node, name.first);
    }

    if (trace)
    {
        trace_names(names_defs);
        trace_cfg(nodes, true);
        trace_frontiers(nodes);
        trace_dtree(nodes);
        trace_doms(nodes);
    }

    return root_node;
}
