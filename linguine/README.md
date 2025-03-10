Linguine
========

Linguine is a fast, portable scripting language designed for seamless
integration into other applications. It features a clean, C-like
syntax and modern programming capabilities including iterators and
dictionaries. Linguine strikes a balance between being easy to learn
for beginners while providing powerful features for experienced
programmers.

Linguine features a built-in Just-in-Time compiler that boosts
performance by generating native code during runtime. The JIT compiler
works with 32-bit Intel, 64-bit AMD/Intel, and 64-bit Arm
architectures, delivering fast and reliable execution. On other
platforms, Linguine falls back to a classic interpreter.

## Build and Run

```
cd linguine/linux
make
./linguine ../../hello.ls
```

## Workflow

### Source Execution

Use the `linguine` command.
JIT is enabled by default. If you want to turn off JIT,
add the `-i` option to turn it off.

```
 +--------+       +=======+
 | Source |  ==>  || Run ||
 +--------+       +=======+
```

### Bytecode Execution

Not implemented yet.  Although the bytecode format is internally used,
we don't have its file format so far.

```
 +-------------------+        +----------------------+       +=======+
 | Source File (.ls) |  --->  | Bytecode File (.lsc) |  ==>  || Run ||
 +-------------------+        +----------------------+       +=======+
```

### Standalone Execution

Not implemented yet.
We are going to make a C backend in the near future.

```
 +-------------------+        +--------------------+       +-------------------+      +=======+
 | Source File (.ls) |  --->  | C Source File (.c) |  -->  | Executable (.exe) |  ==> || Run ||
 +-------------------+        +--------------------+       +-------------------+      +=======+
```

### Plugin Library

Same as standalone execution.

```
 +-------------------+        +--------------------+       +---------------+
 | Source File (.ls) |  --->  | C Source File (.c) |  -->  | Plugin (.dll) |
 +-------------------+        +--------------------+       +---------------+
```

## Internal Data Flow

```
 +--------+  parser  +-----+  HIR-pass  +-----+  LIR-pass  +-----+  JIT-pass  +-------------+
 | Source |  ----->  | AST |  ------->  | HIR |  ------->  | LIR |  ------->  | Native Code |
 +--------+          +-----+            +-----+            +-----+            +-------------+
```

Where:
* AST: represents source code syntax tree. (Abstract Syntax Tree)
* HIR: represents a structured list of statements, which is suitable for optimizations.
* LIR: is a linear list of runnable instructions, also called `bytecode`.
