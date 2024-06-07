#pragma once

#include <string>
#include "builder.hpp"
#include "expr.hpp"
#include "error.hpp"

using std::string;
class OperatorAST
{
public:
    virtual ~OperatorAST(){};
    virtual void codegen() {};
    virtual bool is_return() { return false; };
};

class AssignAST : public OperatorAST
{
public:
    string var_name;
    ExprAST *val;

    ~AssignAST()
    {
        delete val;
    }

    void codegen()
    {
        Value *var_alloca = NamedValues[var_name];

        if (!var_alloca)
        {
            in_func_name_print();
            std::cout << "Такой переменной ранее не было объявлено: " << var_name << std::endl;
            throw CodegenExeption();
        }

        Value *var_val = val->codegen();

        if (!var_val)
        {
            in_func_name_print();
            std::cout << "Не удалось вычислить значение переменной: " << var_name << std::endl;
            throw CodegenExeption();
        }

        Builder->CreateStore(var_val, var_alloca);
    };

    AssignAST(const string var_name, ExprAST *val)
        : var_name(var_name), val(val) {}
};

class ReturnAST : public OperatorAST
{
public:
    ExprAST *val;

    ~ReturnAST()
    {
        delete val;
    }

    void codegen()
    {
        Value *ret_val = val->codegen();

        if (!ret_val)
        {
            in_func_name_print();
            std::cout << "Возвращаемое значение функции не удалось вычислить" << std::endl;
            throw CodegenExeption();
        }

        Builder->CreateRet(ret_val);
    };

    bool is_return() { return true; };

    ReturnAST(ExprAST *val)
        : val(val) {}
};

// Если унаследовать ExprAST от OperatorAST, будет циклическая зависимость между файлами, поэтому так
class ExprOperatorAST : public OperatorAST
{
public:
    ExprAST *val;

    ExprOperatorAST(ExprAST *val) : val(val) {}

    void codegen()
    {
        Value *Val = val->codegen();
        if (!Val)
        {
            in_func_name_print();
            std::cout << "Не удалось вычислить значение свободного выражения" << std::endl;
            throw CodegenExeption();
        }
        Builder->Insert(Val, "tmp");
    }

    ~ExprOperatorAST()
    {
        delete val;
    }
};

class LocalVarAST
{
public:
    string name;
    ExprAST *init_val;

    LocalVarAST(const string &name, ExprAST *init_val) : name(name), init_val(init_val) {}
    ~LocalVarAST()
    {
        delete init_val;
    }

    void codegen()
    {
        Value *old_snapshot_val = snapshots.back()[name];
        if (old_snapshot_val)
        {
            in_func_name_print();
            std::cout << "Повторное объявление локальной переменной в одном блоке: "
                      << name << std::endl;
            throw CodegenExeption();
        }

        AllocaInst *old_val = NamedValues[name];
        snapshots.back()[name] = old_val;
        AllocaInst *alloca = CreateEntryBlockAlloca(Builder->GetInsertBlock()->getParent(), name);
        Builder->CreateStore(init_val->codegen(), alloca);
        NamedValues[name] = alloca;
    }
};

class LocalVarDeclOpAST : public OperatorAST
{
public:
    vector<LocalVarAST *> vars;

    LocalVarDeclOpAST(const vector<LocalVarAST *> &vars) : vars(vars) {}

    void codegen()
    {
        for (auto var : vars)
        {
            var->codegen();
        }
    }

    ~LocalVarDeclOpAST()
    {
        for (auto var : vars)
        {
            delete var;
        }
    }
};

