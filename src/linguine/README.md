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
 +-------------------+  linc  +--------------------+  cc   +-------------------+
 | Source File (.ls) |  --->  | C Source File (.c) |  ==>  | Executable (.exe) |
 +-------------------+        +--------------------+       +-------------------+
```

### Compile source and make binary plugin for Linguine

```
 +-------------------+  linc  +--------------------+  cc   +---------------+
 | Source File (.ls) |  --->  | C Source File (.c) |  ==>  | Plugin (.dll) |
 +-------------------+        +--------------------+       +---------------+
```

## Syntax

### Define a function

```
func main() {
    print("Hello, World!");
}
```

### Assign a value to a variable

```
func main() {
    var a = 123;
    print(a);
}
```
