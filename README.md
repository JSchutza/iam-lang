# IAM Programming Language

A minimalist programming language implemented in x86 assembly.

## Description

IAM is a simple REPL-based programming language with support for:
- Variables (single-letter, a-z)
- Basic arithmetic operations
- Simple control flow (IF statements)
- Printing text and variable values

This language is implemented entirely in x86 assembly and serves as a demonstration of how programming languages can be built at a low level.

## Building

To build the IAM language interpreter, you need:
- NASM (Netwide Assembler)
- A Linux environment (or WSL on Windows)
- GCC toolchain

Then run:

```
make
```

## Running

To start the interpreter:

```
make run
```

Or directly:

```
./iam_lang
```

## Language Syntax

IAM has a simple syntax:

```
LET x = 10      # Assign value 10 to variable x
PRINT x         # Print the value of variable x
PRINT "Hello"   # Print a string
EXIT            # Exit the interpreter
```

## Examples

```
> LET a = 5
> LET b = 10
> PRINT a
5
> PRINT "Hello, world!"
Hello, world!
> EXIT
```

## Implementation Notes

This language is implemented in x86 32-bit assembly and uses Linux syscalls for I/O operations. The code demonstrates fundamental concepts like lexical analysis, parsing, and interpretation in a minimal way. 