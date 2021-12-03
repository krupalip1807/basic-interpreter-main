# basic-interpreter

C inspired language with some modifications. Provided support for new statements and operators. 


a) For-loops:

statement = `"for" "(" expression ";" expression ";" expression ")" statement`

token_t:	T_FORLOOP

stmt_t type:	ST_FORLOOP


b) do-while loops:

statement = `"do" statement "while" "(" expression ")" ";"`

token_t:	T_DWHILE

stmt_t type:	ST_DWHILE


c) Support for an else-clause to while loops (only taken if the while 
condition is never true.)

Addition-	expr1, expr2, welsebody, wifbody


d) Switch statements,

```
statement = "switch" "(" expression ")" statement
          | "case" expression ":" statement
          | "default" ":" statement
```

token_t:	T_SWITCH, T_CASE, T_SCDEFAULT

stmt_t type:	ST_SWITCH, ST_CASE, ST_DEFAULT


e) Operators, 

<=	T_LTE,

!=	T_NEQUAL,

!=	T_GTE,
