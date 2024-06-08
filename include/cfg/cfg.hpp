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

    set<string> phis;

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

pair<Node *, Node *> make_cfg(Block *block)
{
    Node *root = new Node();
    Node *current = root;

    for (auto op : block->operators)
    {
        if (op->is_if())
        {
            IFOperatorAST *if_op = (IFOperatorAST *)op;
            auto then_cfg = make_cfg(if_op->_then);
            Node *then_start_block = then_cfg.first;
            Node *then_end_block = then_cfg.second;
            connect(current, then_start_block);

            auto else_cfg = make_cfg(if_op->_else);
            Node *else_start_block = else_cfg.first;
            Node *else_end_block = else_cfg.second;
            connect(current, else_start_block);

            Node *merge_block = new Node();

            connect(then_end_block, merge_block);
            connect(else_end_block, merge_block);

            current = merge_block;
        }

        if (op->is_while())
        {
            WhileAST *while_op = (WhileAST *)op;
            auto while_cfg = make_cfg(while_op->body);
            Node *while_start_block = while_cfg.first;
            Node *while_end_block = while_cfg.second;

            Node *merge_block = new Node();

            connect(current, while_start_block);
            connect(current, merge_block);
            connect(while_end_block, while_start_block);
            connect(while_end_block, merge_block);

            current = merge_block;
        }

        root->content->operators.push_back(op);
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

    vector<Node*> get_nodes(Node* root) {
        postorder(root);
        return order;
    }
};

void print_cfg(vector<Node *> nodes, std::ostream &out)
{
    out << "digraph {" << std::endl;
    for (auto n : nodes)
    {
        const int name = n->get_name();
        out << name << "[label=<" << name << ">];" << std::endl;
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
    // это новый граф, с новыми ребрами, но старым содержимым в вершинах
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
            std::cout << "Для " << n->name + 1 << " не нашлось idom'а " << std::endl;
        }
    }
}

void get_dtree(vector<Node*> &postorder) {
    Node *root_node = postorder.back();

    auto reverse_postorder = postorder;
    std::reverse(postorder.begin(), postorder.end());

    get_dominators(reverse_postorder); // считаем множества доминаторов неоптимально
    get_dominator_tree(reverse_postorder, root_node); // считаем дерево доминаторов перебором ):
}

void set_frontiers(vector<Node*> &nodes)
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

set<Node*> get_frontier(set<Node*> &S) {
    set<Node*> res;

    for (auto n : S) {
        for (auto b : n->frontier) {
            res.insert(b);
        }
    }

    return res;
}

set<Node*> get_iterate_frontier(set<Node*> &S) {

    set<Node*> F = S;

    bool changed = true;
    while (changed) {
        changed = false;
        auto uFS = F;
        for (auto n : S) {
            uFS.insert(n);
        }
        auto nF = get_frontier(uFS);
        if (nF != F) {
            changed = true;
            F = nF;
        }
    }
    return F;
}

