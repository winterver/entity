based on https://blog.csdn.net/qq_42779423/article/details/105954353

TODO
1 more syntaxes. fix bugs.
    1) empty statement
    2) current scope implementation has a bug:
        consider this function: 
            int max(int a, int b)
        a can be accessed in the second parameter slot:
            max(1, a+1); // even if a is not a local variabe,
                        // it is still accessible.
2 string pool, so strings can be compared directly using ==,
    no need to strdup/free over and over again.
3 token stream, no need to parse src over and over again.
4 rewrite in c++. use reflex as lexer.
5 assembly, bytecode, virtual machine.