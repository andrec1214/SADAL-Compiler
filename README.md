# SADAL Compiler

A C++ lexer, recursive descent parser, and interpreter for the SADAL (Simple ADA-Like) language, originally built as part of a programming language concepts course project.

## Overview

The project has three stages, each with its own executable:

| Program       | Files                                     | What it does                                          |
|---------------|-------------------------------------------|-------------------------------------------------------|
| `sadal-lex`   | `lex.h`, `lex.cpp`, `prog1.cpp`           | Prints the token stream of a source file              |
| `sadal-parse` | `parser.h`, `parser.cpp`, `prog2.cpp`     | Checks syntax and lists the declared variables        |
| `sadal`       | `parserInterp.*`, `val.*`, `prog3.cpp`    | Parses **and runs** the program                       |

All three share the same lexer. Parsing is recursive descent over the 21 EBNF production rules below.

## Building and Running

```sh
make                         # builds sadal-lex, sadal-parse and sadal
./sadal tests/features.ada   # run a program
./sadal-parse tests/example.ada
./sadal-lex tests/example.ada
make test                    # run the regression tests in tests/
```

`Get` reads from standard input, so you can pipe input in: `echo 42 | ./sadal tests/example.ada`.

## Features

- **Lexical Analysis**: Tokenizes source code into identifiers, constants (integer, float, string, boolean, character), keywords, and operators
- **Recursive Descent Parsing**: Hand-written parser implementing all 21 EBNF production rules
- **Variable Declarations**: Type annotations with optional initialization for INTEGER, FLOAT, BOOLEAN, STRING, and CHARACTER types
- **I/O Operations**: Support for Put, PutLine (output) and Get (input) statements
- **Expression Parsing**: Supports arithmetic expressions with proper operator precedence (+, -, *, /, MOD, **)
- **Control Flow**: IF-THEN-ELSIF-ELSE statements with nested conditions
- **Boolean Logic**: AND/OR operations on relational expressions
- **String Indexing**: 1-based indexing and slicing, e.g. `s(1)` and `s(2..4)`
- **Interpretation**: Programs are executed while they are parsed, with type checking and run-time errors

## Grammar Specification

The parser recognizes the complete SADAL language EBNF grammar:

```
1.  Prog        ::= PROCEDURE ProcName IS ProcBody

2.  ProcBody    ::= DeclPart BEGIN StmtList END ProcName ;

3.  ProcName    ::= IDENT

4.  DeclPart    ::= DeclStmt { DeclStmt }

5.  DeclStmt    ::= IDENT {, IDENT } : Type [:= Expr] ;

6.  Type        ::= INTEGER | FLOAT | BOOLEAN | STRING | CHARACTER

7.  StmtList    ::= Stmt { Stmt }

8.  Stmt        ::= AssignStmt | PrintStmts | GetStmt | IfStmt

9.  PrintStmts  ::= (PutLine | Put) ( Expr ) ;

10. GetStmt     ::= Get (Var) ;

11. IfStmt      ::= IF Expr THEN StmtList { ELSIF Expr THEN StmtList } 
                    [ ELSE StmtList ] END IF ;

12. AssignStmt  ::= Var := Expr ;

13. Expr        ::= Relation { (AND | OR) Relation }

14. Relation    ::= SimpleExpr [ ( = | /= | < | <= | > | >= ) SimpleExpr ]

15. SimpleExpr  ::= STerm { ( + | - | & ) STerm }

16. STerm       ::= [ ( + | - ) ] Term

17. Term        ::= Factor { ( * | / | MOD ) Factor }

18. Factor      ::= Primary [** [(+ | -)] Primary ] | NOT Primary

19. Primary     ::= Name | ICONST | FCONST | SCONST | BCONST | CCONST | (Expr)

20. Name        ::= IDENT [ ( Range ) ]

21. Range       ::= SimpleExpr [ .. SimpleExpr ]
```

## Technical Details

- **Language**: C++
- **Parsing Technique**: Recursive descent (top-down, LL(1))
- **Productions**: 21 EBNF grammar rules implemented
- **Operator Precedence**: Properly handles arithmetic and boolean operator precedence

## Language Semantics

The interpreter (`sadal`) follows Ada's rules where the grammar leaves room:

- **Strong typing**: arithmetic operands must both be `INTEGER` or both `FLOAT`; there is no implicit conversion. Assignments and initializers must match the declared type.
- **Operators**: `/` on integers truncates toward zero, `MOD` takes the sign of the divisor, `**` takes an `INTEGER` exponent (non-negative for an `INTEGER` base), and `&` concatenates `STRING`s and `CHARACTER`s.
- **Precedence**: as in Ada, unary minus binds looser than `*`, `/`, `MOD`, and `**`, so `-7 MOD 3` is `-(7 MOD 3)`.
- **Constants**: `x : CONSTANT INTEGER := 5;` must be initialized and cannot be assigned or read into.
- **Input**: `Get` reads one whitespace-separated word for `INTEGER`, `FLOAT`, `CHARACTER`, and `BOOLEAN` (`true`/`false`), and the rest of the line for `STRING`.
- **Lexical**: keywords are case-insensitive, identifiers are case-sensitive, and comments start with `--`.
- **Run-time errors** (uninitialized variables, type mismatches, division by zero, index out of range) stop execution with a line number.
- A range constraint in a declaration such as `s : STRING(1..10);` is accepted but not enforced.

## Example Input

```ada
PROCEDURE Example IS
    x, y : INTEGER;
    result : FLOAT := 3.14;
BEGIN
    Get(x);
    IF x > 10 THEN
        y := x * 2 + 5;
        PutLine(y);
    ELSIF x < 0 THEN
        y := -x;
        Put(y);
    ELSE
        y := x;
    END IF;
END Example;
```

---

## Notes

This was developed as part of a compilers course to understand parsing theory and implementation. The original lexer and interpreter sources were lost and have since been rewritten, along with fixes to the parser (assignment statements, string indexing, ranges, and signed exponents).
