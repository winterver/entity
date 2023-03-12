#ifndef ENTITY_LEXER_H
#define ENTITY_LEXER_H

// tokens
enum {
    TYPE = 128, ID, NUM, STR,
    IF, ELSE, WHILE, DO, FOR, CONTINUE, BREAK, RETURN,
    EQU, NEQ, LE, GE, OR, AND,
};

typedef union semantics {
    int type;
    char* string;
    double floating;
    long long integer;
} semantics;

extern char *src;
extern int token;
extern int lineno;
extern semantics token_val;

void init_lex();
void next();
void match(int tk);

typedef struct token_struct token_struct;

token_struct* save();
void restore(token_struct* s);

// string pool
char* find_string(char* s);

#endif