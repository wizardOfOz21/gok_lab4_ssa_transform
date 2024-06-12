#pragma once

#include <vector>
#include <set>
#include <map>
#include <iostream>

#include "ast/block.hpp"
#include "ast/if.hpp"
#include "ast/while.hpp"
#include "utils.hpp"

using std::map;
using std::pair;
using std::set;
using std::vector;

class Node
{

public:
    int name;
    Block *content;

    vector<Node *> preds;
    vector<Node *> succs;

    Node *idom;
    vector<Node *> dtree_childs;

    set<Node *> Dom;
    set<Node *> frontier;

    set<string> defs;
    set<string> phis;
    vector<PhiOperatorAST *> phis_ops;
    ExprAST *cond;

    string stringify()
    {
        std::ostringstream ss;
        for (auto op : phis_ops)
        {
            ss << op->stringify();
        }
        ss << content->stringify();
        if (cond)
        {
            ss << "jump: ";
            ss << cond->stringify() << " ? ";
            ss << succs[0]->get_name() << " : ";
            ss << succs[1]->get_name() << std::endl;
        }
        return ss.str();
    }

    int which_pred(Node *node)
    {
        for (int i = 0; i < preds.size(); ++i)
        {
            if (preds[i] == node)
            {
                return i;
            }
        }
        return -1;
    }

    int get_name()
    {
        return name + 1;
    }

    Node() : content(new Block()) {}
    Node(Node *n) : name(n->name), content(n->content) {}
    Node(Block *content) : content(content) {}
    Node(Block *content, const vector<Node *> &preds, const vector<Node *> &succs)
        : content(content), preds(preds), succs(succs) {}
};

void connect(Node *parent, Node *child)
{
    parent->succs.push_back(child);
    child->preds.push_back(parent);
}

void connect_idom(Node *node, Node *idom)
{
    node->idom = idom;
    idom->dtree_childs.push_back(node);
}

pair<Node *, Node *> make_cfg(Block *block, map<string, set<Node *>> &names)
{
    Node *root = new Node();
    Node *current = root;

    for (auto op : block->operators)
    {
        if (op->is_if())
        {
            IFOperatorAST *if_op = (IFOperatorAST *)op;
            auto then_cfg = make_cfg(if_op->_then, names);
            Node *then_start_block = then_cfg.first;
            Node *then_end_block = then_cfg.second;
            connect(current, then_start_block);

            auto else_cfg = make_cfg(if_op->_else, names);
            Node *else_start_block = else_cfg.first;
            Node *else_end_block = else_cfg.second;
            connect(current, else_start_block);

            Node *merge_block = new Node();

            connect(then_end_block, merge_block);
            connect(else_end_block, merge_block);

            current->cond = if_op->cond;
            current = merge_block;
            continue;
        }
        else if (op->is_while())
        {
            WhileAST *while_op = (WhileAST *)op;
            auto while_cfg = make_cfg(while_op->body, names);
            Node *while_start_block = while_cfg.first;
            Node *while_end_block = while_cfg.second;

            while_end_block->cond = while_op->cond;

            Node *merge_block = new Node();

            connect(current, while_start_block);
            connect(current, merge_block);
            connect(while_end_block, while_start_block);
            connect(while_end_block, merge_block);

            current->cond = while_op->cond;
            current = merge_block;
            continue;
        }
        // else if (op->is_var_defs())
        // {
        //     LocalVarDeclOpAST *var_defs_op = (LocalVarDeclOpAST *)op;
        //     for (auto def : var_defs_op->vars)
        //     {
        //         current->defs.insert(def->name);
        //         names[def->name].insert(current);
        //     }
        // }
        if (op->is_assign())
        {
            AssignAST *assign_op = (AssignAST *)op;

            current->defs.insert(assign_op->var_name);
            names[assign_op->var_name].insert(current);
        }

        current->content->operators.push_back(op);
    }

    return pair(root, current);
}

class ByPass
{
public:
    vector<Node *> order;
    set<Node *> visited;

    bool is_visited(Node *n)
    {
        return visited.find(n) != visited.end();
    }

    void postorder_dfs(Node *n)
    {
        visited.insert(n);
        for (auto s : n->succs)
        {
            if (!is_visited(s))
            {
                postorder_dfs(s);
            }
        }
        n->name = order.size();
        order.push_back(n);
    }

    void postorder(Node *root)
    {
        order = {};
        visited = {};

        postorder_dfs(root);
    }

    vector<Node *> get_nodes(Node *root)
    {
        postorder(root);
        return order;
    }
};

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

void get_dominators(const vector<Node *> &order)
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

void get_dominator_tree(
    vector<Node *> nodes, Node *root)
{
    for (auto n : nodes)
    {
        // ищем idom и цепляем его к графу
        auto nDom = n->Dom;

        nDom.erase(n);

        bool has_idom = false;
        for (auto d : nDom)
        {
            bool is_dom = true;
            auto dDom = d->Dom;
            for (auto a : nDom)
            {
                if (dDom.find(a) == dDom.end())
                {
                    is_dom = false;
                    break;
                }
            }
            if (is_dom)
            {
                connect_idom(n, d);
                has_idom = true;
            }
        }

        if (!has_idom && (n != root))
        {
            std::cout << "Для " << n->get_name() << " не нашлось idom'а " << std::endl;
        }
    }
}

void get_dtree(vector<Node *> &postorder)
{
    Node *root_node = postorder.back();

    auto reverse_postorder = postorder;
    std::reverse(postorder.begin(), postorder.end());

    get_dominators(reverse_postorder);                // считаем множества доминаторов неоптимально
    get_dominator_tree(reverse_postorder, root_node); // считаем дерево доминаторов перебором ):
}

void set_frontiers(vector<Node *> &nodes)
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

void trace_cfg(const vector<Node *> &nodes, bool info = false)
{
    std::ofstream cfg_out("out/logs/cfg");
    print_cfg(nodes, cfg_out, info);
    cfg_out.close();
}

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

void trace_dtree(const vector<Node *> &nodes)
{
    std::ofstream dtree_out("out/logs/dtree");
    print_dtree(nodes, dtree_out);
    dtree_out.close();
}

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

void ssa_transform(FuncAST *f, bool trace = false)
{
    map<string, set<Node *>> names_defs;
    auto cfg_nodes = make_cfg(f->body, names_defs);

    Node *root_node = cfg_nodes.first;
    // Node *root_node = get_cfg_mock(3);

    ByPass bp;
    auto nodes = bp.get_nodes(root_node);

    get_dtree(nodes);
    set_frontiers(nodes);

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
}
