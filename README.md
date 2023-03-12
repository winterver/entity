### TODO
- [ ] more syntaxes. fix bugs.
  - [ ] variadic parameter
  - [ ] unary operator: -, ++, --
  - [ ] interface for native function registration.
  - [ ] for statement.
  - [ ] complete arithmetic operations for more types.
- [ ] rewrite in c++. use reflex as lexer.
- [ ] assembly, bytecode, virtual machine.
- [ ] jit
#### Accomplished
- [x] more syntaxes. fix bugs.
  - [x] empty statement.
  - [x] anonymous block.
  - [x] while & do-while statement.
  - [x] break & continue.
  - [x] member attachment for entity object.
- [x] string pool, so strings can be compared directly using ==, no need to strdup/free over and over again.
- [x] token stream, no need to parse src over and over again.
### Links
this project is inspired by https://blog.csdn.net/qq_42779423/article/details/105954353