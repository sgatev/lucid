# Architecture

On a high level, the compilation of a program with the Lucid toolchain involves the
following stages:

1. [Lexical analysis](lucid/syntax/lexer.h) turns Source Code into Token Stream.
2. [Syntax analysis](lucid/syntax/parser.h) turns Token Stream into Abstract Syntax Tree.
3. [Type analysis](lucid/syntax/type.cc) turns Abstract Syntax Tree into Typed Abstract
   Syntax Tree.
4. [Comp analysis](lucid/syntax/comp.cc) identifies expressions in the Typed Abstract
   Syntax Tree that must be evaluated during compilation.
5. [Control flow analysis](lucid/syntax/cfg.cc) turns Typed Abstract Syntax Tree into
   Typed Abstract Syntax Control Flow Graph.
6. [Variable analysis](lucid/syntax/ssa.cc) converts Typed Abstract Syntax Control Flow
   Graph into Static Single-Assignment form.
7. [Semantic analysis](lucid/am/translator.cc) turns Typed Abstract Syntax Control Flow
   Graph into Abstract Machine Control Flow Graph.
8. [Register analysis](lucid/am/reg.cc) reduces the number of registers used in Abstract
   Machine Control Flow Graph.
9. [Code generation](lucid/arm64/translator.cc) turns Abstract Machine Control Flow Graph
   into ARM64 Machine Code.
