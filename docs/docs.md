# Docs

## What on Earth is Brainfuck?

Brainfuck is a rather famous esoteric programming language, invented by
[Urban M&uuml;ller](http://esolangs.org/wiki/Urban_Müller) in 1993. It operates
on an array of cells, also referred to as the _tape_. A pointer is used to
address into the tape and perform read and write operations on the cells of the
tape.

Brainfuck provides eight commands in total. These are:

| Command | Action                                                              |
| :-----: | ------------------------------------------------------------------- |
|   `>`   | Move the pointer one cell to the right                              |
|   `<`   | Move the pointer one cell to the left                               |
|   `+`   | Increment the cell under the pointer                                |
|   `-`   | Decrement the cell under the pointer                                |
|   `.`   | Output the cell under the pointer as a character                    |
|   `,`   | Input a character and store it in the cell under the pointer        |
|   `[`   | Jump past the matching `]` if the cell under the pointer is zero    |
|   `]`   | Return to the matching `[` if the cell under the pointer is nonzero |

These eight cleverly thought-out commands are enough to make Brainfuck a
[Turing-complete](http://esolangs.org/wiki/Turing-complete) language. However,
given its adorable syntax and crudity, this Turing-completeness also qualifies
Brainfuck as an unfortunate
[Turing tar-pit](http://esolangs.org/wiki/Turing_tarpit).

### Say, _Hello World_

Saying **_Hello World_** in Brainfuck is no easy feat. It looks something like
this:

```brainfuck
>+++++++++[<++++++++>-]<.>+++++++[<++++>-]<+.+++++++..+++.>>>++++++++[<++++>-]<
.>>>++++++++++[<+++++++++>-]<---.<<<<.+++.------.--------.>>+.
```

As you can see, although it is perfectly capable of producing that famous and
ever-so-loved message, the syntax of the code to achieve this task might creep
you out. Fear not, though&hellip; Fear not!

## Brainfuck, the compiler

Now that Brainfuck, the language is clarified, let's focus on this project. This
is a somewhat optimizing compiler and interpreter suite written in
[C](<http://en.wikipedia.org/wiki/C_(programming_language)>).

The compiler works by first reading in some Brainfuck source code from a
specified stream. It then parses this source code and constructs an intermediate
representation of the program. Then, it applies a bunch of simple and
straightforward optimizations to this intermediate representation. The result is
a vector of instructions ready to be interpreted or converted and emitted in
some other format (e.g. as C or [LLVM](http://en.wikipedia.org/wiki/LLVM)
bytecode).

In addition to these the compiler also supports outputting the internal
intermediate representation of the code it is working on in a human-friendly
form.
