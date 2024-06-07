#pragma once

#include <vector>
#include "decl.hpp"
#include "builder.hpp"
#include "error.hpp"

using std::vector;
class ProgramAST
{
public:
    vector<DeclAST *> decls;

    ProgramAST(const vector<DeclAST *> &decls) : decls(decls){};
    ~ProgramAST()
    {
        for (auto decl : decls)
        {
            delete decl;
        }
    };

    // проходит по дереву и собирает прототипы функций
    // и инициализирует переменные
    void declare()
    {
        Builder->SetInsertPoint(&MainFunc->getEntryBlock());

        for (auto decl : decls)
        {
            decl->signup();
        }

        if (entry_function_name.compare("") == 0) {
            std::cout << "Не была объявлена точка входа, используйте entry = <имя функции>" << std::endl;
            throw DeclareExeption();
        }

        Function* entry_func = TheModule->getFunction(entry_function_name);

        if (!entry_func) {
            std::cout << "Точка входа не найдена" << std::endl;
            throw DeclareExeption();
        }

        Value* ret_val = Builder->CreateCall(entry_func, {}, "mainrettmp");
        Builder->CreateRet(ret_val);
    }

    void codegen()
    {
        for (auto decl : decls)
        {
            decl->codegen();
        }
    }
};
