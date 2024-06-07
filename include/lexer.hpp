#pragma once

#include <string>
#include <iostream>
#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif
#include <location.hpp>

union ValueType {
    std::string* ident_val;
    int num_val;
};

class FooLexer : public yyFlexLexer
{

private:
    int env;

    int cur_line = 1;
    int cur_column = 1;

public:
    FooLexer(std::istream &in, int env) : yyFlexLexer(&in), env(env) {};
    int yylex(void* const yylval, yy::location *const yylloc);
};
