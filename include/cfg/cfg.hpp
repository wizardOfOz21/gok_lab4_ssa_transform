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

/*
    Класс узла графа управления (cfg) и дерева доминаторов тоже.
    Божественные объекты - не очень хорошо.
    TODO: разбить на несколько классов.
*/
class Node
{

public:
    // Элементы узла cfg

    // номер при postorder обходе
    int name;
    // простые операторы в базовом блоке: присваивание, return и jump
    Block *content;
    ////

    vector<Node *> preds;
    vector<Node *> succs;

    // элементы узла дерева доминаторов
    Node *idom;
    vector<Node *> dtree_childs;
    /////

    // Множество узлов, доминирующих над данным
    set<Node *> Dom;
    // DF для данного узла
    set<Node *> frontier;

    set<string> defs;
    // названия переменных, для которых в узле есть phi
    // deprecated: заменено на phis
    set<string> phis;
    // phi-операторы
    // TODO: перенести в content
    vector<PhiOperatorAST *> phis_ops;

    // оператор условного перехода, если такой после блока есть
    // TODO: перенести в content
    ExprAST *cond;

    Node() : content(new Block()) {}
    Node(Node *n) : name(n->name), content(n->content) {}
    Node(Block *content) : content(content) {}
    Node(Block *content, const vector<Node *> &preds, const vector<Node *> &succs)
        : content(content), preds(preds), succs(succs) {}

    // определеяет, каким по счету является node
    // в списке предшественников данного
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

    // строковое представление содержимого узла
    // собирает фи, операторы и условный переход в одно тело
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

    int get_name()
    {
        return name + 1;
    }
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

// создает cfg для блока ast-блока block, разворачивая if- и while-операторы
// в names положатся все имена, которые в нем встречаются
// возвращает первый и последний узлы получившегося графа
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
