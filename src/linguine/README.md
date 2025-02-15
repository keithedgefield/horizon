Linguine
========

Linguine is a small and portable scripting language.

## Workflow

### Run source directly

```
 +--------+  lin  +===============+
 | Source |  ==>  || Interpreter ||
 +--------+       +===============+
```

### Compile source and run bytecode file

```
 +-------------------+  linc  +----------------------+  lin  +===============+
 | Source File (.ls) |  --->  | Bytecode File (.lsc) |  ==>  || Interpreter ||
 +-------------------+        +----------------------+       +===============+
```

### Compile source and run native executable

```
 +-------------------+  linc  +--------------------+  cc  +-------------------+
 | Source File (.ls) |  --->  | C Source File (.c) |  ->  | Executable (.exe) |
 +-------------------+        +--------------------+      +-------------------+
```

### Compile source and make binary plugin for Linguine

```
 +-------------------+  linc  +--------------------+  cc  +---------------+
 | Source File (.ls) |  --->  | C Source File (.c) |  ->  | Plugin (.dll) |
 +-------------------+        +--------------------+      +---------------+
```

### Internal Data Flow

```
 +-------------+  parser  +-----+  HIR pass  +-----+  LIR pass  +-----+
 | Source Code |  ----->  | AST |  ------->  | HIR |  ------->  | LIR |
 +-------------+          +-----+            +-----+            +-----+
```

* AST
    * Abstract Syntax Tree (AST) is a data structure that directly represents source code syntax tree.
* HIR
    * High-level Intermediate Representation (HIR) is a data structure that is suitable for applying various optimizations.
    * HIR is a list of statements, preserving a block structure such as if-block and for-block.
* LIR
    * Low-level Intermediate Representaion (LIR) is a list of runnable instructions, also called bytecode.
    * LIR bytecode may be:
        * Interpreted on the `lin` command. (This is slow but useful and portable)
        * JIT-compiled to a native code and then executed. (This is fast but for x86 and arm64 only)
        * Translated to C source code and then compiled to a native executable. (This is fast and portable)

## Syntax

### Defining Functions

```
func main() {
    print("Hello, World!");
}
```

### Assigning Values

```
func main() {
    a = 123;
    print(a);
}
```
