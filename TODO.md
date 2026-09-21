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

## Tests and benchmarks

### Benchmark a function that branches

The snippets `ChainedValues` and `LiveValues` in [`reg_bench`](lucid/am/reg_bench.cc) are
straight-line, so the graph they build has almost no branching and the join path of
`RunDataflow` barely runs. Avoiding the copies in the analyses moved these benchmarks 3-6%
while moving a chain of diamonds 3.3-6.3x, which is to say the benchmarks do not yet
measure the part of the work that changed.
