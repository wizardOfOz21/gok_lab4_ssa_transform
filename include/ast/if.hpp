#pragma once 

#include "block.hpp"

class IFOperatorAST: OperatorAST
{
public:
    ExprAST *cond;
    Block *_then;
    Block *_else;

    bool is_if() {return true;}

    IFOperatorAST(ExprAST *cond, Block *_then, Block *_else)
        : cond(cond), _then(_then), _else(_else) {}

    ~IFOperatorAST()
    {
        delete cond;
        delete _then;
        delete _else;
    }

    void codegen()
    {
        Value *cond_val = cond->codegen();
        if (!cond_val)
        {
            in_func_name_print();
            std::cout << "Условие для if не вычислилось" << std::endl;
            return;
        }

        cond_val = Builder->CreateFCmpONE(
            cond_val, ConstantInt::get(*TheContext, APInt(32, 0.0)), "ifcond");

        Function *TheFunction = Builder->GetInsertBlock()->getParent();

        BasicBlock *ThenBB =
            BasicBlock::Create(*TheContext, "then", TheFunction);
        BasicBlock *ElseBB = BasicBlock::Create(*TheContext, "else");
        BasicBlock *MergeBB = BasicBlock::Create(*TheContext, "endif");
        Builder->CreateCondBr(cond_val, ThenBB, ElseBB);

        Builder->SetInsertPoint(ThenBB);
            _then->codegen();
        Builder->CreateBr(MergeBB);

        TheFunction->insert(TheFunction->end(), ElseBB);

        Builder->SetInsertPoint(ElseBB);
            _else->codegen();
        Builder->CreateBr(MergeBB);

        TheFunction->insert(TheFunction->end(), MergeBB);
        Builder->SetInsertPoint(MergeBB);
    }
};
