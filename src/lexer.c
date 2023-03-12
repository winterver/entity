#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "lexer.h"
#define MAX_NAME_LEN 64
#define ERROR(...) do { printf(__VA_ARGS__); exit(-1); } while(0);

char *src;
int token;
int lineno = 1;
semantics token_val;

// in value.c
int get_type(const char* s);
// forward declaration
char* pool_add(char* s);

void lex() {
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
            KEYWROD("do", DO);
            KEYWROD("for", FOR);
            KEYWROD("return", RETURN);

            #undef KEYWROD

            int type = get_type(buf);
            if (type != -1) {
                token = TYPE;
                token_val.type = type;
                return;
            }

            token = ID;
            token_val.string = pool_add(buf);
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
                token_val.string = pool_add(last_pos);
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

typedef struct token_struct
{
    struct token_struct* next;
    int token;
    int lineno;
    semantics token_val;
} token_struct;

token_struct* stream_beg = NULL;
token_struct* stream_end = NULL;
token_struct* stream_cur = NULL;

void init_lex()
{
    // scan all tokens and store them
    do
    {
        lex();

        token_struct* t = malloc(sizeof(token_struct));
        t->next = NULL;
        t->token = token;
        t->lineno = lineno;
        t->token_val = token_val;

        if (stream_end == NULL)
        {
            stream_beg = t;
            stream_end = t;
        }
        else
        {
            stream_end->next = t;
            stream_end = t;
        }
    } while (token);

    // load the first token
    stream_cur = stream_beg;
    token = stream_cur->token;
    lineno = stream_cur->lineno;
    token_val = stream_cur->token_val;
}

void next()
{
    if (stream_cur != stream_end)
    {
        // go to next token, if it's not end
        stream_cur = stream_cur->next;
    }

    token = stream_cur->token;
    lineno = stream_cur->lineno;
    token_val = stream_cur->token_val;
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

token_struct* save()
{
    return stream_cur;
}

void restore(token_struct* s)
{
    stream_cur = s;
    token = s->token;
    token_val = s->token_val;
    lineno = s->lineno;
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