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

## Lexical structure

The source is a sequence of bytes ending in a null byte. Nothing in the language depends
on the layout of lines. A newline is whitespace like any other, and there is no statement
terminator.

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
`|` `!` `&`. The two-character operators `==` and `!=` are written as `=` or `!` followed
immediately by `=`, with no space between them.

**Keywords** are not reserved. `fun`, `val`, `comp`, `if`, `else`, `loop`, `break`,
`return`, `do`, `true` and `false` are ordinary identifiers that the parser recognises
where a definition or a statement begins, and `Type` is recognised only in a type
definition. Nothing stops a variable being called `loop`.

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
braces. `comp` on a function means it may be called while the program is being compiled;
see [Compile-time evaluation](#compile-time-evaluation).

Commas between parameters are optional. `fun f(a: Int32 b: Int32)` parses exactly as the
version with a comma, which is a looseness in the parser rather than a style to rely on.

## Statements

**Declaration.** `val name: Type` introduces a variable, optionally with an initialiser.
A variable without one is not zeroed, and reading it before assigning yields an
indeterminate value.

```
val n: Int32 = 21
val buffer: Int32[101]
```

A `val` is not constant. The name is misleading: variables are assigned freely after
declaration.

A declaration holds from where it is made to the end of the block holding it, and a
block is what braces enclose: a function's body, either side of an `if`, a loop's body.
A declaration inside a block is gone after it, and one that repeats a name already
declared stands over the earlier one for as long as its own block lasts.

**Assignment** is marked with a leading `&`, which is what tells it apart from a
declaration. It assigns to a variable, to an array element, or to a field:

```
&n = n + 1
&buffer[col] = 0
&p.x = 3
```

A target reaches through as many steps as it needs, so long as the last of them is a
field rather than an index:

```
&ps[i].x = 3
&q.p.x = 5
```

**Conditionals** take an expression of type `Bool`, with no parentheses around it. `else`
takes either a block or another `if`.

```
if a == b {
  break
} else if a > b {
  &a = a - b
} else {
  &b = b - a
}
```

**Loops** are unconditional. `loop { … }` repeats its body until a `break` leaves the
innermost enclosing loop. There is no `while` and no `for`, and no `continue`.

**Return.** `return expr` leaves the function. The expression's type must be the
function's result type. A function is not obliged to return: control may reach the
closing brace, and the function's result is then whatever the result register happens
to hold. Nothing warns about it.

**Do.** `do expr` evaluates an expression for its effect and discards the value. It is
how a function is called when its result is not wanted.

## Expressions

| Precedence | Operators | Meaning |
|---|---|---|
| 3, tightest | `*` `/` `%` | multiply, divide, remainder |
| 2 | `+` `-` | add, subtract |
| 1, loosest | `>` `<` `==` `!=` | compare, yielding `Bool` |

All of them are binary and all associate to the left, so `a - b - c` is `(a - b) - c`.

**Parentheses group.** What they hold is parsed on its own and binds tighter than
whatever surrounds it, so `(a + b) * c` multiplies the sum where `a + b * c` adds the
product. They leave nothing of themselves behind: `(((7)))` is the literal `7`.

**There are no unary operators.** `-1` is not an expression, and a negative value has to
be computed, as `0 - 1`.

The remaining forms are an identifier, an integer literal, a string literal, `true`,
`false`, a call `f(a, b)`, an index `a[i]`, a field access `p.x` and an expression in
parentheses. Commas between arguments are optional, as they are between parameters.

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
  comp val c: Int32 = square(21)
  return c
}
```

A `comp val`'s initialiser is evaluated during compilation, and two rules govern what may
appear in it: every function it calls must be a `comp fun`, and every identifier it reads
must be another `comp val`. A `comp fun` may not contain a `do` statement, since its body
has to be evaluable with nothing to have an effect on.

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
- **More than ten values live across a branch cannot be compiled.** Ten sometimes cannot
  either, depending on what else is live. The compiler stops rather than emitting wrong
  code. See [TODO.md](TODO.md).
- **A function may not have more than ten parameters**, for a related reason: parameters
  arrive in registers and there is no path for passing them on the stack.
- **An array or a tuple cannot be a parameter or a result.** Both live on the stack, and
  nothing lays one out on either side of a call. This is reported rather than attempted.
- **An assignment cannot end in an index into something reached through.** `&r.v[0] = 1`
  is rejected where `&r.v = …` would not be, because the statement that assigns through
  an index names its array rather than holding an expression for it.
- **Characters outside the token set are skipped silently.** A stray `@` in a function
  body is ignored as though it were a comment.
- `Double` has a name and a size and nothing else. No literal produces one.
- **Nothing checks that a function returns.** A function that falls off its closing
  brace returns whatever was in the result register, and a `return` of the wrong shape
  is caught only by the type rules above, which let `return 0` stand in a function whose
  result is `Void`.
