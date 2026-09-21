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

### Colour the registers a block's phi functions name

[`ColorInterferenceGraph`](lucid/am/reg.cc) gathers the registers to colour by walking the
instructions of every block, and the loop that gathers the ones a block's phi functions
name sits inside that walk rather than beside it. A block holding phi functions and no
instructions therefore contributes none of them, `ColorInterferenceGraph` returns no
colour for those registers, and `UpdateRegister` asserts on the first one it meets. An
optimised build, where that assertion is compiled out, runs on instead: a function that
takes minutes rather than the milliseconds around it.

Branches nested three deep are enough to make a block like that:

```
fun main(a: Int32): Int32 {
  val v0: Int32 = 0
  ...
  if a == 3 { if a == 2 { if a == 1 { &v0 = 1 } else { &v0 = 2 } ... } ... }
  return v0
}
```

Moving the loop out beside the walk rather than inside it compiles that function and the
same shape nested four and six deep, so it is the cause rather than a symptom. Whether it
is the whole fix is unchecked: it also changes what is gathered for every block that has
both phi functions and instructions, where today the phi registers are gathered once per
instruction instead of once.

## Tests and benchmarks

### Benchmark a function that branches

The snippets `ChainedValues` and `LiveValues` in [`reg_bench`](lucid/am/reg_bench.cc) are
straight-line, so the graph they build has almost no branching and the join path of
`RunDataflow` barely runs. Avoiding the copies in the analyses moved these benchmarks 3-6%
while moving a chain of diamonds 3.3-6.3x, which is to say the benchmarks do not yet
measure the part of the work that changed.
