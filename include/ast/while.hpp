
#pragma once
#include "block.hpp"

class WhileAST : OperatorAST
{
public:
    ExprAST *cond;
    Block *body;

    bool is_while() {return true;}

    WhileAST(ExprAST *cond, Block *body)
        : cond(cond), body(body) {}

    ~WhileAST()
    {
        delete cond;
        delete body;
    }

    void codegen()
    {
        Value *cond_val = cond->codegen();
        if (!cond_val)
        {
            in_func_name_print();
            std::cout << "Условие для while не вычислилось" << std::endl;
            return;
        }
        cond_val = Builder->CreateFCmpONE(
            cond_val, ConstantInt::get(*TheContext, APInt(32, 0.0)), "whilecond");

        Function *TheFunction = Builder->GetInsertBlock()->getParent();
        BasicBlock *WhileBB =
            BasicBlock::Create(*TheContext, "while", TheFunction);
        BasicBlock *PostWhileBB =
            BasicBlock::Create(*TheContext, "postwhile");
        
        Builder->CreateCondBr(cond_val, WhileBB, PostWhileBB);
        Builder->SetInsertPoint(WhileBB);
            body->codegen();
        cond_val = Builder->CreateFCmpONE(
            cond->codegen(), ConstantInt::get(*TheContext, APInt(32, 0.0)), "whilecond");
        Builder->CreateCondBr(cond_val, WhileBB, PostWhileBB);

        TheFunction->insert(TheFunction->end(), PostWhileBB);
        Builder->SetInsertPoint(PostWhileBB);
    }
};
