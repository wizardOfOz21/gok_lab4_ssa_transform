#pragma once

#include <string>
#include <map>
#include <sstream>
#include "builder.hpp"
#include "expr.hpp"
#include "error.hpp"

using std::map;
using std::string;

class OperatorAST
{
public:
    virtual ~OperatorAST(){};
    virtual void codegen() {};
    virtual bool is_return() { return false; };
    virtual bool is_while() { return false; }
    virtual bool is_if() { return false; }
    virtual bool is_var_defs() { return false; }
    virtual bool is_assign() { return false; }
    virtual bool is_phi() { return false; }
    virtual bool rename_rhs(string name, int i) { return false; }
    virtual bool rename_lhs(string name, int i) { return false; }
    virtual string stringify() { return ""; }
    virtual bool is_name_def(string name) { return false; }

    std::string name_iter(std::string name, int i)
    {
        std::ostringstream ss;
        ss << name << i;
        return ss.str();
    }
};

class AssignAST : public OperatorAST
{
public:
    string var_name;
    ExprAST *val;

    bool is_name_def(string name) { return var_name == name; }
    bool is_assign() { return true; }

    string stringify()
    {
        std::ostringstream ss;
        ss << var_name << " = " << val->stringify() << ";\\n";
        return ss.str();
    }

    ~AssignAST()
    {
        delete val;
    }

    bool rename_rhs(string name, int i)
    {
        return val->rename(name, i);
    }

    bool rename_lhs(string name, int i)
    {
        if (var_name != name)
        {
            return false;
        }
        var_name = name_iter(var_name, i);
        return true;
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

    bool rename_rhs(string name, int i)
    {
        return val->rename(name, i);
    }

    string stringify()
    {
        std::ostringstream ss;
        ss << "return " << val->stringify() << ";\\n";
        return ss.str();
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

    string stringify()
    {
        std::ostringstream ss;
        ss << val->stringify() << ";\\n";
        return ss.str();
    }

    bool rename_rhs(string name, int i)
    {
        return val->rename(name, i);
    }

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

class PhiOperatorAST : public OperatorAST
{
public:
    string var_name;
    int version;
    vector<int> args;

    bool is_name_def(string _name) { return var_name == _name; }
    bool is_phi() { return true; }

    string stringify()
    {
        std::ostringstream ss;
        ss << (version != -1 ? name_iter(var_name, version) : var_name) << " = phi(";

        if (version != -1)
        {
            ss << name_iter(var_name, args[0]);
            for (int i = 1; i < args.size(); ++i)
            {
                ss << ", " << name_iter(var_name, args[i]);
            }
        }
        ss << ");\\n";
        return ss.str();
    }

    bool rename_lhs(string name, int i)
    {
        if (var_name != name)
        {
            return false;
        }
        version = i;
        return true;
    }

    PhiOperatorAST(const string &var_name, int args_length) : var_name(var_name), args(args_length, -1), version(-1) {}
};

class LocalVarAST
{
public:
    string name;
    ExprAST *init_val;

    bool is_name_def(string _name) { return name == _name; }

    std::string name_iter(std::string name, int i)
    {
        std::ostringstream ss;
        ss << name << i;
        return ss.str();
    }

    bool rename_rhs(string name, int i)
    {
        return init_val->rename(name, i);
    }

    bool rename_lhs(string name_to_rename, int i)
    {
        if (name_to_rename != name)
        {
            return false;
        }
        name = name_iter(name, i);
        return true;
    }

    string stringify()
    {
        std::ostringstream ss;
        ss << name << " = " << init_val->stringify();
        return ss.str();
    }

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
    bool is_var_defs() { return true; }

    vector<LocalVarAST *> vars;

    bool rename_rhs(string name, int i)
    {
        for (auto v : vars)
        {
            v->rename_rhs(name, i);
        }
    }
    bool rename_lhs(string name, int i)
    {
        for (auto v : vars)
        {
            v->rename_lhs(name, i);
        }
    }

    string stringify()
    {
        std::ostringstream ss;
        for (auto v : vars)
        {
            ss << v->stringify() << ";\\n";
        }
        return ss.str();
    }

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

// IFOperatorAST в if.hpp
// WhileAST в while.hpp
