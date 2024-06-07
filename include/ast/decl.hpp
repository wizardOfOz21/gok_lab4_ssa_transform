#pragma once

#include <vector>
#include <string>

#include "block.hpp"
#include <format>

using std::string;
using std::vector;

class DeclAST
{
public:
    virtual ~DeclAST(){};
    virtual void signup() {};
    virtual void codegen() {};
};

class VarAST : public DeclAST
{
public:
    string name;
    ExprAST *val;

    VarAST(const string &name, ExprAST *val) : name(name), val(val) {}

    // выделяет стек под переменную и регистрирует переменную в списке имен
    void signup()
    {
        if (val->has_calls())
        {
            std::cout << "Нельзя использовать вызовы функций при инициализации переменной: "
                      << name << std::endl;
            throw DeclareExeption();
        }

        // TODO: сделать возможным объявлять переменные через другие переменные, которые встречаются позже
        // TODO: сделать возможным объявление без инициализации (с инициализацией по умолчанию нулем)

        Value *init_val = val->codegen();

        if (!init_val)
        {
            std::cout << "Переменная, по-видимому, ссылается на не объявленную переменную, но это не точно: "
                      << name << std::endl;
            throw DeclareExeption();
        }

        Value *old_alloca = NamedValues[name];

        if (old_alloca)
        {
            std::cout << "Переменная \"" << name
                      << "\" уже объявлена, новое значение заменит старое, будьте осторожны" << std::endl;
        }

        AllocaInst *alloca = CreateEntryBlockAlloca(MainFunc, name);
        Builder->CreateStore(init_val, alloca);

        NamedValues[name] = alloca;
    };

    ~VarAST()
    {
        delete val;
    }
};

class FuncAST : public DeclAST
{
public:
    string name;
    vector<string> params;
    Block *body;

    FuncAST(const string &name, const vector<string> &params,
            Block *body) : name(name), params(params), body(body){};

    // регистрирует прототип функции в модуль
    void signup()
    {
        std::vector<Type *> Ints(0, Type::getInt32Ty(*TheContext));
        FunctionType *FT =
            FunctionType::get(Type::getInt32Ty(*TheContext), Ints, false);

        if (!FT)
        {
            std::cout << "Неверный прототип функции: " << name << std::endl;
            throw DeclareExeption();
        }

        Function *old_F = TheModule->getFunction(name);

        if (old_F)
        {
            std::cout << "Такая функция уже объявлена, либо это системная функция \""
                      << main_func_name << "\": " << name << std::endl;
            throw DeclareExeption();
        }

        Function *F =
            Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());

        if (!F)
        {
            std::cout << "Не получилось создать функцию: " << name << std::endl;
            throw DeclareExeption();
        }
    };

    void codegen()
    {
        Function *F = TheModule->getFunction(name);

        if (!F)
        {
            std::cout << "Функция не найдена при проходе кодогенератора: " << name << std::endl;
            throw CodegenExeption();
        }

        if (!F->empty())
        {
            std::cout << "Найденная при проходе кодогенератора функция не пуста: " << name << std::endl;
            throw CodegenExeption();
        }

        BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", F);
        Builder->SetInsertPoint(BB);

        body->codegen();

        // возвращаем 0, на всякий случай
        // если раньше return уже был, то оптимизатор всё равно
        // удалит мертвый код дальше
        auto ret_val = new NumberAST(0);
        ReturnAST(ret_val).codegen();

        if (!verifyFunction(*F))
        {
            std::cout << "Не удалось верифицировать функцию" << std::endl;
            throw CodegenExeption();
        };

        TheFPM->run(*F, *TheFAM);
    };

    ~FuncAST()
    {
        delete body;
    }
};

class EntryAST : public DeclAST
{
public:
    string name;

    EntryAST(const string &name) : name(name){};

    void signup()
    {
        entry_function_name = name;
    }
};
