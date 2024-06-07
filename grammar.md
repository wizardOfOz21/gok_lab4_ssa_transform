#### Лексика
```

IDENT = [a-zA-Z][a-zA-Z0-9_]*
NUMBER = [0-9]+

```
#### Грамматика
```
program  =  decls $
decls = decls ; decl | decl | e
decl = var_decl | func_decl | entry_decl

var_decl = 'VAR' var_list
var_list = var_list , var | var | e
var = IDENT  '=' expr

func_decl = 'FUNC' IDENT params_list body
body = { operator_list ; }
params_list = (<пока пусто>)

entry_decl = 'ENTRY' '=' IDENT

operator_list = operator_list ; operator  | operator | e
operator = assignment | return | expr | var_decl | if | while

// 0 - false, остальное – true
if = 'IF' ( expr ) body 'ELSE' body
while = 'WHILE' ( expr ) body

assignment = IDENT '=' expr
return = 'RETURN' expr

expr    =   t + expr   |  t - expr  |  t
t   =   f * t  |  f / t |  f
f   =  -p  |  p
p   =   IDENT | NUMBER |  (expr)

```
