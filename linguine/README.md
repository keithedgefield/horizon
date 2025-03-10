Linguine
========

Linguine is a fast, modern, compact scripting language for application
integration. With its C-like syntax, it includes modern features like
iterators and dictionaries, making it both beginner-friendly and
powerful. The language includes a Just-in-Time compiler that generates
native code for Intel, AMD, and Arm architectures, with interpreter
fallback for other platforms.

## Build and Run

```
cd build/linguine/linux
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
