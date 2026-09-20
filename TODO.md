# TODO

Improvements that are known but not made yet.

## Dataflow

### Build a block's live-in without copying its live-out

`Transfer` fills `live_out` from the joined prior state and then copies it wholesale into
`live_in` before walking the block backwards, in
[`AbstractMachineLivenessAnalysis`](lucid/am/liveness.cc),
[`SyntaxLivenessAnalysis`](lucid/syntax/liveness.cc) and, as `vars_in` into `vars_out`, in
[`SyntaxReachabilityAnalysis`](lucid/syntax/reachability.cc). The same copy is made again
in `FindRegToSpill` in [`reg.cc`](lucid/am/reg.cc). It is a whole hash table per block
visit, and now the largest copy left in an analysis.

Both sets could be filled in one pass over the priors, which means handing `Transfer` the
prior states themselves rather than one state joined from them. That buys the copy at the
price of `Join`, and with it the join-semilattice the framework is written around, so it
is a design decision rather than a cleanup.

### Reuse the state a vertex already has

`RunDataflow` asks `Transfer` for a whole new state on every visit, compares it against
the one the vertex holds, and move-assigns over it, which frees the tables the old one
had sized. An `Update(State&, ...) -> bool` in place of `Transfer` would let an analysis
grow into that capacity instead of allocating its own, but deciding whether anything
changed needs the old value, so the change detection has to be rethought along with it.

## Hash table

### Destroy the values a table holds

[`~HashTable`](lucid/core/container/hash_table.h) frees `storage_` without running any
destructor, and `Remove` leaves the value it moved out of in its slot, which is never
destroyed either. Every `V` in the tree today is trivially destructible, so nothing leaks
yet, but `HashMap<Reg, HashSet<Reg>>` — the interference graph — is not: destroying
one would leak the storage of every set in it.

### Let an empty table hold nothing

The default constructor allocates `kInitialCapacity` slots, so an empty table costs a
`malloc` and a state of two sets costs two. Leaving `storage_` null until the first insert
would make the bottom element of a lattice free, which is what `RunDataflow` now starts
every join from. There is also no `Clear`, so a table that is finished with cannot lend
its capacity to the next one.

### Count only the tombstones a table still has

`resize` rebuilds the table by re-probing the full slots, which drops the slots a removal
emptied, and then takes `non_empty_slots_count_` from the table it rebuilt from, where
those slots were still counted. The new table therefore believes it is fuller than it is
and resizes earlier than it needs to.

## Tests and benchmarks

### Benchmark a function that branches

The snippets `ChainedValues` and `LiveValues` in [`reg_bench`](lucid/am/reg_bench.cc) are
straight-line, so the graph they build has almost no branching and the join path of
`RunDataflow` barely runs. Avoiding the copies in the analyses moved these benchmarks 3-6%
while moving a chain of diamonds 3.3-6.3x, which is to say the benchmarks do not yet
measure the part of the work that changed.
