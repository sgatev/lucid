# The Lucid language

What the compiler in this repository accepts today, written down. Anything listed under
[Limits](#limits) is a gap rather than a decision, and [TODO.md](TODO.md) carries the
ones that are being worked on.

## A program

A source file is a sequence of definitions. There are no imports, no modules and no
declarations separate from definitions. A program is one file.

```
fun square(n: Int32): Int32 {
  return n * n
}

fun main(): Int32 {
  return square(7)
}
```

The definitions are compiled one at a time, in the order they are written, and each one
is finished before the next is read. **A function must therefore be defined above every
call to it.** The function named `main` is where the program starts, and the value it
returns is the process's exit status.

A program without a `main` is rejected. `main` takes no parameters, since nothing is
passed to it, and returns an `Int32`, an `Int64` or a `Bool`, whose lowest byte becomes the
exit status; `true` is 1 and `false` is 0.

## Lexical structure

The source is a sequence of bytes ending in a null byte. Nothing in the language depends
on the layout of lines. A newline is whitespace like any other, and there is no statement
terminator. A carriage return is whitespace too, so a file whose lines end in `\r\n`
reads the same as one whose lines end in `\n`, and a UTF-8 byte order mark at the head of
a file is passed over.

Outside comments and strings the source is ASCII. A byte from `0x80` up anywhere else is
an error where it stands, as is any ASCII byte the language has no use for, such as `@`.

**Comments** run from a `#` to the end of the line.

**Identifiers** begin with a letter or an underscore and continue with letters,
underscores and digits.

**Numbers** are a run of decimal digits. There is no sign, no radix prefix, no separator
and no floating-point literal.

**Strings** are bytes between two `"` characters. A string may run across lines, and it
has no escape for `"` itself, so a string cannot contain one. The other escapes are the
POSIX ones, interpreted when the string is written into the program:

| Escape | Byte | | Escape | Byte |
|---|---|---|---|---|
| `\\` | backslash | | `\r` | carriage return |
| `\a` | alert | | `\t` | tab |
| `\b` | backspace | | `\v` | vertical tab |
| `\f` | form feed | | `\0` … `\777` | up to three octal digits |
| `\n` | newline | | | |

**Punctuation** is `=` `(` `)` `{` `}` `[` `]` `:` `,` `+` `-` `*` `/` `%` `>` `<` `.`
`|` `!`. The operators `==`, `!=`, `>=` and `<=` are written as their first
character followed immediately by `=`, with no space between them.

**Keywords** are not reserved. `fun`, `val`, `mut`, `comp`, `if`, `else`, `loop`,
`break`, `return`, `do`, `true` and `false` are ordinary identifiers that the parser
recognises where a definition or a statement begins; `and` and `or` are ones it
recognises where an operator would stand, between two expressions; and `Type` is
recognised only in a type definition. Nothing stops a variable being called `loop`.

## Types

Six types are built in:

| Type | Size | Notes |
|---|---|---|
| `Void` | 0 | the absence of a value |
| `Bool` | 4 | `true` and `false` |
| `Int32` | 4 | signed |
| `Int64` | 8 | signed |
| `Double` | 8 | named and sized, but no literal syntax reaches it |
| `String` | 8 | a pointer to bytes |

**An array type** is written `T[n]`, where `n` is an integer literal. The length is part
of the type.

```
val buffer: Int32[101]
```

**A tuple type** is defined at the top level and has named fields:

```
val Point: Type = (x: Int32, y: Int32)
```

`val NAME: Type = (…)` is the only form of type definition, and the word `Type` is
required in it. Elsewhere `val` declares a variable, which is a different thing entirely.

## Definitions

```
fun name(param: Type, …): ResultType { … }
comp fun name(param: Type, …): ResultType { … }
val Name: Type = (field: Type, …)
```

A function takes zero or more parameters, always names its result type, and has a body in
braces. **A `mut` before a parameter's name says the body may write to it**, as it does
on a declaration. Nothing is passed by reference, so a write reaches the function's own
copy and the caller never sees it; the mark is what the signature says about the body.

```
fun gcd(mut a: Int32, mut b: Int32): Int32 { … }
```

A tuple's fields take no `mut`: a field is not a binding anything writes through, and
what allows a write to one is the `mut` on the variable holding the tuple. `comp` on a
function means it may be called while the program is being compiled; see
[Compile-time evaluation](#compile-time-evaluation).

Commas between parameters are optional. `fun f(a: Int32 b: Int32)` parses exactly as the
version with a comma, which is a looseness in the parser rather than a style to rely on.

## Statements

**Declaration.** `val name: Type` introduces a variable, optionally with an initialiser.
A variable without one is not zeroed, and reading it before assigning yields an
indeterminate value.

```
val n: Int32 = 21
mut val total: Int32 = 0
mut val buffer: Int32[101]
```

**A `mut` before the declaration says a write can reach what it introduces.** A
declaration without one is written where it is made and read from then on; assigning to
it is rejected, and so is assigning through it to an element or a field. The mark is the
same one every write carries, so wherever `mut` appears it says the same thing: *this can
be written*.

```
val n: Int32 = 21
mut n = 22              # ERROR: no 'mut' on the declaration of 'n'
```

A declaration holds from where it is made to the end of the block holding it, and a
block is what braces enclose: a function's body, either side of an `if`, a loop's body.
A declaration inside a block is gone after it, and one that repeats a name already
declared stands over the earlier one for as long as its own block lasts.

**Assignment** is marked with a leading `mut`, the same mark the declaration of what it
writes carries. It assigns to a variable, to an array element, or to a field:

```
mut n = n + 1
mut buffer[col] = 0
mut p.x = 3
```

A target reaches through as many steps as it needs, so long as the last of them is a
field rather than an index:

```
mut ps[i].x = 3
mut q.p.x = 5
```

What the write reaches is the variable at the foot of the target, and that is the
declaration whose `mut` allows it: `mut ps[i].x = 3` needs `mut val ps`.

**Conditionals** take an expression of type `Bool`, with no parentheses around it. `else`
takes either a block or another `if`.

```
if a == b {
  break
} else if a > b {
  mut a = a - b
} else {
  mut b = b - a
}
```

**Loops** are unconditional. `loop { … }` repeats its body until a `break` leaves the
innermost enclosing loop. There is no `while` and no `for`, and no `continue`.

**Return.** `return expr` leaves the function. The expression's type must be the
function's result type. A function is not obliged to return: control may reach the
closing brace, and the function's result is then whatever the result register happens
to hold. Nothing warns about it.

**Do.** `do f(…)` calls a function for what calling it does and discards its result. What
follows `do` has to be a call; anything else would be worked out and thrown away, and is
rejected. So is `do comp f(…)`: a call made during compilation can only be to a `comp fun`,
which has nothing to do.

## Expressions

| Precedence | Operators | Meaning |
|---|---|---|
| 5, tightest | `*` `/` `%` | multiply, divide, remainder |
| 4 | `+` `-` | add, subtract |
| 3 | `>` `<` `>=` `<=` `==` `!=` | compare, yielding `Bool` |
| 2 | `and` | both hold |
| 1, loosest | `or` | either holds |

All of them are binary and all associate to the left, so `a - b - c` is `(a - b) - c`.

`!` and `comp` stand before one element rather than between two, and reach no further
than that element: `!a == b` compares what `!a` came to, and `comp f() * 2` works out
the call during compilation and multiplies while the program runs. Parentheses are what
give either of them more to work on.

**Arithmetic wraps at the width of its type.** A result that runs past what an `Int32` or
`Int64` holds comes back round, so `2147483647 + 1` is `-2147483648`. Division truncates
towards zero and a remainder takes the sign of what was divided: `-7 / 2` is `-3` and
`-7 % 2` is `-1`. Dividing the least number by `-1` runs past the width too, and comes
back round to the least number. Work done during compilation follows the same rules, so
a `comp` expression comes to what the same expression would while the program runs.

**Dividing by zero is an error.** `x / 0` and `x % 0` stop the program where they happen:
what it printed so far is written out, `division by zero` goes to standard error, and it
exits with status 136, which is what a shell reports for a process stopped by an
arithmetic trap. Inside a `comp` expression the same division is reported while
compiling instead.

**Parentheses group.** What they hold is parsed on its own and binds tighter than
whatever surrounds it, so `(a + b) * c` multiplies the sum where `a + b * c` adds the
product. They leave nothing of themselves behind: `(((7)))` is the literal `7`.

**`and` and `or` read their right side only where their left side leaves the answer
open**, so `no() and q()` never calls `q`. Everything a statement reads before reaching
one of them is still read first: in `f() + pick(p() and q())`, `f` runs before `p`.

**`!` is the one prefix operator**, and asks whether what follows is false. There is no
prefix minus: `-1` is not an expression, and a negative value is computed, as `0 - 1`.

The remaining forms are an identifier, an integer literal, a string literal, `true`,
`false`, a call `f(a, b)`, an index `a[i]`, a field access `p.x`, `!` and what it
negates, and an expression in parentheses. Commas between arguments are optional, as
they are between parameters.

An index and a field access apply to an identifier or a call, and as many of them as
follow do so in turn, each taking what came before it as its base. `ps[1].x` indexes an
array of tuples and reads a field of the element; `q.p.x` reads through a tuple held by
a tuple; `r.v[0]` reads an array held by one.

## Typing

Every expression has a type, worked out for a whole function at once. Rather than
computing a type for each expression from its parts, the checker collects what each
position requires — a `return` requires the function's result type, an `if` requires
`Bool`, an argument requires its parameter's type, both sides of a binary operator
require each other — and then solves them together.

The consequences worth knowing:

- **An integer literal has no type of its own.** It takes the type the position requires,
  and the only check is that its value fits: a literal outside the 32-bit range is
  rejected where a type smaller than eight bytes is required. A literal required to be
  nothing in particular becomes `Int32`, or `Int64` if it does not fit. This is why
  `val b: Bool = 1` is accepted.
- **Comparisons yield `Bool`** and require their two sides to have the same type.
- **Arithmetic** requires both sides and the result to have one type. There are no
  implicit conversions anywhere in the language.

## Compile-time evaluation

`comp` marks work to be done while the program is compiled rather than while it runs.

```
comp fun square(n: Int32): Int32 {
  return n * n
}

fun main(): Int32 {
  return comp square(21)
}
```

**`comp` stands on an expression**, and asks for that expression's value to be worked out
during compilation. It takes the one element that follows it, so parentheses are what
reach further:

```
val a: Int32 = comp square(21)        # the call is made during compilation
val b: Int32 = comp square(21) + n    # the call is, the addition is not
val c: Int32 = comp (f() + g())       # both calls and the addition are
```

Two rules govern what may appear inside a `comp` expression: every function it calls must
be a `comp fun`, and every identifier it reads must hold a value that compilation worked
out. **A variable holds one when the whole of what initialises it is a `comp` expression
and no `mut` allows a write to it**, which is not written down anywhere — it is read off
the declaration:

```
val x: Int32 = comp square(4)
val y: Int32 = comp (x + 1)           # x holds 16, worked out during compilation

mut val z: Int32 = comp square(4)
val w: Int32 = comp (z + 1)           # ERROR: a write can reach z
```

A `comp fun` may not contain a `do` statement, since its body has to be evaluable with
nothing to have an effect on. It is a capability rather than an obligation: a `comp fun`
called without a `comp` on the call is called while the program runs, like any other.

**Compilation gives up on work that does not end, and reports where.** Two limits say
when:

- It follows a program at most **1000 calls deep**. Following a call costs a frame of the
  compiler's own stack rather than of one the program lays out, so this is what keeps a
  program that recurses without end from taking the compiler down with it. It counts how
  deep the calls stand rather than how many are made: a `comp fib(20)` makes 21891 calls
  and never stands more than 20 deep.
- It works through at most **10 million instructions**, counted across the whole of one
  function's compilation. This is what a loop without an end runs into. A block counts
  along with what it holds, so `loop { }` is caught as surely as a loop that does
  something.

## Built-in functions

Two names are implemented by the compiler rather than by the program: `printString` and
`sleep`. A program that wants one **declares it itself**, with any body, and the compiler
replaces that body:

```
fun printString(s: String): Int32 {
  return 0
}
```

`printString` writes its argument to standard output, through C `printf`, so the string
is taken as a format. `sleep` suspends the program, through `nanosleep`.

## Limits

These are defects rather than decisions, and a program that meets one gets no diagnostic
worth the name.

- **Definitions have to precede their uses**, so two functions cannot call each other.
  A call to a name defined further down the file is reported as though the name were
  not there at all, which is true of the compiler's view of it and not of the program's.
- **A function's frame may not be more than 16MB**, which is the largest a sequence of
  instructions here reserves. The stack a program is given runs out well before that.
- **An array or a tuple cannot be a parameter or a result.** Both live on the stack, and
  nothing lays one out on either side of a call. This is reported rather than attempted.
- **An assignment cannot end in an index into something reached through.** `mut r.v[0] = 1`
  is rejected where `mut r.v = …` would not be, because the statement that assigns through
  an index names its array rather than holding an expression for it.
- **Characters outside the token set are skipped silently.** A stray `@` in a function
  body is ignored as though it were a comment.
- `Double` has a name and a size and nothing else. No literal produces one.
- **Nothing checks that a function returns.** A function that falls off its closing
  brace returns whatever was in the result register, and a `return` of the wrong shape
  is caught only by the type rules above, which let `return 0` stand in a function whose
  result is `Void`.
