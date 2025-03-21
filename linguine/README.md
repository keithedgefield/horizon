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

## Syntax

### Assigning a value

Variables in Linguine are dynamically typed and don't require explicit
declaration. The assignment operator (`=`) is used to create and
assign values to variables. As shown in the example below, Linguine
supports various data types including integers, floating-point
numbers, and strings. Variables can be reassigned to different types
at any time during execution.

```
func main() {
    a = 123;
    print(a);

    b = 1.0;
    print(b);

    c = "string";
    print(c);
}
```

### Array

Linguine arrays are ordered collections of values, accessed by
index. The language provides built-in functions like `push()` to add
elements to the end of an array. Arrays support iteration through the
`for` loop construct, allowing you to iterate through each value
directly. Arrays can hold values of different types simultaneously,
reflecting Linguine's dynamic typing system.

```
func main() {
    array = [0, 1, 2];
    push(array, 3);
    for (value in array) {
        print(value);
    }
}
```

### Dictionary

Dictionaries in Linguine store key-value pairs, similar to hash maps
or objects in other languages. They are defined using curly braces
with key-value pairs separated by colons. Dictionaries support
iteration where both the key and value can be accessed
simultaneously. The built-in function `remove()` allows for the
deletion of entries by key.

```
func main() {
    dict = {key1: "value1", key2: "value2"};
    for (key, value in dict) {
        print("key = " + key);
        print("value = " + value);
    }
    remove(dict, "key1");
}
```

## For-loop

Linguine's for-loop construct provides a concise syntax for iterating
through sequences such as ranges, arrays, and dictionaries. The range
syntax (using the `..` operator) creates an iterator that generates
values from the start to one less than the end value. For-loops can
also iterate directly over arrays and other collection types.

```
func main() {
    for (i in 0..10) {
        print(i);
    }
}
```

### While-loop

The while-loop in Linguine provides a traditional iteration mechanism
that continues execution as long as a specified condition remains
true. Unlike for-loops which are designed for iterating over
collections, while-loops are more flexible and can be used for
implementing various algorithms where the number of iterations isn't
known in advance. The example shows a basic counter implementation
incrementing from 0 to 9.

```
func main() {
    i = 0;
    while (i < 10) {
        print(i);
        i = i + 1;
    }
}
```

### If and else-blocks

Control flow in Linguine allows for conditional execution based on
evaluated expressions. The if-else construct follows a familiar syntax
where conditions are evaluated in sequence. If the initial if
condition is false, the program checks each else-if clause, and if all
conditions are false, it executes the else block. This provides clean,
readable code for handling multiple conditions and branching logic.

```
func main() {
    for (i in 0..3) {
        if (a == 0) {
            print("0");
        } else if (a == 1) {
            print("1");
        } else {
            print("2");
        }
    }
}
```

### Lambda Functions

In Linguine, functions are first-class objects. Anonymous functions,
also known as `lambda` expressions, allow you to create functions
without names.

```
func main() {
    f = lambda (a, b) { return a + b; }
    print(f(1, 2));
}
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

## Compilation Stages

```
 +--------+  parser  +-----+  HIR-pass  +-----+  LIR-pass  +-----+  JIT-pass  +-------------+
 | Source |  ----->  | AST |  ------->  | HIR |  ------->  | LIR |  ------->  | Native Code |
 +--------+          +-----+            +-----+            +-----+            +-------------+
```

The Linguine compiler first transforms source code into an AST
(abstract syntax tree)—a data structure optimized for program
parsing.

The AST is then converted to HIR (high-level intermediate
representation), which is better suited for optimization. Future
versions will implement additional optimizers.

Next, the HIR is transformed into LIR (low-level intermediate
representation), also known as "bytecode"—a byte sequence designed
for interpretation.

Finally, the JIT compiler translates this LIR into native code.
