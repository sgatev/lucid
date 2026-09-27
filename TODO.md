# TODO

Improvements that are known but not made yet.

## Dataflow

### Reuse the state a vertex already has

`RunDataflow` asks `Transfer` for a whole new state on every visit, compares it against
the one the vertex holds, and move-assigns over it, which frees the tables the old one
had sized. An `Update(State&, ...) -> bool` in place of `Transfer` would let an analysis
grow into that capacity instead of allocating its own, but deciding whether anything
changed needs the old value, so the change detection has to be rethought along with it.
A state that held a hash table would also want a `Clear` on it, which there is none of.

## Register allocation

### Keep a spilt phi function in memory

A phi function reads each argument where control leaves the block that argument
comes from, so every argument has to be in a register at that point. The point
where the sides of a branch meet therefore needs as many registers as there are
values crossing it, and no amount of spilling changes that: spilling a value a phi
reads now loads it back at that point, which buys room everywhere else but not
there. Eleven values crossing a branch against ten registers cannot be coloured.

A phi whose result is spilt needs no register at all if its arguments are stored
to the result's own slot, which is to say if the result and its arguments are
given one slot between them.

This is what stops more values crossing a branch than there are registers. Ten
crossing are coloured; eleven are not, however crowded the sides are.

## Tests and benchmarks

### Benchmark a function that branches

The snippets `ChainedValues` and `LiveValues` in [`reg_bench`](lucid/am/reg_bench.cc) are
straight-line, so the graph they build has almost no branching and the join path of
`RunDataflow` barely runs. Avoiding the copies in the analyses moved these benchmarks 3-6%
while moving a chain of diamonds 3.3-6.3x, which is to say the benchmarks do not yet
measure the part of the work that changed.
