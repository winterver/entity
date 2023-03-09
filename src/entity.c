#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ERROR(...) do { printf(__VA_ARGS__); exit(-1); } while(0);

/*************************
 * Lexer
 *************************/

#define MAX_NAME_LEN 64

// tokens
enum {
    TYPE = 128, ID, NUM, STR,
    IF, ELSE, WHILE, RETURN,
    EQU, NEQ, LE, GE, OR, AND,
};

int lineno = 1;
char *src;
int token;
typedef union semantics {
    int type;
    char* string;
    double floating;
    long long integer;
} semantics;
semantics token_val;

// forward declaration. in value.c
int get_type(const char* s);
// pool_add() is below next()
char* pool_add(char* s);

void next() {
    char* last_pos;

    while ((token = *src)) {
        ++src;
        if (token == '\n') {                // a new line
            //old_src = src;
            lineno++;
        }
        else if (token == '#') {            // skip comments
            while (*src != 0 && *src != '\n') {
                src++;
            }
            lineno++;
        }
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
            last_pos = src - 1;             // process symbols
            char buf[MAX_NAME_LEN+1];
            buf[0] = token;
            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                if ((src - last_pos) >= MAX_NAME_LEN) {
                    ERROR("(%d) identifer too long\n", lineno);
                }
                buf[src - last_pos] = *src;
                src++;
            }
            buf[src - last_pos] = 0;                 // get symbol name

            #define KEYWROD(str, T) \
                if (!strcmp(str, buf)) { token = T; return; }

            KEYWROD("if", IF);
            KEYWROD("else", ELSE);
            KEYWROD("while", WHILE);
            KEYWROD("return", RETURN);

            #undef KEYWROD

            int type = get_type(buf);
            if (type != -1) {
                token = TYPE;
                token_val.type = type;
                return;
            }

            token = ID;
            token_val.string = pool_add(buf);//strdup(buf);
            return;
        }
        else if (token >= '0' && token <= '9') {        // process numbers
            // TODO: support floating point
            token_val.integer = token - '0';
            while (*src >= '0' && *src <= '9') {
                token_val.integer = token_val.integer * 10 + *src++ - '0';
            }
            token = NUM;
            return;
        }
        else if (token == '\'') {               // parse char
            // TODO: support escape characters
            token_val.integer = *src++;
            token = NUM;
            src++;
            return;
        }
        else if (token == '"' ) {               // parse string
            // TODO: support escape characters
            last_pos = src;
            int count = 0;
            while (*src != 0 && *src != '"') {
                src++;
                count++;          
            }
            if (*src) {
                *src = 0;
                token_val.string = pool_add(last_pos);//strdup(last_pos);
                *src = '"';
                src++;
            }
            token = STR;
            return;
        }
        else if (token == '=') {            // parse '==' and '='
            if (*src == '=') {
                src++;
                token = EQU;
            }
            return;
        }
        else if (token == '!') {               // parse '!='
            if (*src == '=') {
                src++;
                token = NEQ;
            }
            return;
        }
        else if (token == '<') {               // parse '<=',  or '<'
            if (*src == '=') {
                src++;
                token = LE;
            }
            return;
        }
        else if (token == '>') {                // parse '>=',  or '>'
            if (*src == '=') {
                src++;
                token = GE;
            }
            return;
        }
        else if (token == '|') {                // parse  '||'
            if (*src == '|') {
                src++;
                token = OR;
            }
            return;
        }
        else if (token == '&') {                // parse  '&&'
            if (*src == '&') {
                src++;
                token = AND;
            }
            return;
        }
        else if (
            token == '.'
            || token == '*' 
            || token == '/'  
            || token == ';' 
            || token == ',' 
            || token == '+' 
            || token == '-' 
            || token == '(' 
            || token == ')' 
            || token == '{' 
            || token == '}' 
            || token == '[' 
            || token == ']')
        {
            return;
        }
        else if (token == ' ' || token == '\t') {
            /* DO NOTHING */
        }
        else {
            ERROR("(%d) unexpected token: %c\n", lineno, token);
        }
    }
}

void match(int tk) {
    if (token == tk) {
        next();
    }
    else {
        ERROR("(%d) unexpected token: %d, %d required\n", 
            lineno, token, tk);
    }
}

// lexer state management

typedef struct state
{
    char* old_src;
    int old_token;
    semantics old_token_val;
    int old_lineno;
} state;

void save(state* s)
{
    s->old_src = src;
    s->old_token = token;
    s->old_token_val = token_val;
    s->old_lineno = lineno;
}

void restore(const state* s)
{
    src = s->old_src;
    token = s->old_token;
    token_val = s->old_token_val;
    lineno = s->old_lineno;
}

// string pool

typedef struct pool_node
{
    struct pool_node* next;
    char* string;
} pool_node;

pool_node* pool_beg = NULL;
pool_node* pool_end = NULL;

char* _find_string(pool_node* n, char* s)
{
    if (n == NULL)
    {
        return NULL;
    }

    if (!strcmp(n->string, s))
    {
        return n->string;
    }

    return _find_string(n->next, s);
}

char* find_string(char* s)
{
    return _find_string(pool_beg, s);
}

char* pool_add(char* s)
{
    if (pool_end == NULL)
    {
        pool_node* n = malloc(sizeof(pool_node));
        n->next = NULL;
        n->string = strdup(s);
        pool_beg = n;
        pool_end = n;
        return n->string;
    }

    char* f = find_string(s);
    if (f != NULL)
    {
        return f;
    }

    pool_node* n = malloc(sizeof(pool_node));
    n->next = NULL;
    n->string = strdup(s);
    pool_end->next = n;
    pool_end = n;
    return n->string;
}

/*************************
 * Variable Management
 *************************/

#include "value.c"

typedef struct variable
{
    struct variable* next;
    char* name;
    value val;
} variable;

typedef struct scope
{
    struct scope* parent;
    variable* beg;
    variable* end;
} scope;

scope* scope_beg = NULL; // begining & the global scope
scope* scope_end = NULL; // current scope

void new_scope()
{
    scope* n = malloc(sizeof(scope));
    n->beg = NULL;
    n->end = NULL;
    n->parent = scope_end;
    scope_end = n;
    
    if (scope_beg == NULL) {
        scope_beg = n;
    }
}

void free_variable(variable* v)
{
    if (v == NULL)
        return;
    free_variable(v->next);
    //free(v->name);
    free(v);
}

void exit_scope()
{
    scope* orig = scope_end;
    scope_end = orig->parent;
    free_variable(orig->beg);
    free(orig);

    if (scope_end == NULL) {
        scope_beg = NULL;
    }
}

// find variable in variable list
value* _find_variable(variable* v, const char* name)
{
    if (v == NULL)
        return NULL;
    //if (!strcmp(v->name, name))
    if (v->name == name)
        return &v->val;
    return _find_variable(v->next, name);
}

value* find_variable(scope* scp, const char* name)
{
    if (scp == NULL)
        return NULL;
    value* val = _find_variable(scp->beg, name);
    if (val != NULL)
        return val;
    return find_variable(scp->parent, name);
}

void new_variable(char* name, value val)
{
    if (scope_end == NULL)
        ERROR("no scope\n");

    // search in current scope
    if (_find_variable(scope_end->beg, name) != NULL)
    {
        //bug: lineno is not accurate if new_variable() call by user.
        ERROR("(%d) redefinition of variable %s\n", lineno, name);
    }

    // if it is an uninitialized list
    if (scope_end->end == NULL)
    {
        // initialize it
        variable* var = malloc(sizeof(variable));
        var->next = NULL;
        var->name = name;//strdup(name);
        var->val = val;
        scope_end->beg = var;
        scope_end->end = var;
        return;
    }

    // append to end of list
    variable* var = malloc(sizeof(variable));
    var->next = NULL;
    var->name = name;//strdup(name);
    var->val = val;
    scope_end->end->next = var;
    scope_end->end = var;
}

// search variable with the given name
value* get_variable(const char* name)
{
    value* val = find_variable(scope_end, name);
    if (val == NULL)
        ERROR("(%d) no such variable: %s\n", lineno, name);
    return val;
}

/*************************
 * Function Management
 *************************/

typedef struct param
{
    struct param* next;
    int type;
    char* name;
} param;

typedef struct function
{
    struct function* next;
    int type; // return type
    char* name;
    param* params; // when appending native functions, you need to
                    // construct this by yourself.
    state stat; // token = '{', the start of the function body
    value (*fp)(); // function pointer to native function
                    // NULL by default. if not NULL, the native
                    // function will be called, and stat is ignored.
                    // when fp doesn't return a value, let return_value.type = TYPE_VOID
} function;

// linked list to global functions
function* funcs_beg = NULL;
function* funcs_end = NULL;

function* _find_function(function* fun, const char* name)
{
    if (fun == NULL)
        return NULL;

    //if (!strcmp(fun->name, name))
    if (fun->name == name)
        return fun;

    return _find_function(fun->next, name);
}

function* find_function(const char* name)
{
    return _find_function(funcs_beg, name);
}

function* get_function(const char* name)
{
    function* fun = find_function(name);
    if (fun == NULL)
    {
        ERROR("(%d) no such function %s\n", lineno, name);
    }
    return fun;
}

void new_function(
    int type, 
    char* name, 
    param* params, 
    state stat,
    value (*fp)()
)
{
    if (find_function(name) != NULL)
    {
        //bug: lineno it not accurate here.
        ERROR("(%d) redefinition of function %s\n", lineno, name);
    }

    if (funcs_end == NULL)
    {
        function* fun = malloc(sizeof(function));
        fun->next = NULL;
        fun->type = type;
        fun->name = name;//strdup(name);
        fun->params = params;
        fun->stat = stat;
        fun->fp = fp;
        funcs_beg = fun;
        funcs_end = fun;
        return;
    }

    function* fun = malloc(sizeof(function));
    fun->next = NULL;
    fun->type = type;
    fun->name = name;//strdup(name);
    fun->params = params;
    fun->stat = stat;
    fun->fp = fp;
    funcs_end->next = fun;
    funcs_end = fun;
}

/*************************
 * Parser & Interpreter
 *************************/

// allow function call evaluation
// set to true when global variables are all processed
//int allow_func_eval = 0;

/*
exp -> term1 { op1 term1 }
term1 -> term2 { op2 term2 }
term2 -> term3 { op3 term3 }
    ......
term? -> factor { op? factor }
factor -> NUM | ref | call | ( exp )
op1 -> '<' | '>'
op2 -> '+' | '-'
op? -> '*' | '/'

the larger the number behind 'op' is, the higher precedence the operators have.
*/

value factor();
value term2();
value term1();
value expression();

value* reference();
value call();
value block();

value factor() {
    value out;
    if (token == '(') {
        match('(');
        out = expression();
        match(')');
    }
    else if(token == NUM) {
        out.type = TYPE_INT;
        out.i32 = token_val.integer;
        match(NUM);
    }
    else if (token == ID) {
        state s;
        save(&s);

        match(ID);
        if (token == '(') {
            restore(&s);
            out = call();
        }
        else {
            restore(&s);
            out = *reference();
        }
    }
    return out;
}

value term2() {
    value lhs = factor(), rhs;
    while (token == '*' || token == '/') {
        if (token == '*') {
            match('*');
            rhs = factor();
            binary_op(&lhs, &lhs, '*', &rhs);
        }
        else {
            match('/');
            rhs = factor();
            binary_op(&lhs, &lhs, '/', &rhs);
        }
    }
    return lhs;
}

value term1() {
    value lhs = term2(), rhs;
    while (token == '+' || token == '-') {
        if (token == '+') {
            match('+');
            rhs = term2();
            binary_op(&lhs, &lhs, '+', &rhs);
        }
        else {
            match('-');
            rhs = term2();
            binary_op(&lhs, &lhs, '-', &rhs);
        }
    }
    return lhs;
}

value expression() {
    value lhs = term1(), rhs;
    while (token == '<' || token == '>') {
        if (token == '<') {
            match('<');
            rhs = term1();
            binary_op(&lhs, &lhs, '<', &rhs);
        }
        else {
            match('>');
            rhs = term1();
            binary_op(&lhs, &lhs, '>', &rhs);
        }
    }
    return lhs;
}

// ref -> ID { '.' ID }
value* reference()
{
    char* name = token_val.string;
    match(ID);
    value* ref = get_variable(name);
    //free(name);

    /*
    while(token == '.')
    {

    }
    */

    return ref;
}

// 在多重嵌套的block中返回时设为true
// 这样就能快速跳出递归的block()
// 每次call()之后设为false
int retflag = 0;

value call()
{
    value ret;

    char* name = token_val.string;
    match(ID);
    
    // get the function and its parameter list
    function* fun = get_function(name);
    param* par = fun->params;
    
    // find number of arguments we need to pass
    int n_args = 0;
    for(param* p = par; p; p = p->next)
        n_args++;

    scope* bak = scope_end;
    // 先生成一个新scope，暂时挂到bak下面。
    new_scope();
    // 保存新的scope
    scope* neo = scope_end;

    // number of arguments passed to the function
    int n_passed = 0;

    match('(');
    if (token != ')')
    {
    NextArg:
        if (par == NULL)
        {
            ERROR("(%d) too many arguments to function %s\n",
                lineno, name);
        }
        // 计算表达式时，切换到原来的scope。
        scope_end = bak;
        value val = expression();
        if (val.type != par->type)
        {
            ERROR("(%d) wrong type provided to function %s at pos %d, %s required, but %s provided\n",
                lineno, name, n_passed+1, type_name(par->type), type_name(val.type));
        }
        // 传参时切换到新的scope
        scope_end = neo;
        new_variable(par->name, val);
        par = par->next;
        n_passed++;

        if (token == ',')
        {
            match(',');
            goto NextArg;
        }
    }
    match(')');

    if (n_passed != n_args)
    {
        ERROR("(%d) too few arguments to function %s, %d required, but %d provided\n",
            lineno, name, n_args, n_passed);
    }

    // save return address
    state s;
    save(&s);

    // 传参结束，scope_end=neo,新scope挂到scope_beg下。
    scope_end->parent = scope_beg;
    // finally, call it!
    restore(&fun->stat);
    ret = block();

    if (ret.type != fun->type)
    {
        ERROR("(%d) function %s returns wrong type\n", lineno, name);
    }

    restore(&s);

    exit_scope();
    scope_end = bak;

    // see begining of call()
    retflag = 0;

    //free(name);
    return ret;
}

// block -> '{' { stat } '}'
// stat -> var | call | assign
// var -> TYPE name { ',' name } ';'
// call -> ID '(' ID ID { ',' ID ID } ')'
// assign -> ref '=' expr

void var();
void skip_block();
void assign()
{
    value* left = reference();
    match('=');
    value right = expression();
    if (left->type != right.type)
    {
        ERROR("(%d) assignment on different types\n", lineno);
    }
    *left = right;
}

value block()
{
    value ret;
    memset(&ret, 0, sizeof(value));
    ret.type = TYPE_VOID;

    match('{');
    while(token != '}')
    {
        state s;
        save(&s);

        // empty statement
        if (token == ';')
        {
            match(';');
        }
        else if (token == TYPE)
        {
            var();
        }
        else if (token == ID)
        {
            match(ID);
            if (token == '(')
            {
                restore(&s);
                call();
                match(';');
            }
            else
            {
                restore(&s);
                assign();
                match(';');
            }
        }
        else if (token == RETURN)
        {
            match(RETURN);
            if (token == ';')
            {
                match(';');
                retflag = 1;
                return ret;
            }
            else
            {
                ret = expression();
                match(';');
                retflag = 1;
                return ret;
            }
        }
        else if (token == IF) {
        NextIf:
            match(IF);
            match('(');
            value val = expression();
            match(')');

            // TODO: conversion to bool
            if (val.i32) {
                new_scope();
                ret = block();
                exit_scope();
                if (retflag)
                {
                    return ret;
                }
            }
            else {
                skip_block();
            }

            if (token == ELSE) {
                match(ELSE);
                if (!val.i32) {
                    // improve this, goto is dangerous.
                    if (token == IF)
                    {
                        goto NextIf;
                    }
                    else {
                        new_scope();
                        ret = block();
                        exit_scope();
                        if (retflag)
                        {
                            return ret;
                        }
                    }
                }
                else {
                    skip_block();
                }
            }
        }
    }
    match('}');
    return ret;
}

/*
program -> { var } { func }
func -> TYPE ID '(' [ ID ID { ',' ID ID } ] ')' block
var -> TYPE name { ',' name } ';'
name -> ID | ID '=' expr
*/

void var()
{
    int type = token_val.type;
    match(TYPE);

NextVar:
    char* name = token_val.string;
    match(ID);

    if (token == '=')
    {
        match('=');
        value val = expression();
        type_convert(&val, type);
        new_variable(name, val);
    }
    else
    {
        value val;
        memset(&val, 0, sizeof(val));
        val.type = type;
        new_variable(name, val);
    }
    //free(name);

    if (token == ',')
    {
        match(',');
        goto NextVar;
    }

    match(';');
}

void skip_block()
{
    //if(token == '{')
    //    token = *src++;
    match('{');
    int count = 0;
    while (token && !(token == '}' && count == 0)) {
        if (token == '}') count++;
        if (token == '{') count--;
        token = *src++;
    }
    match('}');
}

void func()
{
    int type = token_val.type;
    match(TYPE);
    char* name = token_val.string;
    match(ID);

    param* params_beg = NULL;
    param* params_end = NULL;
    // match params
    match('(');
    if (token != ')')
    {
    NextParam:
        int param_type = token_val.type;
        match(TYPE);
        char* param_name = token_val.string;
        match(ID);

        param* p = malloc(sizeof(param));
        p->next = NULL;
        p->type = param_type;
        p->name = param_name;//strdup(param_name);
        
        if (params_end == NULL)
        {
            params_beg = p;
            params_end = p;
        }
        else
        {
            params_end->next = p;
            params_end = p;
        }
        //free(param_name);

        if (token == ',')
        {
            match(',');
            goto NextParam;
        }
    }
    match(')');

    state stat;
    save(&stat);

    skip_block();

    new_function(
        type,
        name,
        params_beg,
        stat,
        NULL
    );
    //free(name);
}

void program()
{
    new_scope();
    next();

    state s;

    // process global variables
    while(token)
    {
        // save lexer state
        save(&s);
        match(TYPE);
        match(ID);

        if (token == '=' 
            || token == ',' 
            || token == ';')
        {
            // restore lexer state
            restore(&s);
            var();
        }
        else {
            restore(&s);
            break;
        }
    }

    while(token)
    {
        func();
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        ERROR("usage: entity <source>\n");
    }

    FILE *f = fopen(argv[1], "r");
    if (f == NULL)
    {
        ERROR("no such file\n");
    }

    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* orig;
    orig = src = malloc(len+1);
    memset(src, 0, len+1);
    fread(src, 1, len, f);

    program();

    value result;
    char* str = find_string("main");
    function* entry = find_function(str);

    if (entry != NULL) {
        new_scope();
        restore(&entry->stat);
        result = block();
        exit_scope();
    }
    else {
        ERROR("main() not found\n");
    }

    printf("%d\n", result.i32);

    free(orig);
    return 0;
}