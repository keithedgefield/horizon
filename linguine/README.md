Linguine
========

Linguine is a small, portable scripting language designed for easy
integration into other applications. The language offers a clean,
C-like syntax and includes modern programming features such as
iterators and dictionaries.

## Workflow

### Source Execution

```
 +--------+       +=======+
 | Source |  ==>  || Run ||
 +--------+       +=======+
```

### Bytecode Execution

```
 +-------------------+        +----------------------+       +=======+
 | Source File (.ls) |  --->  | Bytecode File (.lsc) |  ==>  || Run ||
 +-------------------+        +----------------------+       +=======+
```

### Standalone Execution

```
 +-------------------+        +--------------------+       +-------------------+      +=======+
 | Source File (.ls) |  --->  | C Source File (.c) |  -->  | Executable (.exe) |  ==> || Run ||
 +-------------------+        +--------------------+       +-------------------+      +=======+
```

### Plugin Library

```
 +-------------------+        +--------------------+       +---------------+
 | Source File (.ls) |  --->  | C Source File (.c) |  -->  | Plugin (.dll) |
 +-------------------+        +--------------------+       +---------------+
```

## Internal Data Flow

```
 +--------+  parser  +-----+  HIR pass  +-----+  LIR pass  +-----+
 | Source |  ----->  | AST |  ------->  | HIR |  ------->  | LIR |
 +--------+          +-----+            +-----+            +-----+
```

Where:
* AST represents source code syntax tree.
* HIR represents a structured list of statements that is suitable for optimizations
* LIR is a linear list of runnable instructions, also called bytecode.
