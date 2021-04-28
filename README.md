# Compiler from scratch

A [Brainfuck](http://esolangs.org/wiki/brainfuck) compiler & interpreter with
optimizations implemented in both C and Python.

See [docs](./docs/docs.md) for more details about the goals and design.

## Usage (C compiler)

The most straightforward way of invoking the compiler would be to simply give it
a source file to chew on:

```bash
compiler filename
```

Executing the compiler like this, with a filename alone will result in that file
being treated as the source code of interest. The compiler will then try to
compile the source code and interpret the resulting byte code.

In addition to this simple use case the compiler supports some options. Here's
the help screen for reference:

```
    dP                         oo          .8888b                   dP
    88                                     88   "                   88
    88d888b. 88d888b. .d8888b. dP 88d888b. 88aaa  dP    dP .d8888b. 88  .dP
    88'  `88 88'  `88 88'  `88 88 88'  `88 88     88    88 88'  `"" 88888"
    88.  .88 88       88.  .88 88 88    88 88     88.  .88 88.  ... 88  `8b.
    88Y8888' dP       `88888P8 dP dP    dP dP     `88888P' `88888P' dP   `YP

Released into the public domain.

Usage:
        ./compiler [--cdehlruvxz] <input>

Options:
        --                          read input from stdin
        -c [filename=`compiler.c`] generate and emit C code
        -d                          print disassembly
        -e                          explain source code
        -h                          display this help screen
        -l [filename=`compiler.l`] generate and emit LLVM IR
        -r                          JIT compile and execute
        -u                          disable optimizations
        -v                          display version information
        -x                          disable interpretation
        -z <length=`30000`>         set tape length
```

The `[]` brackets indicate an optional block of argument. You can omit these at
will. For example, If you'd like to emit C code into the default file
`compiler.c` you can specify the `-c` option while omitting its argument.

On the other hand, `<>` brackets stand for an obligatory argument block and they
cannot be omitted; although if the input is read from `stdin` there is no need
to specify a separate source code file as `<input>`.

## Python compiler

This repo also includes a compiler implemented using Python 3 that produces x64
NASM assembly code for Linux that has no external dependencies (uses only Linux
syscalls). The resulting code has few basic optimizations, but overall it's
quite straightforward.

The Compiler supports following targets:

- x64-linux - produces standalone 64-bit Linux assembly
- x64-linux-lib - produces 64-bit Linux assembly that respects calling
  convention and exports global function that can be called by C program

### Compiler options

- memory model - defines what happens after memory overflow/underflow
  - wrap - memory wraps around on negative or too big index
  - abort - produces error on overflow/underflow
  - undefined - behavior undefined, overflow may cause segmentation fault
  - cell size
    - byte - 8 bits
    - word - 16 bits
    - dword - 32 bits

## Example Programs

### Hello World

    ++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.

### Echo single character

    ,.

### Print the alphabet

    ++++++++++++++++++++++++++>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++<[>.+<-]
