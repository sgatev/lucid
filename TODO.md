# TODO

* Clean up AST and parsing:
  * Add a statement that enables calling functions without using their results, e.g. `do printf("hello")`.
  * Remove the `Expr` type from the `Stmt` variant.
  * Store `Stmt`, `Expr`, and `Type` objects on separate arenas.
