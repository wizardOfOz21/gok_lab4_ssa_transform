#pragma once
#include "dominator/dominator.hpp"

// считает для каждого узла idom путем перебора и строит дерево доминаторов
// на вход нужно подавать дерево, в котором у узлов уже вычислены множества доминаторов
void count_dtree_on_doms(
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

// считает множества доминаторов и вычисляет дерево доминаторов, сохраняя данные в самих узлах
void count_dtree(vector<Node *> &postorder)
{
    auto reverse_postorder = postorder;
    std::reverse(postorder.begin(), postorder.end());

    count_doms(reverse_postorder);
    count_dtree_on_doms(reverse_postorder, reverse_postorder[0]);
}
