%require "3.2"
%language "c++"

%code requires {
    #include "lexer.hpp"

    #include <string>
    #include "ast/program.hpp"
    #include "ast/if.hpp"
    #include "ast/while.hpp"
}

%locations
%define api.location.file "location.hpp"

%parse-param {ProgramAST* &ast_root}

%parse-param {FooLexer &lexer}

%header

%code {
    #define yylex lexer.yylex
}

%token <ident_val> IDENT
%token <num_val> NUMBER

%token _VAR_ "var"
%token _FUNC_ "func"
%token _ENTRY_ "entry"
%token _RETURN_ "return"
%token _IF_ "if"
%token _ELSE_ "else"
%token _WHILE_ "while"

%union {
    std::string* ident_val;
    int num_val;

    vector<DeclAST*>* DeclAST_list;
    DeclAST* DeclAST;
    vector<OperatorAST*>* OperatorAST_list;
    OperatorAST* OperatorAST;

    ExprAST* ExprAST;

    vector<string>* strings;

    Block* Block;
    IFOperatorAST* IFOperatorAST;
    WhileAST* WhileAST;
    
    // не менять порядок
    vector<LocalVarAST*>*   LocalVarAST_list;
    LocalVarAST*            LocalVarAST;
    LocalVarDeclOpAST*      LocalVarDeclOpAST;
}

%type <DeclAST_list> var_decl var_list decl decls

%type <LocalVarAST>         local_var;
%type <LocalVarAST_list>    local_var_list;
%type <LocalVarDeclOpAST>   local_var_decl;

%type <DeclAST> var entry_decl func_decl
%type <Block> block
%type <OperatorAST_list> operator_list

%type <strings> params_list

%type <OperatorAST> operator assignment return_statement 
%type <ExprAST> expr t f p

%type <IFOperatorAST> if_operator;
%type <WhileAST> while_operator;

%%

program: 
    decls ';' { ast_root = new ProgramAST(*$1); }

decls: 
      decls ';' decl    { $1->insert($1->end(), $3->begin(), $3->end()); $$ = $1; } 
    | decl              { $$ = $1; }
    | %empty            { $$ = new vector<DeclAST*>(); }

decl: 
      var_decl
    | func_decl     { $$ = new vector<DeclAST*>({$1}); } 
    | entry_decl    { $$ = new vector<DeclAST*>({$1}); }

var_decl: 
    "var" var_list { $$ = $2; }

var_list: 
    var_list ',' var 
    {
        $1->push_back($3);
        $$ = $1;
    } 
    | var 
    { 
        $$ = new vector<DeclAST*>({$1});
    }

var: 
    IDENT '=' expr 
    { 
        $$ = new VarAST(*$1, $3);
        delete $1;
    }

entry_decl: 
    "entry" '=' IDENT 
    { 
        $$ = new EntryAST(*$3); 
        delete $3;
    }

func_decl:
    "func" IDENT params_list block 
    {
        $$ = new FuncAST(*$2, *$3, $4);
        delete $2;
    }

params_list: 
    '(' ')'
    {
        $$ = new vector<string>();
    }

block:
    '{' operator_list ';' '}' 
    { 
        $$ = new Block(*$2);
    }

operator_list: 
        operator_list ';' operator 
        { 
            $1->push_back($3); 
            $$ = $1; 
        }
        | operator 
        { 
            $$ = new vector<OperatorAST*>({$1});
        }
        | %empty 
        { 
            $$ = new vector<OperatorAST*>(); 
        }

operator: 
          assignment 
        | return_statement 
        | local_var_decl
        | if_operator
        | while_operator
        | expr { $$ = new ExprOperatorAST($1); }

local_var_decl: 
    "var" local_var_list { $$ = new LocalVarDeclOpAST(*$2); }

local_var_list: 
    local_var_list ',' local_var 
    {
        $1->push_back($3);
        $$ = $1;
    } 
    | local_var 
    { 
        $$ = new vector<LocalVarAST*>({$1});
    }

local_var: 
    IDENT '=' expr 
    {
        $$ = new LocalVarAST(*$1, $3);
        delete $1;
    }

if_operator:
    "if" '(' expr ')' block "else" block
    {
        $$ = new IFOperatorAST($3, $5, $7);
    }

while_operator: 
    "while" '(' expr ')' block
    {
        $$ = new WhileAST($3, $5);
    }

assignment: 
    IDENT '=' expr 
    {
        $$ = new AssignAST(*$1,$3);
        delete $1;
    }

return_statement: 
    "return" expr 
    {
        $$ = new ReturnAST($2);
    }

expr:   
       t '+' expr  
    {
        $$ = new BinaryOpExprAST('+', $1, $3); 
    }
    |  t '-' expr  
    {
        $$ = new BinaryOpExprAST('-', $1, $3); 
    }
    |  t

t:   
    f '*' t  
    { 
        $$ = new BinaryOpExprAST('*', $1, $3);
    }
    |  f '/' t 
    { 
        $$ = new BinaryOpExprAST('/', $1, $3);
    }
    |  f

f: 
    '-' p  
    { 
        $$ = new UnaryOpExprAST('-', $2);
    }
    |  p

p:   
     IDENT 
    { 
        $$ = new VariableAST(*$1);
        delete $1;
    } 
    | NUMBER 
    { 
        $$ = new NumberAST($1); 
    }
    | IDENT '(' ')' 
    { 
        $$ = new FCallAST(*$1); 
        delete $1;
    }
    | '(' expr ')' 
    { 
        $$ = $2; 
    }

%%

void yy::parser::error(const yy::location& loc, const std::string &message)
{
    std::cerr << "Error: " << message << " on " << loc << std::endl;
}
