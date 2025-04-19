# IAM Programming Language

A minimalist programming language implemented in C++.

## Description

IAM is a simple programming language with support for:
- Variables
- Basic printing of strings and values
- User input
- Extensible design for easy addition of features

This language is implemented in C++ and serves as a demonstration of how programming languages can be built with a clean, modular design.

## Installation and Building

### Automated Installation

The easiest way to build the IAM language interpreter is to use the provided installation script:

```bash
chmod +x install_and_build.sh
./install_and_build.sh
```

This script will:
1. Install necessary dependencies (C++ compiler, Make)
2. Build the interpreter
3. Create example files if they don't exist

### Manual Building

If you prefer to build manually, you need:
- A C++ compiler (g++ or Visual C++)
- Make (optional)

To build with Make:
```bash
make
```

To build directly with g++:
```bash
g++ -std=c++11 -Wall -o iam_lang iam_lang.cpp
```

## Running

To start the interpreter with an input file:

```bash
./iam_lang examples.iam
```

To use the interpreter interactively:

```bash
./iam_lang
```

You can also pipe content to the interpreter:

```bash
cat examples.iam | ./iam_lang
```

## Language Syntax

IAM has a simple syntax:

```
# Comments start with a hash

# Print a string
print "Hello, world!"

# Set a variable
set counter 10

# Print a variable
print counter

# Get user input
print "Enter a number:"
input userValue

# Print the entered value
print userValue
```

The language also supports legacy syntax from the previous version:

```
LET x = 10      # Assign value 10 to variable x
PRINT x         # Print the value of variable x
PRINT "Hello"   # Print a string
EXIT            # Exit the interpreter
```

## Examples

An examples file (`examples.iam`) is included in the repository, which demonstrates the basic functionality:

```
# Print a welcome message
print "Welcome to IAM Language!"

# Variable assignment
set counter 10

# Print a variable
print "The counter value is:"
print counter

# Ask for user input
print "Please enter a number:"
input userValue

# Show the entered value
print "You entered:"
print userValue
```

## Implementation Notes

This language is implemented in C++ and features:
- A lexer that converts source code into tokens
- A parser that processes tokens into executable statements
- A simple variable system for storing integer values
- Extensible design for adding new language features

## Platform Support

The IAM language interpreter should work on:
- Windows (with Scoop for dependencies)
- Linux
- macOS (untested) 