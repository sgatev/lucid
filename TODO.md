# TODO

Improvements that are known but not made yet.

## Dataflow

### Reuse the state a vertex already has

`RunDataflow` asks `Transfer` for a whole new state on every visit, compares it against
the one the vertex holds, and move-assigns over it, which frees the tables the old one
had sized. An `Update(State&, ...) -> bool` in place of `Transfer` would let an analysis
grow into that capacity instead of allocating its own, but deciding whether anything
changed needs the old value, so the change detection has to be rethought along with it.

## Hash table

### Let an empty table hold nothing

The default constructor allocates `kInitialCapacity` slots, so an empty table costs a
`malloc` and a state of two sets costs two. Leaving `storage_` null until the first insert
would make the bottom element of a lattice free, which is what `RunDataflow` now starts
every join from. There is also no `Clear`, so a table that is finished with cannot lend
its capacity to the next one.

## Register allocation

### Spill a register a phi function reads

Spilling a register puts a store after where it is written and a load before each
instruction that reads it. A phi function reads its arguments too, and
[`reg.cc`](lucid/am/reg.cc) leaves those alone, as the `TODO` beside them says. A
register a phi reads therefore stays live from where it is written to the end of the
block the phi takes it from, and spilling it makes no room at all.

What follows is that no function that branches can be given registers once it needs to
spill. Twelve values live across a single branch are enough: spilling runs until every
register live at the crowded point has been spilled, makes no room in the process, and
colouring then reaches a register with no colour left for it and asserts. In a build with
the assertions compiled out it takes whichever colour the empty set yields.

A phi reads its argument where control leaves the block that argument comes from, so the
load belongs at the end of that block, and the phi argument becomes the register loaded
into. Until that is written, a test of a branching function that spills cannot be added:
`BranchingValues(12, 1)` in [`reg_test`](lucid/am/reg_test.cc) is the shape of it.

### Pass a parameter that has no register on the stack

A function reaches the same wall for a second reason, with no branch in it at all. Every
parameter is live where the function is entered, because that is where the caller leaves
it, and spilling one puts its store after that point rather than before it. The room a
spill is meant to buy at the entry is therefore never bought, and a function of eleven
parameters against ten registers spills every one of them and is still over full.

Parameters past the ones there are registers for have to arrive on the stack, which is
a question for the calling convention rather than for the allocator.

## Tests and benchmarks

### Benchmark a function that branches

The snippets `ChainedValues` and `LiveValues` in [`reg_bench`](lucid/am/reg_bench.cc) are
straight-line, so the graph they build has almost no branching and the join path of
`RunDataflow` barely runs. Avoiding the copies in the analyses moved these benchmarks 3-6%
while moving a chain of diamonds 3.3-6.3x, which is to say the benchmarks do not yet
measure the part of the work that changed.
