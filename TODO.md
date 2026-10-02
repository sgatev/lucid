# TODO

Improvements that are known but not made yet.

## Register allocation

### Search for what to spill without starting over

Spilling is where the time goes on a function that branches. A chain of 300 values
compiles in about 17ms of work; the same 300 written on both sides of an `if` and read
after takes about 970ms, of which spilling is 89%. It grows faster than the function
does: 30, 60 and 120 diamonds cost 0.8ms, 3.4ms and 15.1ms of spilling, roughly
quadrupling as the function doubles.

`FindRegToSpill` walks every block and builds a map of where each register is next read
as it goes, and the loop that drives it calls it again from nothing after every spill.
One spill per diamond over an *n* block function is *n* walks of the whole thing. What
is live is already carried from one spill to the next rather than worked out again;
where the crowded points are is not, and it is the last part of the loop that still
starts over.

`AllocateDiamonds15` and `AllocateDiamonds30` in [`reg_bench`](lucid/am/reg_bench.cc)
measure it: twice the branches take 3.7ms and 14.3ms, nearly four times as long. A
search that kept what it found would bring the second closer to twice the first.
