#pragma once
#include "operator.hpp"

class Block
{
public:
    vector<OperatorAST *> operators;
    Block(const vector<OperatorAST *> &ops) : operators(ops) {}
    ~Block()
    {
        for (auto op : operators)
        {
            delete op;
        }
    }

    void codegen()
    {
        // создаем словарь(снимок) для сохранения переменных, 
        // которые могут быть перекрыты в блоке
        snapshots.push_back(snapshot());

        for (auto op : operators)
        {
            op->codegen();
        }

        snapshot &snap = snapshots.back();
        // возвращаем NamedValues в состояние до входа в блок
        // с помощью снимка
        snapshot::iterator it;
        for (it = snap.begin(); it != snap.end(); it++)
        {
            NamedValues[it->first] = it->second;
        }

        // удаляем снимок
        snapshots.pop_back();
    }
};
