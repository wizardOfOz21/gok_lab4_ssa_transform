%{
    #include "parser.tab.hpp"
    #include "lexer.hpp"
    
    #undef  YY_DECL
    #define YY_DECL int FooLexer::yylex(void *const yylval, yy::location *const yylloc)

    typedef yy::parser::token token;

    #define YY_USER_ACTION \
    { \
        int i; \
        yylloc->begin.line = cur_line; \
        yylloc->begin.column = cur_column; \
        for (int i = 0; i < yyleng; ++i) { \
            if (yytext[i] == '\n') { \
                cur_line += 1; \
                cur_column = 1; \
            } else { \
                cur_column += 1; \
            } \
        } \
        yylloc->end.line = cur_line; \
        yylloc->end.column = cur_column; \
    }
%}


%option c++ noyywrap
%option yyclass="FooLexer"

%%

[\r\t\n ]+

func      return token::_FUNC_;
var       return token::_VAR_;
entry     return token::_ENTRY_;
return    return token::_RETURN_;
if        return token::_IF_;
else      return token::_ELSE_;
while      return token::_WHILE_;

\{\*.*\*\}

[0-9]+                  { ((ValueType*)yylval)->num_val = std::stoi(std::string(yytext, yytext + yyleng)); }   return token::NUMBER;
[a-zA-Z][a-zA-Z0-9_]*    { ((ValueType*)yylval)->ident_val = new std::string(yytext, yytext + yyleng); }        return token::IDENT;

.   {return *yytext; }
