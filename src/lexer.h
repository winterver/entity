#ifndef ENTITY_LEXER_H
#define ENTITY_LEXER_H

// tokens
enum {
    TYPE = 128, ID, NUM, STR,
    IF, ELSE, WHILE, RETURN,
    EQU, NEQ, LE, GE, OR, AND,
};

typedef union semantics {
    int type;
    char* string;
    double floating;
    long long integer;
} semantics;

extern int lineno;
extern char *src;
extern int token;
extern semantics token_val;

void next();
void match(int tk);

typedef struct state
{
    char* old_src;
    int old_token;
    semantics old_token_val;
    int old_lineno;
} state;

void save(state* s);
void restore(const state* s);

// string pool
char* find_string(char* s);

#endif