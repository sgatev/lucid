# Architecture

On a high level, the compilation of a program in the Lucid language involves the following stages:

1. [Lexical analysis](lucid/lexer.h) turns Source Code into Token Stream.
2. [Syntax analysis](lucid/parser.h) turns Token Stream into Abstract Syntax Tree.
3. [Type analysis](lucid/type.cc) turns Abstract Syntax Tree into Typed Abstract Syntax Tree.
4. [Control-flow analysis](lucid/cfg.cc) turns Typed Abstract Syntax Tree into Control-flow Graph.
5. [Semantic analysis](lucid/am_gen.cc) turns Control-flow Graph into Abstract Machine Instructions.
6. [Code generation](lucid/arm64_gen.cc) turns Abstract Machine Instructions into Arm64 Assembly Code.
