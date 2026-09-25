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

### Colour a branch that ten values cross

Ten values crossing a branch are coloured or not depending on what else is live
beside them, where nine always are and eleven never can be. Of the shapes tried,
ten crossing with four, twelve, sixteen or twenty values live inside one side
come out coloured, and ten crossing with none or with eight do not.

A phi function's result interferes with each of its arguments, so where the sides
meet the arguments take every register there is and each result needs one that is
not its own argument's. Whether the order the registers are taken in can be made
to find such a colouring, or whether there is none to find, is the thing to settle
first.

### Pass a parameter that has no register on the stack

A function runs out of registers for a reason of its own too, with no branch in it
at all. Every parameter is live where the function is entered, because that is
where the caller leaves it, and spilling one puts its store after that point rather
than before it. The room a spill is meant to buy at the entry is therefore never
bought, and a function of eleven parameters against ten registers spills every one
of them and is still over full.

Parameters past the ones there are registers for have to arrive on the stack, which is
a question for the calling convention rather than for the allocator.

## Tests and benchmarks

### Benchmark a function that branches

The snippets `ChainedValues` and `LiveValues` in [`reg_bench`](lucid/am/reg_bench.cc) are
straight-line, so the graph they build has almost no branching and the join path of
`RunDataflow` barely runs. Avoiding the copies in the analyses moved these benchmarks 3-6%
while moving a chain of diamonds 3.3-6.3x, which is to say the benchmarks do not yet
measure the part of the work that changed.
